/**
  * User: hewro
  * Date: 2020/4/19
  * Time: 17:24
  * Description: 画面信息抓取
  */
#ifndef JUSTPLAYER_FFMPEGGRABBER_H
#define JUSTPLAYER_FFMPEGGRABBER_H

#include <iostream>
#include <string>
#include <utility>

#include<string>
#include <thread>
#include<vector>
#include<sstream>
#include<list>
#include<tuple>
#include "AudioProcessor.h"
#include "VideoProcessor.h"


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include<libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include<libavutil/time.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include "libavutil/log.h"
}

using std::string;
using std::mutex;
using std::vector;
class FFmpegGrabber {

public:
    FFmpegGrabber(const string& filePath);

    string filePath;
    bool openInput();//打开输入流
    bool openCodec();//初始化解码器和编码器

    bool start();//启动
    void startGrab();//循环解封装
    void close();//关闭时候一些释放操作
    AVFormatContext		*v_inputContext;//打开文件的上下文

    //音频流处理
    AudioProcessor *audioProcessor;
    //视频流处理
    VideoProcessor *videoProcessor;


    bool stopFlag = false;//文件读取结束标志，或者文件读取出错


    void setMutex(mutex *pMutex);
    void setVector(vector<AVFrame *> *vec);
};


#endif //JUSTPLAYER_FFMPEGGRABBER_H
