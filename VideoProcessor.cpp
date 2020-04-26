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

void VideoProcessor::setFramerate() {
    if (inputStream != nullptr && inputStream->r_frame_rate.den > 0) {
        frameRate = inputStream->r_frame_rate.num / inputStream->r_frame_rate.den;
    } else if (inputStream != nullptr && inputStream->r_frame_rate.den > 0) {
        frameRate = inputStream->r_frame_rate.num / inputStream->r_frame_rate.den;
    }
}

bool VideoProcessor::setDecodeCtx() {
    int ret = -1;
    bool flag = true;
    AVCodec *decodeVideo;//视频流的解码器

    //1. 查找解码器
    decodeVideo = avcodec_find_decoder(inputStream->codecpar->codec_id);
    if (!decodeVideo) {
        av_log(NULL, AV_LOG_INFO, "摄像头输入流解码器查找失败");
        flag = false;
    }else{//2. 解码器上下文创建
        decodeContext = avcodec_alloc_context3(decodeVideo);
        if (!decodeContext) {
            av_log(NULL, AV_LOG_INFO, "视频输入流解码器上下文分配内存失败");
            flag = false;
        }else{//3. 解码器参数复制
            ret = avcodec_parameters_to_context(decodeContext, inputStream->codecpar);
            if (ret < 0) {
                av_log(NULL, AV_LOG_INFO, "拷贝视频输入流解码器上下文参数失败:");
                flag = false;
            } else{//4. 打开解码器
                ret = avcodec_open2(decodeContext, decodeVideo, NULL);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_INFO, "打开视频输入流解码器失败:%d", ret);
                    flag = false;
                }
            }
        }
    }

    return flag;
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
