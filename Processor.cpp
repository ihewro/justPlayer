/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:43
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#include <thread>
#include "include/Processor.h"

bool Processor::avP2F(bool &stopFlag,AVPacket *inputPkt,AVFrame *inputFrame) {

    int flag = false;

    int ret = avcodec_send_packet(decodeContext, inputPkt);

    if (ret == 0) {
//            av_packet_free(&targetPkt);
//            targetPkt = nullptr;
//        cout << "[video] avcodec_send_packet success." << endl;
    } else{//文件结束了
        stopFlag = true;

        if (ret == AVERROR(EAGAIN)) {
            cout << "[avcodec_send_packet]send buff full" << endl;
            // buff full, can not decode any more, nothing need to do.
            // keep the packet for next time decode.
        } else if (ret == AVERROR_EOF) {
            // no new packets can be sent to it, it is safe.
            cout << "[WARN]  no new packets can be sent to it. index=" << this->index << endl;
        } else {
            string errorMsg = "+++++++++ ERROR avcodec_send_packet error: ";
            errorMsg += std::to_string(ret);
            cout << errorMsg << endl;
            throw std::runtime_error(errorMsg);
        }
        av_log(NULL, AV_LOG_INFO, "avcodec_send_packet fail:%d", ret);
    }


    ret = avcodec_receive_frame(decodeContext, inputFrame);

    if (ret == 0 || ret == AVERROR(EAGAIN)) {
        if (ret == AVERROR(EAGAIN)) {
            // need more packet.
            std::cout << "[avcodec_receive_frame失败] need more packet." << std::endl;
        }else{
//            std::cout << "avcodec_receive_frame success." << std::endl;
            //解码成功
            flag = true;
        }

    } else {
        stopFlag = true;
        if (ret == AVERROR_EOF) {
            cout << "+++++++++++++++++++++++++++++ MediaProcessor no more output frames. index="
                 << this->index << endl;
        } else {
            string errorMsg = "avcodec_receive_frame error: ";
            errorMsg += ret;
            cout << errorMsg << endl;
            throw std::runtime_error(errorMsg);
        }

        av_log(NULL, AV_LOG_INFO, "avcodec_receive_frame fail:%d", ret);
    }

    return flag;

}


void Processor::releasePAndF(AVPacket *inputPkt, AVFrame *inputFrame) {
    if (inputPkt) {
        av_packet_free(&inputPkt);
    }
    if (inputFrame) {
        av_frame_free(&inputFrame);
    }
}

bool Processor::setDecodeCtx() {
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

void Processor::readPacket(AVPacket *inputPkt, AVFrame *inputFrame) {

    av_init_packet(inputPkt);
//        AVPacket* inputPkt = (AVPacket*)av_malloc(sizeof(AVPacket));
    int ret = av_read_frame(v_inputContext, inputPkt);
    if (ret < 0) {
        av_log(NULL, AV_LOG_INFO, "读取视频图像失败:%d", ret);
        Processor::releasePAndF(inputPkt,inputFrame);
        //todo:抛出一个运行时错误
    }else{
//        cout << "获取avPacket成功" << endl;
    }


}



void Processor::close() {
    if (decodeContext) {
        avcodec_close(decodeContext);
        avcodec_free_context(&decodeContext);
    }

    //todo:写到析构函数里面
//    sws_freeContext(convert_ctx);

    if (v_inputContext) {
        avformat_close_input(&v_inputContext);
        //avformat_free_context(v_inputContext);
    }

}

void Processor::startGrab(bool &stopFlag) {
//    cout << "startGrab" << endl;
    stopFlag = false;
    int ret;
    while (true) {
        if (stopFlag) {
            close();
            break;
        }
        AVPacket *inputPkt = av_packet_alloc();
        AVFrame *inputFrame = av_frame_alloc();

        readPacket(inputPkt,inputFrame);

        if (inputPkt->stream_index == index){
            bool flag = avP2F(stopFlag, inputPkt, inputFrame);
            if(flag){
                //重编码
                avFrameEncode(inputFrame);
            }
        }

        Processor::releasePAndF(inputPkt,inputFrame);

        //不加这一行会出现加速的问题，不知道什么原因，所以av_send av_receive 最好还是分成两个线程
        //时间问题，就这样了
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }
}

uint64_t Processor::getPts() {
    return currentTimestamp.load();
}
