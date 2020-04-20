#include <iostream>

#include <string>
#include "VideoGrabber.h"

using std::string;
using std::cout;
using std::endl;
extern "C" {
#include "SDL2/SDL.h"
};

void playSdlVideo(VideoGrabber* videoGrabber,mutex *pMutex,vector<AVFrame*> frameVec);
void picRefresher(int timeInterval, bool& exitRefresh);

int main() {
    string filePath = "/Users/hewro/Downloads/产品介绍4.mp4";
    std::cout << "Hello, World!" << std::endl;
    std::mutex			mtx{};//帧锁，避免frameVector出现线程冲突问题
    vector<AVFrame*>	frameVec{};//存储视频帧
    //启动视频抓取
    auto* videoGrabber = new VideoGrabber(filePath);
    videoGrabber->setMutex(&mtx);
    videoGrabber->setVector(&frameVec);
    videoGrabber->start();

    //播放视频
    playSdlVideo(videoGrabber, &mtx, frameVec);

    return 0;
}


void playSdlVideo(VideoGrabber* videoGrabber,mutex *pMutex,vector<AVFrame*> frameVec){
    //播放视频
    SDL_Window* screen;
    int width = 640;
    int height = 360;
    // SDL 2.0 Support for multiple windows
    screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED,
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

    //定时器刷新图片的定时器
    std::thread refreshThread{picRefresher, (int)(1000 / videoGrabber->frameRate), std::ref(videoGrabber->stopFlag)};

    while (true){
        //等待sdl 事件
        SDL_WaitEvent(&event);

        if (videoGrabber->stopFlag){
            break;
        }
        if (!frameVec.empty()){
            AVFrame* frame = frameVec.back();
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
        }

        //根据帧率休眠一定时间
//        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 / videoGrabber->frameRate)));

    }

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