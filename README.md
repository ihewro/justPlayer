# c++播放器

## 技术栈

* c++ 11+
* ffmpeg
* sdl

## 文件结构

ffmpegGrabber 打开文件流->解封装
processor 基本信息的初始化（比如流的index、上下文之类的）、解码、重编码
    * videoProcessor 重编码
    * audioProcessor 重采样
main 渲染图片和音画同步

### main

#### 画面播放

使用sdl 将avFrame，直接绘制在窗口中

#### 音频播放

？

### 音画同步

获取视频每帧的时间戳
和音频avframe的时间戳
每次播放的时候，比较视频时间戳和音频的时间戳，如果视频的时间戳小了，则加快速度播放，反之减低帧率播放


## 主要线程列表

* 主线程：等待sdl事件，绘制图像，播放声音（？）
* 线程1：read_frame 获取 avPacket
* 线程2：解码和重编码 获取avFrame
* 线程3：图像定时器，使用avFrame 激活sdl的事件
