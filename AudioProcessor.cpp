/**
  * User: hewro
  * Date: 2020/4/23
  * Time: 22:34
  * Description: 
  */
//
// Created by hewro on 2020/4/23.
//

#include "include/AudioProcessor.h"



void AudioProcessor::avFrameEncode(AVFrame *frame) {
    //1. 分配outBuffer空间
    uint8_t* dataBuffer = nullptr;
    int dataBufferSize = allocBuffer(frame->nb_samples, &dataBuffer);

    cout << "dataBufferSize" << dataBufferSize << endl;

    //2. 重采样
    outSamples = swr_convert(convert_ctx, &dataBuffer, dataBufferSize,
                                 (const uint8_t**)&frame->data[0], frame->nb_samples);
    // cout << "reSample: nb_samples=" << frame->nb_samples << ", sample_rate = " <<
    // frame->sample_rate <<  ", outSamples=" << outSamples << endl;
    if (outSamples <= 0) {
        throw std::runtime_error("error: outSamples=" + outSamples);
    }

    outDataSize =
            av_samples_get_buffer_size(NULL, out.channels, outSamples, out.format, 1);

    if (outDataSize <= 0) {
        throw std::runtime_error("error: outDataSize=" + outDataSize);
    }else{
//        if (outBufferVec->empty()) {
//            outBufferVec->push_back(dataBuffer);
//        } else {
//            av_frame_free(&frameRGB);
//        }
        outBufferVec.push_back(dataBuffer);

    }


}


bool AudioProcessor::setCovertCtx() {
    convert_ctx = swr_alloc_set_opts(nullptr, out.layout, out.format, out.sampleRate, in.layout,
                             in.format, in.sampleRate, 0, nullptr);
    if (swr_init(convert_ctx)) {
        throw std::runtime_error("swr_init error.");
    }
    return true;
}

void AudioProcessor::writeAudioData(unsigned char *stream, int len) {
    static uint8_t* silenceBuff = nullptr;
    if (silenceBuff == nullptr) {
        silenceBuff = (uint8_t*)av_malloc(sizeof(uint8_t) * len);
        std::memset(silenceBuff, 0, len);
    }

    if (!outBufferVec.empty()) {
        //向流中写入数据
        currentTimestamp.store(nextFrameTimestamp.load());
        auto outBuffer = outBufferVec.back();
//        if (outDataSize != len) {
//            cout << "WARNING: outDataSize[" << outDataSize << "] != len[" << len << "]" << endl;
//        }
        std::memcpy(stream, outBuffer, outDataSize);
//        isNextDataReady.store(false);
    } else {
        // if list is empty, silent will be written.
        cout << "WARNING: writeAudioData, audio data not ready." << endl;
        std::memcpy(stream, silenceBuff, len);
    }
}

int AudioProcessor::allocBuffer(int inputSamples, uint8_t **dataBuffer) {
    int bytePerOutSample = -1;
    switch (out.format) {
        case AV_SAMPLE_FMT_U8:
            bytePerOutSample = 1;
            break;
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S16:
            bytePerOutSample = 2;
            break;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            bytePerOutSample = 4;
            break;
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_DBLP:
        case AV_SAMPLE_FMT_S64:
        case AV_SAMPLE_FMT_S64P:
            bytePerOutSample = 8;
            break;
        default:
            bytePerOutSample = 2;
            break;
    }
    cout << "inputSamples" << inputSamples << endl;
    cout << "out.sampleRate" << out.sampleRate << endl;
    cout << "in.sampleRate" << in.sampleRate << endl;

    int guessOutSamplesPerChannel =
            av_rescale_rnd(inputSamples, out.sampleRate, in.sampleRate, AV_ROUND_UP);

    cout << "bytePerOutSample" << bytePerOutSample << endl;
    cout << "guessOutSamplesPerChannel" << guessOutSamplesPerChannel << endl;
    int guessOutSize = guessOutSamplesPerChannel * out.channels * bytePerOutSample;

    std::cout << "GuessOutSamplesPerChannel: " << guessOutSamplesPerChannel << std::endl;
    std::cout << "GuessOutSize: " << guessOutSize << std::endl;

    guessOutSize *= 1.2;  // just make sure.

    *dataBuffer = (unsigned char *)av_malloc(sizeof(uint8_t) * guessOutSize);

    // av_samples_alloc(&outData, NULL, outChannels, guessOutSamplesPerChannel,
    // AV_SAMPLE_FMT_S16, 0);
    return guessOutSize;
}

bool AudioProcessor::setDecodeCtx() {
    bool flag=  Processor::setDecodeCtx();
    if (flag){
        //设置音频流的信息
        int64_t inLayout = decodeContext->channel_layout;
        int inSampleRate = decodeContext->sample_rate;
        int inChannels = decodeContext->channels;
        AVSampleFormat inFormat = decodeContext->sample_fmt;

        in = AudioInfo(inLayout, inSampleRate, inChannels, inFormat);
        out = AudioInfo(inSampleRate);
    }
}


