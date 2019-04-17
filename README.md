# FFmpegAndroidPlayer
This is a project based on complied ffmpeg &amp;&amp; ffprobe,OpenSLES intended to build a simple video player
这是一个嵌套FFmpeg的项目,FFmpeg版本4.0.2,ndk版r17c,目标是实现一个简单的音视频播放器以及各种特效实现
# Build
FFmpeg的编译参考了[FFmpeg Build](https://github.com/inFullMobile/videokit-ffmpeg-android),其中我对build.sh进行了适配ubuntu18.0.4的修改
## Progress 当前进度
成功编译集成FFmpeg后,编写了FFSurfaceView,并使用它获取视频流进行了播放
成功使用OpenSLES播放本地音频,下一步目标是播放原始pcm数据以连接ffmpeg进行播放音频与音画同步
思路:OpenSLES+bufferedQueue+pcm
成功播放pcm数据,
