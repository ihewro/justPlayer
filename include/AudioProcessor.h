/**
  * User: hewro
  * Date: 2020/4/23
  * Time: 22:34
  * Description: 
  */
//
// Created by hewro on 2020/4/23.
//

#ifndef JUSTPLAYER_AUDIOPROCESSOR_H
#define JUSTPLAYER_AUDIOPROCESSOR_H
#include <iostream>
#include <string>
#include <utility>

#include<string>
#include <thread>
#include<vector>
#include<sstream>
#include<list>
#include<tuple>
#include "Processor.h"


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

using std::cout;
using std::endl;

struct AudioInfo {


    int64_t layout;
    int sampleRate;
    int channels;
    AVSampleFormat format;

    AudioInfo() {
        layout = -1;
        sampleRate = -1;
        channels = -1;
        format = AV_SAMPLE_FMT_S16;
    }

    AudioInfo(int sr) {
        layout = AV_CH_LAYOUT_STEREO;
        sampleRate = sr;
        channels = 2;
        format = AV_SAMPLE_FMT_S16;
    }


    AudioInfo(int64_t l, int rate, int c, AVSampleFormat f)
            : layout(l), sampleRate(rate), channels(c), format(f) {}
};



class AudioProcessor: public Processor {


public:

    SwrContext *convert_ctx;//重编码器
    //todo 考虑使用智能指针
    std::vector<uint8_t* > *outBufferVec;//存储等待sdl回调函数调用的数据
    AudioInfo in;//输入音频流的元信息
    AudioInfo out;//重采样后的音频流的元信息

    int outDataSize = -1; //重采样后数据的大小
    //每一个声道的样本数
    int outSamples = -1;//number of samples output per channel, negative value on error


    bool setCovertCtx() override;
    void avFrameEncode(AVFrame *inputFrame) override;
    bool setDecodeCtx() override;


    void writeAudioData(unsigned char *string, int i);//sdl 回调函数，向sdl流中写入数据


    //确定dataBuffer 的大小并分配空间，以便转换的时候将 avframe里面的data转到buffer中
    //返回outbuffer大小
    int allocBuffer(int inputSamples, uint8_t **dataBuffer);
};





#endif //JUSTPLAYER_AUDIOPROCESSOR_H
