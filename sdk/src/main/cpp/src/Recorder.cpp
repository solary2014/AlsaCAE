/*
 * Recorder.cpp
 *
 *  Created on 2019/6/21 11:29
 *    Author: liangzhu4
 *    Email : liangzhu4@iflytek.com
 */
#include <sys/param.h>
#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <zconf.h>
#include "Recorder.h"
#include "cae/include/apr_inf.h"
#include "recorder/alsa/alsa_recorder.h"
#include "util/log/JniLog.h"
#include "ErrorCode.h"

//tinycap data/2.wav -D 2 -d 3 -c 2 -r 48000 -n 6
//#define CARD 2
//#define DEVICE 3
//#define CHANNELS 2
//#define SAMPLE_RATE 48000
//#define PERIOD_SIZE 1024
//#define PERIOD_COUNT 6
//#define FORMAT 0


#define CARD 0
#define CARD1 1
#define CARD2 2
#define DEVICE 3
#define CHANNELS 2
#define SAMPLE_RATE 48000
#define PERIOD_SIZE 1024
#define PERIOD_COUNT 6
#define FORMAT 0
#define CARD_BT_SCO    "all-i2s"

//tinycap data/2.wav -D 0 -d 3 -c 2 -r 48000 -n 6

using namespace std;

#define IS_SAVE_ORIGIN_AUDIO

#ifdef IS_SAVE_ORIGIN_AUDIO
FILE *pcmFileBefore = nullptr;
FILE *pcmFileAfter = nullptr;
#endif


Recorder::Recorder() : recordCallback_(nullptr), caeHandle_(nullptr), isRecording_(false),
                       isInit_(false) {

}

Recorder &Recorder::getInstance() {
    static Recorder recorder;
    return recorder;
}

void
CAEWakeupCallback(short angle, short channel, float power, short CMScore, short beam, char *param1,
                  void *param2, void *userData) {
//    LOGD("wakeup threadId : %d, angle: %d, channel: %d, power: %f, CMScore: %d, beam: %d, param1: %s, ",
//         pthread_self(), angle, channel, power, CMScore, beam, param1);
    if (Recorder::getInstance().getCAECallback() != nullptr) {
        Recorder::getInstance().getCAECallback()->CAEWakeup(angle, channel, power, CMScore, beam,
                                                            param1, param2,
                                                            userData);
    }

}

void
CAEIvwAudioCallback(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                    void *userData) {
    if (Recorder::getInstance().getCAECallback() != nullptr) {
        Recorder::getInstance().getCAECallback()->CAEIvwAudio(audioData, audioLen, param1, param2,
                                                              userData);
    }
}

void CAEAudioCallback(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                      void *userData) {
    if (Recorder::getInstance().getCAECallback() != nullptr) {
        Recorder::getInstance().getCAECallback()->CAEAudio(audioData, audioLen, param1, param2,
                                                           userData);
    }
}

int get_file_size(FILE * file_handle) {
    //获取当前读取文件的位置 进行保存
    unsigned int current_read_position = ftell(file_handle);
    int file_size;
    fseek(file_handle, 0, SEEK_END);
    //获取文件的大小
    file_size = ftell(file_handle);
    //恢复文件原来读取的位置
    fseek(file_handle, current_read_position, SEEK_SET);

    return file_size;
}


int64_t getCurrentTime() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

bool Recorder::init(const char *resPath) {
    if (caeHandle_ != nullptr) {
        return true;
    }

    char *params = const_cast<char *>("");
    int rv = CAENew(&caeHandle_, resPath, CAEWakeupCallback, CAEIvwAudioCallback, CAEAudioCallback,
                    params,
                    params);
    LOGD("init cae: %d", rv);
    isInit_ = (rv == 0);
    return isInit_;
}

void Recorder::destroy() {
    isRecording_ = false;
    isInit_ = false;
    if (recordCallback_ != nullptr) {
        delete recordCallback_;
        recordCallback_ = nullptr;
    }

    CAEDestroy(caeHandle_);
    if (caeHandle_ != nullptr) {
        caeHandle_ = nullptr;
    }
}

bool Recorder::isRecording() const {
    return isRecording_;
}

static void *RecordThread(void *param) {
    return static_cast<Recorder *>(param)->record();
}

void Recorder::startRecord() {
    if (!isInit_) {
        if (recordCallback_ != nullptr) {
            recordCallback_->onStartRecord(CAE_ERROR_UNINIT);
        }
        return;
    }
    if (isRecording_) {
        if (recordCallback_ != nullptr) {
            recordCallback_->onStartRecord(CAE_ERROR_RECORDING);
        }
        return;
    }

#ifdef IS_SAVE_ORIGIN_AUDIO
    pcmFileBefore = fopen("/sdcard/pcm_before.pcm", "wb+");
    pcmFileAfter = fopen("/sdcard/pcm_after.pcm", "wb+");
#endif
    pthread_t pid;
    pthread_create(&pid, nullptr, RecordThread, (void *) this);
    pid_ = pid;
}

void Recorder::stopRecord() {
    if (!isRecording_) {
        if (recordCallback_ != nullptr) {
            recordCallback_->onStopRecord(CAE_ERROR_NOT_RECORDING);
        }
    }
    isRecording_ = false;
}

int checkStart(const char *src, int length) {
    static const int SRC_CHANNEL = 6;
    static const int SRC_BIT = 4;
    if (length < 2 * SRC_CHANNEL * SRC_BIT) {//至少2帧数据才能判断
        LOGE("src length low for check start");
        return false;
    }

    return ((src[0] & 0x80) == 0x80) && ((src[4] & 0x80) == 0x80) &&
           ((src[8] & 0x00) == 0x00) && ((src[12] & 0x00) == 0x00) &&
           ((src[16] & 0x00) == 0x00) && ((src[20] & 0x00) == 0x00) &&
           ((src[24] & 0x80) == 0x80) && ((src[28] & 0x80) == 0x80) &&
           ((src[32] & 0x00) == 0x00) && ((src[36] & 0x00) == 0x00) &&
           ((src[40] & 0x00) == 0x00) && ((src[44] & 0x00) == 0x00);

}

int getChannelIndex(const char *src, int length) {
    static const int SRC_CHANNEL = 6;
    static const int SRC_BIT = 4;

    if (length < 4 * SRC_CHANNEL * SRC_BIT) { //至少2帧数据才能判断
        LOGE("src length low for get channel index");
        return -1;//未找到
    }

    //判断开始位置以对齐第一通道的有效数据
    int index = -1;
    for (int i = 0; i < (length - 2 * SRC_CHANNEL * SRC_BIT); ++i) {
        if (checkStart(src + i, length-i)) {
            index = i;
            break;
        }
    }
    LOGD("src index: %d", index);
    return index;
}

int statistics = 0;
void ReSamples(const char *in, char *out, int inSize, bool *isFirst) {
    static const int SRC_CHANNEL = 6;
    static const int DST_CHANNEL = 4;
    static const int SRC_BIT = 4;
    static const int DST_BIT = 4;
    static const int SRC_FRAME = 24;//SRC_BIT*SRC_CHANNEL通道个字节（SRC_BIT字节，SRC_CHANNEL通道）一帧有效数据
    static const int DST_FRAME = 16;//DST_BIT*DST_CHANNEL通道个字节（DST_BIT字节，DST_CHANNEL通道）一帧有效数据

    int frameIndex = 0;//帧
    int frameNum = inSize / SRC_FRAME;
    int srcIndex = 0;
    int dstIndex = 0;

    char *reIn = const_cast<char *>(in);

    if (*isFirst) {
        if (inSize < 2 * SRC_FRAME) {
            LOGE("inSize length low ");
            *isFirst = true;
            return;
        }

        //判断开始位置以对齐第一通道的有效数据
        int index = getChannelIndex(in, inSize);
        if (index != -1) {
            reIn = reIn + index;
            LOGE("find start index!");
            *isFirst = false;
        } else {
            //未找到起始位置，丢弃该段数据
            LOGE("cann't find start index!");
            *isFirst = true;
            return;
        }
    }

    while (frameIndex < frameNum) {
        srcIndex = SRC_FRAME * frameIndex;//原音频位32位（单通道4字节），6通道
        dstIndex = DST_FRAME * frameIndex;//输出音频位32位（单通道4字节），4通道

       /* memcpy(out + dstIndex, reIn + srcIndex, 4);//第一声道
        memcpy(out + dstIndex + 4, reIn + srcIndex + 4, 4);//第二声道
        memcpy(out + dstIndex + 8, reIn + srcIndex + 8, 4);//第三声道
        memcpy(out + dstIndex + 12, reIn + srcIndex + 12, 4);//第四声道,只取前4声道 5、6声道数据无用
        //memcpy(out + dstIndex + 8, reIn + srcIndex + 16, 4);//第四声道,只取前4声道 5、6声道数据无用
        //memcpy(out + dstIndex + 12, reIn + srcIndex + 20, 4);//第四声道,只取前4声道 5、6声道数据无用
       */

        char *startreIn = reIn + srcIndex + 4;
        if((startreIn[0] & 0x80) != 0x80){
            statistics++;
            if(statistics == 1024){
                statistics = 0;
                if (Recorder::getInstance().getCAECallback() != nullptr) {
                    Recorder::getInstance().getCAECallback()->CAEAudioException();
                }
            }

        }
        memcpy(out + dstIndex, reIn + srcIndex + 4, 4);//第二声道
        //memcpy(out + dstIndex + 4, reIn + srcIndex + 12, 4);//第四声道
        memcpy(out + dstIndex + 4, reIn + srcIndex + 8, 4);//第三声道
        //memcpy(out + dstIndex + 12, reIn + srcIndex + 12, 4);//第四声道,只取前4声道 5、6声道数据无用
        memcpy(out + dstIndex + 8, reIn + srcIndex + 16, 4);//第四声道,只取前4声道 5、6声道数据无用
        memcpy(out + dstIndex + 12, reIn + srcIndex + 20, 4);//第四声道,只取前4声道 5、6声道数据无用

        ++frameIndex;
    }
}

void *Recorder::record() {
    isRecording_ = true;
    int ret = 0;
    int s_bt_sco = -1;
    s_bt_sco = get_snd_card_number(CARD_BT_SCO);
    LOGD("s_bt_sco == %d ", s_bt_sco );
    ret = PcmOpen(s_bt_sco, DEVICE, CHANNELS, SAMPLE_RATE, PERIOD_SIZE, PERIOD_COUNT, FORMAT);
    if (ret != 0) {
        LOGE("pcm open error! trying another card ");
        isRecording_ = false;
        if (recordCallback_ != nullptr) {
            recordCallback_->onStartRecord(CAE_ERROR_OPEN_PCM);
        }
        return nullptr;
    }

    int pcmSize = PcmBufferSize();
    int pcmMicSize = pcmSize;
    //change here.
    ret = PcmStartRecord(pcmSize, pcmSize * 64);
    if (ret != 0) {
        LOGE("pcm record error!");
        isRecording_ = false;
        if (recordCallback_ != nullptr) {
            recordCallback_->onStartRecord(CAE_ERROR_PCM_RECORD);
        }

        return nullptr;
    }

    pcmSize = 3072;//128*4*6
    pcmMicSize = 2048;//128*4*4

    char *pcmBuffer = new char[pcmSize];
    char *pcmMicBuffer = new char[pcmMicSize];

    if (recordCallback_ != nullptr) {
        recordCallback_->onStartRecord(CAE_SUCCESS);
    }

    bool isFirst = true;
    while (isRecording_) {
        if(!IsPcmRecording()){
            LOGE("pcm_read_es fail");
            break;
        }
        PcmRead(pcmBuffer, pcmSize, shouldClearBuffer());

#ifdef IS_SAVE_ORIGIN_AUDIO
        fwrite(pcmBuffer, sizeof(char), pcmSize, pcmFileBefore);
#endif
        ReSamples(pcmBuffer, pcmMicBuffer, pcmSize, &isFirst);

        if (isFirst) {
            LOGE("cann't find start index! need check");
            setClearBuffer(true);
            recordCallback_ ->CAEAudioTinyException();
            continue;
        }

#ifdef IS_SAVE_ORIGIN_AUDIO
        fwrite(pcmMicBuffer, sizeof(char), pcmMicSize, pcmFileAfter);
        if(get_file_size(pcmFileBefore) > 1024 * 1024 * 1024 ){
            remove("/sdcard/pcm_before.pcm");
            remove("/sdcard/pcm_after.pcm");
            pcmFileBefore = fopen("/sdcard/pcm_before.pcm", "wb+");
            pcmFileAfter = fopen("/sdcard/pcm_after.pcm", "wb+");
        }


#endif
//
//        int64_t time = getCurrentTime();
        if (caeHandle_ != nullptr) {
            //CAEAudioWrite(caeHandle_, pcmBuffer, pcmSize);
            CAEAudioWrite(caeHandle_, pcmMicBuffer, pcmMicSize);
        }
//
//        int delay = (int) (getCurrentTime() - time);
//        if (delay > 5) {
//            LOGD("cae write time: %d", delay);
//        }

    }
#ifdef IS_SAVE_ORIGIN_AUDIO
    fclose(pcmFileAfter);
    pcmFileAfter = nullptr;
    fclose(pcmFileBefore);
    pcmFileBefore = nullptr;
#endif
    PcmClose();
    if (!IsPcmRecording()){
        if (recordCallback_ != nullptr) {
            recordCallback_->onStopRecord(CAE_ERROR_PCM_RECORD);
        }
    }
    if (!isRecording_) {
        if (recordCallback_ != nullptr) {
            recordCallback_->onStopRecord(CAE_SUCCESS);
        }
    }
    isRecording_ = false;
    delete[] pcmBuffer;
    delete[] pcmMicBuffer;
    return nullptr;
}

pthread_t Recorder::getPid() const {
    return pid_;
}

void Recorder::setCAECallback(IRecordCallback *callback) {
    recordCallback_ = callback;
}

IRecordCallback *Recorder::getCAECallback() const {
    return recordCallback_;
}

int Recorder::caeReloadResource(const char *resPath) {
    if (caeHandle_ == nullptr) {
        return CAE_FAIL;
    }
    int ret = CAEReloadResource(caeHandle_, resPath);
    return ret;
}

int Recorder::caeSetRealBeam(int beam) {
    if (caeHandle_ == nullptr) {
        return CAE_FAIL;
    }
    int ret = CAESetRealBeam(caeHandle_, beam);
    return ret;
}

char *Recorder::caeGetVersion() {
    char *ver = CAEGetVersion();
    return ver;
}

int Recorder::caeSetShowLog(int showLog) {
    int ret = CAESetShowLog(showLog);
    return ret;
}

int Recorder::caeAuth(const char *sn) {
    int ret = CAEAuth(const_cast<char *>(sn));
    return ret;
}

void Recorder::setClearBuffer(bool shouldClearBuffer){
    clearBuffer_ = shouldClearBuffer;
}

bool Recorder::shouldClearBuffer() {
    return clearBuffer_;
}
