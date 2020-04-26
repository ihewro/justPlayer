/**
  * User: hewro
  * Date: 2020/4/26
  * Time: 17:43
  * Description: 
  */
//
// Created by hewro on 2020/4/26.
//

#ifndef JUSTPLAYER_PROCESSOR_H
#define JUSTPLAYER_PROCESSOR_H

/**
 * 视频流和音频公用的流程：
 * avPacket -> avFrame
 */
class Processor {

public:
    void avP2F();
    virtual void avFrameEncode();
};


#endif //JUSTPLAYER_PROCESSOR_H
