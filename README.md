# 1 项目简介

项目描述：目采用UI和播放器核心分离的模式，将播放器适配到PC（Win/Ubuntu/MAC/）；支持打开本地视频文件(如mp4、mov、av等)、网络视频流(rtsp、rtmp、http等)；针对直播流，实现直播低延迟播放。

主要工作：
1.优化视频播放器的音视频同步： 基于JitterBuffer 机制的缓冲控制和低延迟优化；当缓存超过阈值时启动变速播放来消耗缓存，每 500ms 轮询，查询音频和视频缓存数据；缓存阀值数据大小可根据业务调整。

2.通过消息队列机制实现模块间的解耦通信；采用UI和播放器核心分离的模式，方便进行移植和维护；

3.使用SDL实现跨平台音频输出，利用Sonic算法实现变速播放但不改变音调；

4. 使用Qt的QPainter实现视频渲染，并利用FFmpeg滤镜进行图像处理。支持通过FFmpeg滤镜实现画面的旋转、色调与饱和度调整、翻转以及模糊处理（包括boxblur盒式模糊和gblur高斯模糊）；支持unsharp锐化滤镜，用于增强画面细节。


# 2 操作说明




# 部分功能实现原理

## seek操作

1. 需要先获取到当前视频的播放时间
   1. 解复用后得到当前视频的总时长
   2. ui主动去获取时长
   3. ui显示更新时长
   4. ijkplayer
      1. IjkMediaPlayer_getDuration
      2. ijkmp_get_duration
      3. ffp_get_duration_l实际调用is->ic->duration
2. 进度条seek都某个位置，将位置比例换算成要seek的播放位置。
   1. FFP_REQ_SEEK
   2. ffp_seek_to_l
   3. stream_seek



## 播放完毕检查

检测条件

1. av_read_frame返回AVERROR_EOF，将eof变量设置为1
2. 检测audio是否还有数据输出，如果没有则将audio_no_data设置为1
3. 检测video是否还有数据输出，如果没有则将video_no_data设置为1



检测方法，分音视频同时存在、只存在音频、只存在视频的场景，如果检测到上述变量==1成立，发送FFP_MSG_PLAY_FNISH消息：

```c
void FFPlayer::check_play_finish()
{
    //    LOG(INFO) << "eof: " << eof << ", audio_no_data: " << audio_no_data  ;
    if(eof == 1) { // 1. av_read_frame已经返回了AVERROR_EOF
        if(audio_stream >= 0 && video_stream >= 0) { // 2.1 音频、视频同时存在的场景
            if(audio_no_data == 1 && video_no_data == 1) {
                // 发送停止
                ffp_notify_msg1(this, FFP_MSG_PLAY_FNISH);
            }
            return;
        }
        if(audio_stream >= 0) { // 2.2 只有音频存在
            if(audio_no_data == 1) {
                // 发送停止
                ffp_notify_msg1(this, FFP_MSG_PLAY_FNISH);
            }
            return;
        }
        if(video_stream >= 0) { // 2.3 只有视频存在
            if(video_no_data == 1) {
                // 发送停止
                ffp_notify_msg1(this, FFP_MSG_PLAY_FNISH);
            }
            return;
        }
    }
}
```



UI处理，通过发送信号sig_stopped  触发 **HomeWindow::sto**p的调用，因为消息线程不适合设置ui：

```c++
case FFP_MSG_PLAY_FNISH:
        tips.sprintf("播放完毕");
        emit sig_showTips(Toast::INFO, tips);
        // 发送播放完毕的信号触发调用停止函数
        emit sig_stopped(); // 触发停止
        break;
```



# 第三方库

## easylogging

| Level   | Description                                                  |
| :------ | :----------------------------------------------------------- |
| Global  | Generic level that represents all levels. Useful when setting global configuration for all levels. |
| Trace   | Information that can be useful to back-trace certain events - mostly useful than debug logs. |
| Trace   | Information that can be useful to back-trace certain events - mostly useful than debug logs. |
| Fatal   | Very severe error event that will presumably lead the application to abort. |
| Error   | Error information but will continue application to keep running. |
| Warning | Information representing errors in application but application will keep running. |
| Info    | Mainly useful to represent current progress of application.  |
| Verbose | Information that can be highly useful and vary with verbose logging level. Verbose logging is not applicable to hierarchical logging. |
| Unknown | Only applicable to hierarchical logging and is used to turn off logging completely. |

Global级别，个概念性的级别，不能应用于实际的日志记录，也就是说不能用宏 LOG(GLOBLE) 进行日志记录。在划分级别的日志记录中，设置门阀值为 el::Level::Global 表示所有级别的日志都生效。

  ·Trace级别，不过实际验证发现，不论是debug还是release版本，Trace级别的日记都会生效。

  ·Debug级别，只在debug模式生效，在Release模式会自动屏蔽该级别所有的日志记录。

  ·Fatal级别，默认情况下会使程序中断，可设置标记 LoggingFlag::DisableApplicationAbortOnFatalLog 来阻止中断。

  ·Verbose级别，可以更加详细地记录日志信息，但不适用于划分级别的日志记录，意思就是说即使门阀值设置大于该级别，该级别的日志记录同样生效。同时，该级别只能用宏VLOG而不能用宏 LOG(VERBOSE) 进行日志记录，并且在默认情况下，只有VLOG(0)日志记录生效。

   ·Unknown级别，同样也是一个概念性的级别，不能用宏 LOG(UNKNOWN) 进行日志记录。该级别只适用于在划分级别的日志记录中，如果设置门阀值为 el::Level::Unknown ，那么就表示所有级别的日志记录都会被完全屏蔽，需要注意的是，Verbose 级别不受此影响。但是如果程序没有设置划分级别标记：LoggingFlag::HierarchicalLogging，那么即使设置了
————————————————
版权声明：本文为CSDN博主「冷月醉雪」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/lengyuezuixue/article/details/79230166



```
enum class Level : base::type::EnumType {
  /// @brief Generic level that represents all the levels. Useful when setting global configuration for all levels
  Global = 1,
  /// @brief Information that can be useful to back-trace certain events - mostly useful than debug logs.
  Trace = 2,
  /// @brief Informational events most useful for developers to debug application
  Debug = 4,
  /// @brief Severe error information that will presumably abort application
  Fatal = 8,
  /// @brief Information representing errors in application but application will keep running
  Error = 16,
  /// @brief Useful when application has potentially harmful situations
  Warning = 32,
  /// @brief Information that can be highly useful and vary with verbose logging level.
  Verbose = 64,
  /// @brief Mainly useful to represent current progress of application
  Info = 128,
  /// @brief Represents unknown level
  Unknown = 1010
};
```



致命信息打印：Fatal

错误信息打印：Error

警告信息打印：Warning

通用信息打印：Verbose

跟踪信息打印：Info
