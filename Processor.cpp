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

bool Processor::avP2F(bool &stopFlag,AVPacket *inputPkt,AVFrame *inputFrame) {

    int flag = false;

    int ret = avcodec_send_packet(decodeVideoContext, inputPkt);

    if (ret == 0) {
//            av_packet_free(&targetPkt);
//            targetPkt = nullptr;
        cout << "[video] avcodec_send_packet success." << endl;
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


    ret = avcodec_receive_frame(decodeVideoContext, inputFrame);

    if (ret == 0 || ret == AVERROR(EAGAIN)) {
        if (ret == AVERROR(EAGAIN)) {
            // need more packet.
            std::cout << "[avcodec_receive_frame失败] need more packet." << std::endl;
        }else{
            std::cout << "avcodec_receive_frame success." << std::endl;
            //解码成功
        }
        flag = true;

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

//    Processor::releasePAndF(inputPkt,inputFrame);

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
