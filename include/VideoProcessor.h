/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:44
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#ifndef JUSTPLAYER_VIDEOPROCESSOR_H
#define JUSTPLAYER_VIDEOPROCESSOR_H


#include "Processor.h"
#include <vector>

using std::vector;
using std::mutex;

class VideoProcessor: public Processor  {
public:

    uint8_t *out_buffer = nullptr;
    mutex *lock;
    void avFrameEncode(AVFrame *inputFrame) override;
    vector<AVFrame *> *frameVec;//存储视频流中的帧
    int frameRate = 30;//视频的帧率

    SwsContext *convert_ctx;//重编码器

    void setFramerate();

    bool setCovertCtx() override;

};


#endif //JUSTPLAYER_VIDEOPROCESSOR_H
