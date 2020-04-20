/**
  * User: hewro
  * Date: 2020/4/19
  * Time: 17:24
  * Description: 
  */
//
// Created by hewro on 2020/4/19.
//

#include "VideoGrabber.h"
using std::cout;
using std::endl;

bool VideoGrabber::start() {

    if (!openInput()) {
        av_log(NULL, AV_LOG_ERROR, "输入流打开失败");
        return false;
    }

    if (!openCodec()) {
        av_log(NULL, AV_LOG_ERROR, "解码器打开失败");
        return false;
    }
    cout << "ok" << endl;

    std::thread videoThread{&VideoGrabber::startGrab, this};
    videoThread.detach();

    return true;
}

void VideoGrabber::startGrab() {
    cout << "startGrab" << endl;
    stopFlag = false;
    int ret;
    while (true) {
        if (stopFlag) {
            close();
            break;
        }
        AVPacket *inputPkt = av_packet_alloc();
        AVFrame *inputFrame = av_frame_alloc();
        AVFrame *frameRGB = nullptr;
        av_init_packet(inputPkt);
        ret = av_read_frame(v_inputContext, inputPkt);
        if (ret < 0) {
            av_log(NULL, AV_LOG_INFO, "读取摄像头图像失败:%d", ret);
            goto __END;
        }else{
            cout << "获取avPacket成功" << endl;
        }

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
            std::cout << "avcodec_receive_frame success." << std::endl;
            if (ret == AVERROR(EAGAIN)) {
                // need more packet.
                std::cout << "[avcodec_receive_frame失败] need more packet." << std::endl;
            }else{
                std::cout << "2333?" << std::endl;
                if (inputFrame!= nullptr){
                    frameRGB = av_frame_alloc();
                    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer,
                                         AV_PIX_FMT_RGB24, decodeVideoContext->width, decodeVideoContext->height, 32);
                    frameRGB->format = AV_PIX_FMT_RGB24;
                    frameRGB->width = inputFrame->width;
                    frameRGB->height = inputFrame->height;
                    sws_scale(video_convert_ctx, (const uint8_t *const *) inputFrame->data,
                              inputFrame->linesize, 0, decodeVideoContext->height, frameRGB->data, frameRGB->linesize);
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


        __END:
        if (inputPkt) {
            av_packet_free(&inputPkt);
        }
        if (inputFrame) {
            av_frame_free(&inputFrame);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }

}

void VideoGrabber::close() {
    if (decodeVideoContext) {
        avcodec_close(decodeVideoContext);
        avcodec_free_context(&decodeVideoContext);
    }

    sws_freeContext(video_convert_ctx);

    if (v_inputContext) {
        avformat_close_input(&v_inputContext);
        //avformat_free_context(v_inputContext);
    }

}


bool VideoGrabber::openInput() {
    int ret = -1;
    bool flag = true;

    AVDictionary *format_opts = nullptr;
    av_dict_set_int(&format_opts, "rtbufsize", 3041280 * 100, 0);

    AVInputFormat *ifmt = av_find_input_format("avfoundation");
    //mac 下需要添加这个参数
    av_dict_set(&format_opts, "pixel_format", "uyvy422", 0);
    av_dict_set(&format_opts, "video_size", "640x480", 0);
    av_dict_set_int(&format_opts, "framerate", 30, 0);


    cout << this->filePath.c_str() << endl;
    ret = avformat_open_input(&v_inputContext, "0", ifmt, &format_opts);
//    ret = avformat_open_input(&v_inputContext, this->filePath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        cout << "打开文件失败" + std::to_string(ret) << endl;
        flag = false;
        goto __END;
    }
    ret = avformat_find_stream_info(v_inputContext, nullptr);
    if (ret < 0) {
        av_log(NULL, AV_LOG_INFO, "查找文件的流失败:");
        flag = false;
        goto __END;
    }
    cout << "查找文件的流成功" << endl;

    for (int i = 0; i < v_inputContext->nb_streams; ++i) {
        if (v_inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            inputVideoStream = v_inputContext->streams[i];
            break;
        }
    }

    if (videoIndex < 0) {
        flag = false;
        av_log(NULL, AV_LOG_INFO, "文件流的视频流查找失败");
        goto __END;
    }

    cout << "文件流的视频流查找成功" << endl;

//    av_log(NULL, AV_LOG_INFO, "文件流的视频流查找成功\n");

    if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0) {
        frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
    } else if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0) {

        frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
    }

    av_dump_format(v_inputContext, 0, this->filePath.c_str(), 0);


    __END:
    if (!flag) {
        if (v_inputContext) {
            avformat_close_input(&v_inputContext);
        }
        av_log(NULL, AV_LOG_INFO, "输入初始化失败");
    }
    return flag;
}

bool VideoGrabber::openCodec() {
    bool flag = true;
    int ret = -1;

    decodeVideo = avcodec_find_decoder(inputVideoStream->codecpar->codec_id);
    if (!decodeVideo) {
        av_log(NULL, AV_LOG_INFO, "摄像头输入流解码器查找失败");
        flag = false;
        goto __END;
    }
    decodeVideoContext = avcodec_alloc_context3(decodeVideo);
    if (!decodeVideoContext) {
        av_log(NULL, AV_LOG_INFO, "视频输入流解码器上下文分配内存失败");
        flag = false;
        goto __END;
    }

    ret = avcodec_parameters_to_context(decodeVideoContext, inputVideoStream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_INFO, "拷贝视频输入流解码器上下文参数失败:");
        flag = false;
        goto __END;
    }
//    decodeVideoContext->thread_count = 2;
    ret = avcodec_open2(decodeVideoContext, decodeVideo, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_INFO, "打开视频输入流解码器失败:%d", ret);
        flag = false;
        goto __END;
    }

    out_buffer = (uint8_t *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_RGB24, decodeVideoContext->width, decodeVideoContext->height, 32) *
            sizeof(uint8_t));
    //重编码器
    video_convert_ctx = sws_getContext(decodeVideoContext->width, decodeVideoContext->height,
                                       decodeVideoContext->pix_fmt, decodeVideoContext->width,
                                       decodeVideoContext->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    __END:
    if (!flag) {
        if (decodeVideoContext) {
            avcodec_free_context(&decodeVideoContext);
        }
        av_log(NULL, AV_LOG_INFO, "初始化编码器失败");
    }
    return flag;
}

void VideoGrabber::setMutex(mutex *pMutex) {
    lock = pMutex;
}

void VideoGrabber::setVector(vector<AVFrame *> *vec) {
    frameVec = vec;
}
