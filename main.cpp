#include <iostream>

#include <string>
#include "include/FFmpegGrabber.h"

using std::string;
using std::cout;
using std::endl;
extern "C" {
#include "SDL2/SDL.h"
};

void playSdlVideo(FFmpegGrabber* videoGrabber, mutex *pMutex);
void picRefresher(int timeInterval, bool& exitRefresh);

int main() {
    string filePath = "/Users/hewro/Downloads/产品介绍4.mp4";
    std::cout << "Hello, World!" << std::endl;
    std::mutex			mtx{};//帧锁，避免frameVector出现线程冲突问题
    vector<AVFrame*>	frameVec{};//存储视频帧
    //启动视频抓取
    auto* videoGrabber = new FFmpegGrabber(filePath);
    videoGrabber->setMutex(&mtx);
    videoGrabber->setVector(&frameVec);
    videoGrabber->start();

    //播放视频
    playSdlVideo(videoGrabber, &mtx);

    return 0;
}


void playSdlVideo(FFmpegGrabber* videoGrabber, mutex *pMutex){
    //播放视频
    SDL_Window* screen;

    cout << "width" << videoGrabber-> videoProcessor->decodeContext->width << endl;
    cout << "height" << videoGrabber-> videoProcessor-> decodeContext->height << endl;
    int width = videoGrabber->videoProcessor-> decodeContext->width;
    int height = videoGrabber-> videoProcessor->decodeContext->height;
    // SDL 2.0 Support for multiple windows
    screen = SDL_CreateWindow("justPlayer", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, width, height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!screen) {
        string errMsg = "SDL: could not create window - exiting:";
        errMsg += SDL_GetError();
        cout << errMsg << endl;
        throw std::runtime_error(errMsg);
    }

    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    Uint32 pixformat = SDL_PIXELFORMAT_IYUV;

    SDL_Texture* sdlTexture =
            SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_Event event;

    cout << "帧率" << videoGrabber->videoProcessor->frameRate << endl;
    //定时器刷新图片的定时器
    std::thread refreshThread{picRefresher, (int)(1000 / videoGrabber->videoProcessor->frameRate), std::ref(videoGrabber->stopFlag)};

    while (true){
        //等待sdl 事件
        SDL_WaitEvent(&event);

        if (videoGrabber->stopFlag){
            break;
        }
        if (!videoGrabber->videoProcessor->frameVec->empty()){
            cout << "not empty" << endl;
            AVFrame* frame = videoGrabber->videoProcessor->frameVec->back();
            if (frame != nullptr) {
                //显示画面
                SDL_UpdateYUVTexture(sdlTexture,  // the texture to update
                                     NULL,        // a pointer to the rectangle of pixels to update, or
                        // NULL to update the entire texture
                                     frame->data[0],      // the raw pixel data for the Y plane
                                     frame->linesize[0],  // the number of bytes between rows of pixel
                        // data for the Y plane
                                     frame->data[1],      // the raw pixel data for the U plane
                                     frame->linesize[1],  // the number of bytes between rows of pixel
                        // data for the U plane
                                     frame->data[2],      // the raw pixel data for the V plane
                                     frame->linesize[2]   // the number of bytes between rows of pixel
                        // data for the V plane
                );
                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                SDL_RenderPresent(sdlRenderer);
            }
        }else{
            //
            cout << "empty" << endl;
        }
    }

    refreshThread.join();
}

#define REFRESH_EVENT (SDL_USEREVENT + 1)


//定时器
void picRefresher(int timeInterval, bool& exitRefresh) {
    cout << "picRefresher timeInterval[" << timeInterval << "]" << endl;
    while (!exitRefresh) {
        SDL_Event event;
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        std::this_thread::sleep_for(std::chrono::milliseconds(timeInterval));

    }
    cout << "[THREAD] picRefresher thread finished." << endl;
}