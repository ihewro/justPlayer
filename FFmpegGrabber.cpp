/**
  * User: hewro
  * Date: 2020/4/19
  * Time: 17:24
  * Description: 
  */
//
// Created by hewro on 2020/4/19.
//

#include "include/FFmpegGrabber.h"
using std::cout;
using std::endl;

bool FFmpegGrabber::start() {

    if (!openInput()) {
        av_log(NULL, AV_LOG_ERROR, "输入流打开失败");
        return false;
    }

    if (!openCodec()) {
        av_log(NULL, AV_LOG_ERROR, "解码器打开失败");
        return false;
    }

    std::thread videoThread{&FFmpegGrabber::startGrab, this};
    videoThread.detach();

    return true;
}

void FFmpegGrabber::startGrab() {
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
        av_init_packet(inputPkt);
//        AVPacket* inputPkt = (AVPacket*)av_malloc(sizeof(AVPacket));
        ret = av_read_frame(v_inputContext, inputPkt);
        if (ret < 0) {
            av_log(NULL, AV_LOG_INFO, "读取视频图像失败:%d", ret);
            Processor::releasePAndF(inputPkt,inputFrame);
            //todo:抛出一个运行时错误
        }else{
            cout << "获取avPacket成功" << endl;
        }

        if (inputPkt->stream_index == videoProcessor->index){
            bool flag = videoProcessor->avP2F(stopFlag, inputPkt, inputFrame);
            if(flag){
                //重编码
                videoProcessor->avFrameEncode(inputFrame);
            }
        }else{
            //音频的avPacket处理

        }

        Processor::releasePAndF(inputPkt,inputFrame);

        //不加这一行会出现加速的问题，不知道什么原因，所以av_send av_receive 最好还是分成两个线程
        //时间问题，就这样了
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }

}



void FFmpegGrabber::close() {
    if (videoProcessor->decodeContext) {
        avcodec_close(videoProcessor->decodeContext);
        avcodec_free_context(&videoProcessor->decodeContext);
    }

    sws_freeContext(videoProcessor->convert_ctx);

    if (v_inputContext) {
        avformat_close_input(&v_inputContext);
        //avformat_free_context(v_inputContext);
    }

}


bool FFmpegGrabber::openInput() {
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
//    ret = avformat_open_input(&v_inputContext, "0", ifmt, &format_opts);
    ret = avformat_open_input(&v_inputContext, this->filePath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        cout << "打开文件失败" + std::to_string(ret) << endl;
        flag = false;
    }

    ret = avformat_find_stream_info(v_inputContext, nullptr);
    if (ret < 0) {
        av_log(NULL, AV_LOG_INFO, "查找文件的流失败:");
        flag = false;
    }else{
        cout << "查找文件的流成功" << endl;
        for (int i = 0; i < v_inputContext->nb_streams; ++i) {
            if (v_inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoProcessor->index = i;
                cout << "video stream index = : [" << i << "]" << endl;
                videoProcessor->inputStream = v_inputContext->streams[i];
            }

            if (v_inputContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioProcessor->index == -1) {
                audioProcessor->index = i;
                audioProcessor->inputStream = v_inputContext->streams[i];
                cout << "audio stream index = : [" << i << "]" << endl;
            }
        }
        if (videoProcessor->index < 0) {
            flag = false;
            av_log(NULL, AV_LOG_INFO, "文件流的视频流查找失败");
        }

        if (audioProcessor->index < 0) {
            flag = false;
            av_log(NULL, AV_LOG_INFO, "文件流的音频流查找失败");
        }

        cout << "文件流的视频流查找成功" << endl;
    }


    if(flag){//打开设备成功
        //设置帧率
        videoProcessor->setFramerate();
        av_dump_format(v_inputContext, 0, this->filePath.c_str(), 0);
    }else{
        if (v_inputContext) {
            avformat_close_input(&v_inputContext);
        }
        av_log(NULL, AV_LOG_INFO, "输入初始化失败");
    }

    return flag;
}

bool FFmpegGrabber::openCodec() {
    bool flag = videoProcessor->setDecodeCtx();
    videoProcessor->setCovertCtx();
    if (!flag) {
        if (videoProcessor->decodeContext) {
            avcodec_free_context(&videoProcessor->decodeContext);
        }
        av_log(NULL, AV_LOG_INFO, "初始化编码器失败");
    }
    return flag;
}

void FFmpegGrabber::setMutex(mutex *pMutex) {
    videoProcessor->lock = pMutex;
}

void FFmpegGrabber::setVector(vector<AVFrame *> *vec) {
    videoProcessor->frameVec = vec;
}


//构造函数
FFmpegGrabber::FFmpegGrabber(const string &filePath) {
    avdevice_register_all();
    this->filePath = filePath;

    v_inputContext = nullptr;
    stopFlag = false;
    this->v_inputContext = avformat_alloc_context();

    audioProcessor = new AudioProcessor();
    videoProcessor = new VideoProcessor();


    std::cout <<  "file: " <<filePath << std::endl;
}
