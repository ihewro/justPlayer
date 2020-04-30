/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:44
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#include <thread>
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
                    cout << "frameVec push back" << endl;
                    frameVec->push_back(frameRGB);
                } else {
//                    cout << "size:" << frameVec->size()<<endl;
                    av_frame_free(&frameRGB);
//                    cout << "size2:" << frameVec->size()<<endl;
                }
                            lock->unlock();
            }
        }
    }


}

void VideoProcessor::setFramerate() {
    if (inputStream != nullptr && inputStream->r_frame_rate.den > 0) {
        frameRate = inputStream->r_frame_rate.num / inputStream->r_frame_rate.den;
    } else if (inputStream != nullptr && inputStream->r_frame_rate.den > 0) {
        frameRate = inputStream->r_frame_rate.num / inputStream->r_frame_rate.den;
    }
}


bool VideoProcessor::setCovertCtx() {

    out_buffer = (uint8_t *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                     decodeContext->width,
                                     decodeContext->height, 32) *
            sizeof(uint8_t));
    //重编码器
    convert_ctx = sws_getContext(
            decodeContext->width,
            decodeContext->height,
            decodeContext->pix_fmt,
            decodeContext->width,
            decodeContext->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC, NULL, NULL, NULL);

    return true;
}


