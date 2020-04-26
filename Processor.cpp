/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:43
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#include "include/Processor.h"

void Processor::avP2F() {


    ret = avcodec_send_packet(decodeVideoContext, inputPkt);

    if (ret == 0) {
//            av_packet_free(&targetPkt);
//            targetPkt = nullptr;
        cout << "[video] avcodec_send_packet success." << endl;
    } else{
        stopFlag = true;

        if (ret == AVERROR(EAGAIN)) {
            cout << "[avcodec_send_packet]send buff full" << endl;
            // buff full, can not decode any more, nothing need to do.
            // keep the packet for next time decode.
        } else if (ret == AVERROR_EOF) {
            // no new packets can be sent to it, it is safe.
            cout << "[WARN]  no new packets can be sent to it. index=" << this->videoIndex << endl;
        } else {
            string errorMsg = "+++++++++ ERROR avcodec_send_packet error: ";
            errorMsg += std::to_string(ret);
            cout << errorMsg << endl;
            throw std::runtime_error(errorMsg);
        }
        av_log(NULL, AV_LOG_INFO, "avcodec_send_packet fail:%d", ret);
        goto __END;
    }



    ret = avcodec_receive_frame(decodeVideoContext, inputFrame);

    if (ret == 0 || ret == AVERROR(EAGAIN)) {
        if (ret == AVERROR(EAGAIN)) {
            // need more packet.
            std::cout << "[avcodec_receive_frame失败] need more packet." << std::endl;
        }else{
            std::cout << "avcodec_receive_frame success." << std::endl;
            if (inputFrame!= nullptr){
                frameRGB = av_frame_alloc();
                av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer,
                                     AV_PIX_FMT_YUV420P, decodeVideoContext->width,
                                     decodeVideoContext->height, 32);
                frameRGB->format = AV_PIX_FMT_YUV420P;
                frameRGB->width = inputFrame->width;
                frameRGB->height = inputFrame->height;
                sws_scale(video_convert_ctx, (const uint8_t *const *) inputFrame->data,
                          inputFrame->linesize, 0, decodeVideoContext->height,
                          frameRGB->data, frameRGB->linesize);
                if (frameRGB) {
                    if (lock != nullptr && frameVec != nullptr) {
//                            lock->lock();
                        if (frameVec->empty()) {
                            frameVec->push_back(frameRGB);
                        } else {
                            av_frame_free(&frameRGB);
                        }
//                            lock->unlock();
                    }
                }
            }
        }
    } else {
        stopFlag = true;
        if (ret == AVERROR_EOF) {
            cout << "+++++++++++++++++++++++++++++ MediaProcessor no more output frames. index="
                 << this->videoIndex << endl;
        } else {
            string errorMsg = "avcodec_receive_frame error: ";
            errorMsg += ret;
            cout << errorMsg << endl;
            throw std::runtime_error(errorMsg);
        }

        av_log(NULL, AV_LOG_INFO, "avcodec_receive_frame fail:%d", ret);
        goto __END;
    }
}
