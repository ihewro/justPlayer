/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:44
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#include "include/VideoProcessor.h"

void VideoProcessor::avFrameEncode(AVFrame *inputFrame) {
    //重编码

    if (inputFrame!= nullptr){
        AVFrame *frameRGB = nullptr;
        frameRGB = av_frame_alloc();
        av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer,
                             AV_PIX_FMT_YUV420P, decodeContext->width,
                             decodeContext->height, 32);
        frameRGB->format = AV_PIX_FMT_YUV420P;
        frameRGB->width = inputFrame->width;
        frameRGB->height = inputFrame->height;
        sws_scale(convert_ctx, (const uint8_t *const *) inputFrame->data,
                  inputFrame->linesize, 0, decodeContext->height,
                  frameRGB->data, frameRGB->linesize);
        if (frameRGB) {
            if (lock != nullptr && frameVec != nullptr) {
                            lock->lock();
                if (frameVec->empty()) {
                    frameVec->push_back(frameRGB);
                } else {
                    av_frame_free(&frameRGB);
                }
                            lock->unlock();
            }
        }
    }


}
