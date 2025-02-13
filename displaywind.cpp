    #include "displaywind.h"
    #include "ui_displaywind.h"
    #include <QDebug>
    #include <QPainter>
    DisplayWind::DisplayWind(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::DisplayWind)
    {
        ui->setupUi(this);
        win_width_ = width();
        win_height_ = height();
        memset(&dst_video_frame_, sizeof(VideoFrame), 0);
        play_state_ = 2;
    }

    DisplayWind::~DisplayWind()
    {
        QMutexLocker locker(&m_mutex);
        delete ui;
        DeInit();
    }

    int DisplayWind::Draw(const Frame *frame)
    {
        QMutexLocker locker(&m_mutex);
        if(!img_scaler_ || req_resize_) {
            if(img_scaler_) {
                DeInit();
            }
            win_width_ = width();
            win_height_ = height();
            video_width = frame->width;
            video_height = frame->height;
            img_scaler_ = new ImageScaler();
            double video_aspect_ratio = frame->width * 1.0 / frame->height;
            double win_aspect_ratio = win_width_ * 1.0 / win_height_;
            if(win_aspect_ratio > video_aspect_ratio) {
                //此时应该是调整x的起始位置，以高度为基准
                img_height = win_height_;
                img_height &= 0xfffc;
                img_width = img_height * video_aspect_ratio;
                img_width &= 0xfffc;
                y_ = 0;
                x_ = (win_width_ - img_width) / 2;
            } else {
                //此时应该是调整y的起始位置，以宽度为基准
                img_width = win_width_;
                img_width &= 0xfffc;
                img_height = img_width / video_aspect_ratio;
                img_height &= 0xfffc;
                x_ = 0;
                y_ = (win_height_ - img_height) / 2;
            }
            img_scaler_->Init(video_width, video_height, frame->format,
                            img_width, img_height, AV_PIX_FMT_RGB24);
            memset(&dst_video_frame_, 0, sizeof(VideoFrame));
            dst_video_frame_.width = img_width;
            dst_video_frame_.height = img_height;
            dst_video_frame_.format = AV_PIX_FMT_RGB24;
            dst_video_frame_.data[0] = (uint8_t*)malloc(img_width * img_height * 3);
            dst_video_frame_.linesize[0] = img_width * 3; // 每行的字节数
            req_resize_ = false;
        }

        // 创建滤镜链并应用
        char filters_descr[512];
        snprintf(filters_descr, sizeof(filters_descr),
            "format=yuv420p,"  // 确保输入格式是 yuv420p
            "%s%s"            // 水平和垂直翻转
            "eq=brightness=%f:contrast=%f:saturation=%f,"  // 亮度、对比度、饱和度
            "hue=h=%f,"                                    // 色相
            "unsharp=luma_msize_x=%d:luma_amount=%f,"     // 锐化
            "colorbalance=rs=%f:gs=%f:bs=%f,"             // 色温
            "colorbalance=rm=%f:gm=%f:bm=%f,"             // 色调
            "boxblur=lr=%d:lp=1:cr=%d:cp=1",             // 模糊
            flip_horizontal_ ? "hflip," : "",              // 水平翻转
            flip_vertical_ ? "vflip," : "",                // 垂直翻转
            brightness_, contrast_, saturation_,
            hue_,
            3, sharpAmount_,
            temperature_ > 1.0f ? std::min((temperature_ - 1.0f) * 0.5f, 1.0f) : 0.0f,  // 限制在 [-1, 1] 范围内
            0.0f,
            temperature_ < 1.0f ? std::min((1.0f - temperature_) * 0.5f, 1.0f) : 0.0f,  // 限制在 [-1, 1] 范围内
            tint_ > 0.0f ? std::min(tint_, 1.0f) : 0.0f,  // 红色调，限制在 [-1, 1] 范围内
            0.0f,                                          // 绿色调
            tint_ < 0.0f ? std::min(-tint_, 1.0f) : 0.0f, // 蓝色调，限制在 [-1, 1] 范围内
            blurRadius_, blurRadius_);

        // 应用滤镜
        if (filter_graph_) {
            avfilter_graph_free(&filter_graph_);
            filter_graph_ = nullptr;
        }

        int ret = init_filters(filters_descr, frame->width, frame->height, frame->format);
        if (ret < 0) {
            LOG(ERROR) << "Failed to initialize filters";
            return ret;
        }

        // 将帧送入滤镜链
        if ((ret = av_buffersrc_add_frame_flags(buffersrc_ctx_, 
                frame->frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
            LOG(ERROR) << "Error while feeding the filtergraph";
            return ret;
        }

        // 获取滤镜处理后的帧
        AVFrame *filt_frame = av_frame_alloc();
        ret = av_buffersink_get_frame(buffersink_ctx_, filt_frame);
        if (ret < 0) {
            av_frame_free(&filt_frame);
            return ret;
        }

        // 使用滤镜处理后的帧进行缩放
        Frame filtered_frame = {0};
        filtered_frame.frame = filt_frame;
        filtered_frame.width = filt_frame->width;
        filtered_frame.height = filt_frame->height;
        filtered_frame.format = filt_frame->format;

        img_scaler_->Scale3(&filtered_frame, &dst_video_frame_);

        QImage imageTmp = QImage((uint8_t *)dst_video_frame_.data[0],
                                img_width, img_height, QImage::Format_RGB888);
        img = imageTmp.copy(0, 0, img_width, img_height);
        update();

        av_frame_free(&filt_frame);
        return 0;
    }

    void DisplayWind::DeInit()
    {
        if(dst_video_frame_.data[0]) {
            free(dst_video_frame_.data[0]);
            dst_video_frame_.data[0] = NULL;
        }
        if(img_scaler_) {
            delete img_scaler_;
            img_scaler_ = NULL;
        }

        // 释放滤镜资源
        if (filter_graph_) {
            avfilter_graph_free(&filter_graph_);
            filter_graph_ = nullptr;
            buffersrc_ctx_ = nullptr;  // 这些 context 会在 graph 释放时自动释放
            buffersink_ctx_ = nullptr;
        }
    }

    void DisplayWind::StartPlay()
    {
        QMutexLocker locker(&m_mutex);
        play_state_ = 1;
    }

    void DisplayWind::StopPlay()
    {
        QMutexLocker locker(&m_mutex);
        play_state_ = 2;
        update();
    }

    void DisplayWind::setFilterParams(float brightness, 
                                    float contrast,
                                    int blurRadius,
                                    float sharpAmount,
                                    float hue,
                                    float saturation,
                                    float temperature)
    {
        brightness_ = brightness;
        contrast_ = contrast;
        blurRadius_ = blurRadius;
        sharpAmount_ = sharpAmount;
        hue_ = hue;
        saturation_ = saturation;
        temperature_ = temperature;

        // TODO: 在渲染时应用这些滤镜参数
        // 可以在 Draw() 函数中使用这些参数来调整图像效果
        // 具体实现可能需要使用 OpenGL shader 或其他图像处理方法
        update(); // 触发重绘
    }

    void DisplayWind::paintEvent(QPaintEvent *)
    {
        QMutexLocker locker(&m_mutex);
        if(play_state_ == 1) {  // 播放状态
            if (img.isNull()) {
                return;
            }
            QPainter painter(this);
            painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            //    //    p.translate(X, Y);
            //    //    p.drawImage(QRect(0, 0, W, H), img);
            QRect rect = QRect(x_, y_, img.width(), img.height());
            //        qDebug() << rect << ", win_w:" << this->width() << ", h:" << this->height();
            painter.drawImage(rect, img.scaled(img.width(), img.height()));
        } else if(play_state_ == 2) {
            QPainter p(this);
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            p.drawRect(rect());
        }
    }


    //void DisplayWind::paintEvent(QPaintEvent *)
    //{
    //    QMutexLocker locker(&m_mutex);
    //    if(play_state_ == 1) {  // 播放状态
    //        if (img.isNull()) {
    //            return;
    //        }
    //        QPainter painter(this);
    //        //        painter.setRenderHint(QPainter::Antialiasing, true);
    //        painter.setRenderHint(QPainter::HighQualityAntialiasing);
    //        int w = this->width();
    //        int h = this->height();
    //        img.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //        //    //    p.translate(X, Y);
    //        //    //    p.drawImage(QRect(0, 0, W, H), img);
    //        QRect rect = QRect(x_, y_, w, h);
    //        painter.drawImage(rect, img);
    //    } else if(play_state_ == 2) {
    //        QPainter p(this);
    //        p.setPen(Qt::NoPen);
    //        p.setBrush(Qt::black);
    //        p.drawRect(rect());
    //    }
    //}


    void DisplayWind::resizeEvent(QResizeEvent *event)
    {
        QMutexLocker locker(&m_mutex);
        if(win_width_ != width() || win_height_ != height()) {
            //        DeInit();       // 释放尺寸缩放资源，等下一次draw的时候重新初始化
            //        win_width = width();
            //        win_height = height();
            req_resize_ = true;
        }
    }

    int DisplayWind::init_filters(const char *filters_descr, int width, int height, int format)
    {
        char args[512];
        int ret = 0;
        const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
        const AVFilter *buffersink = avfilter_get_by_name("buffersink");
        AVFilterInOut *outputs = avfilter_inout_alloc();
        AVFilterInOut *inputs  = avfilter_inout_alloc();
        enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
        
        filter_graph_ = avfilter_graph_alloc();
        if (!outputs || !inputs || !filter_graph_) {
            ret = AVERROR(ENOMEM);
            goto end;
        }

        // buffer video source: the decoded frames from the decoder will be inserted here
        snprintf(args, sizeof(args),
                "video_size=%dx%d:pix_fmt=%d:time_base=1/1:pixel_aspect=1/1",
                width, height, format);
        ret = avfilter_graph_create_filter(&buffersrc_ctx_, buffersrc, "in",
                                        args, NULL, filter_graph_);
        if (ret < 0) {
            LOG(ERROR) << "Cannot create buffer source";
            goto end;
        }

        // buffer video sink: to terminate the filter chain
        ret = avfilter_graph_create_filter(&buffersink_ctx_, buffersink, "out",
                                        NULL, NULL, filter_graph_);
        if (ret < 0) {
            LOG(ERROR) << "Cannot create buffer sink";
            goto end;
        }

        // 设置输出像素格式
        ret = av_opt_set_int_list(buffersink_ctx_, "pix_fmts", pix_fmts,
                                AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            LOG(ERROR) << "Cannot set output pixel format";
            goto end;
        }

        // Endpoints for the filter graph
        outputs->name       = av_strdup("in");
        outputs->filter_ctx = buffersrc_ctx_;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        inputs->name       = av_strdup("out");
        inputs->filter_ctx = buffersink_ctx_;
        inputs->pad_idx    = 0;
        inputs->next       = NULL;

        if ((ret = avfilter_graph_parse_ptr(filter_graph_, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
            goto end;

        if ((ret = avfilter_graph_config(filter_graph_, NULL)) < 0)
            goto end;

    end:
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);

        return ret;
    }
