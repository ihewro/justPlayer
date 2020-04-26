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

class AudioProcessor: Processor {

    int					audioIndex;
    AVCodec				*decodeAudio;
    AVCodecContext		*decodeAudioContext;
    AVFormatContext		*a_inputContext;
    AVStream			*inputAudioStream;
    AVCodecID			m_audio_codec_id;
    bool				stopFlag;

public:

    void avFrameEncode() override;

};


#endif //JUSTPLAYER_AUDIOPROCESSOR_H
