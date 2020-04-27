/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:43
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#ifndef JUSTPLAYER_PROCESSOR_H
#define JUSTPLAYER_PROCESSOR_H

#include <iostream>
#include <string>
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
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include "libavutil/log.h"
}

/**
 * 视频流和音频公用的流程：
 * avPacket -> avFrame
 */

 using std::cout;
 using std::endl;
 using std::string;

class Processor {

public:

    AVFormatContext		*v_inputContext;//文件打开上下文

    AVStream * inputStream;//流
    int index = -1;//流序号

    AVCodecContext *decodeContext;//解码器


    std::atomic<uint64_t> currentTimestamp{0};
    std::atomic<uint64_t> nextFrameTimestamp{0};

    //设置解码器
    virtual bool setDecodeCtx();

    //设置编码器
    virtual bool setCovertCtx() = 0;

    //解封装
    void readPacket(AVPacket *inputPkt, AVFrame *inputFrame);

    //解码
    bool avP2F(bool &stopFlag, AVPacket *inputPkt, AVFrame *inputFrame);

    //重编码
    virtual void avFrameEncode(AVFrame *inputFrame) = 0;

    //采取数据 =  先解封装 + 解码 + 重编码，把这个函数放到单独线程里面执行
    void startGrab(bool &stopFlag);


    //释放packet和frame
    static void releasePAndF(AVPacket *inputPkt,AVFrame *inputFrame );


    //关闭解码器、编码器和文件流上下文
    void close();

    //获取时间戳
    uint64_t getPts();
};


#endif //JUSTPLAYER_PROCESSOR_H
