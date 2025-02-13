#ifndef DISPLAYWIND_H
#define DISPLAYWIND_H

#include <QWidget>
#include <QMutex>
#include "ijkmediaplayer.h"
#include "imagescaler.h"

// 添加 FFmpeg 滤镜相关头文件
extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

namespace Ui {
class DisplayWind;
}

class DisplayWind : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayWind(QWidget *parent = 0);
    ~DisplayWind();
    int Draw(const Frame *frame);
    void DeInit();
    void StartPlay();
    void StopPlay();
    void setFilterParams(float brightness,  // -1.0 到 1.0
                        float contrast,     // 0.0 到 3.0
                        int blurRadius,     // 0 到 20
                        float sharpAmount,  // 0.0 到 3.0
                        float hue,          // -180 到 180 度
                        float saturation,   // 0.0 到 3.0
                        float temperature); // 0.0 到 3.0 (1.0 是正常值)
    // 获取滤镜参数的方法
    float getBrightness() const { return brightness_; }
    float getContrast() const { return contrast_; }
    int getBlurRadius() const { return blurRadius_; }
    float getSharpAmount() const { return sharpAmount_; }
    float getHue() const { return hue_; }
    float getSaturation() const { return saturation_; }
    float getTemperature() const { return temperature_; }
    // 设置翻转
    void setFlip(bool horizontal, bool vertical) {
        flip_horizontal_ = horizontal;
        flip_vertical_ = vertical;
        update();
    }
    // 设置色调
    void setTint(float tint) {
        tint_ = tint;
        update();
    }
protected:
    // 这里不要重载event事件，会导致paintEvent不被触发
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event);
private:
    Ui::DisplayWind *ui;

    int m_nLastFrameWidth; ///< 记录视频宽高
    int m_nLastFrameHeight;
    bool is_display_size_change_ = false;

    int x_ = 0; //  起始位置
    int y_ = 0;
    int video_width = 0;
    int video_height = 0;
    int img_width = 0;
    int img_height = 0;
    int win_width_ = 0;
    int win_height_ = 0;
    bool req_resize_ = false;
    QImage img;
    VideoFrame dst_video_frame_;
    QMutex m_mutex;
    ImageScaler *img_scaler_ = NULL;

    int play_state_ = 0;    // 0 初始化状态; 1 播放状态; 2 停止状态

    // 滤镜参数
    float brightness_ = 0.0f;
    float contrast_ = 1.0f;
    int blurRadius_ = 0;
    float sharpAmount_ = 0.0f;
    float hue_ = 0.0f;
    float saturation_ = 1.0f;
    float temperature_ = 1.0f;
    bool flip_horizontal_ = false;  // 水平翻转
    bool flip_vertical_ = false;    // 垂直翻转
    float tint_ = 0.0f;            // 色调 (-1.0 到 1.0)

    // FFmpeg filter 相关
    AVFilterGraph *filter_graph_ = nullptr;
    AVFilterContext *buffersrc_ctx_ = nullptr;
    AVFilterContext *buffersink_ctx_ = nullptr;
    
    int init_filters(const char *filters_descr, int width, int height, int format);
};

#endif // DISPLAYWIND_H
