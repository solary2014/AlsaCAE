/*
 * main.cpp
 *
 *  Created on: 2019/7/8 17:02
 *      Author: liangzhu4
 *      Email : liangzhu4@iflytek.com
 */
#include <pthread.h>
#include <iostream>
#include "JniLog.h"
#include "Recorder.h"

class RecordCallback : public IRecordCallback {
public:
    void onStartRecord(int result) override {
        LOGD("onStartRecord ------------------> result:%d", result);
    }

    void CAEWakeup(short angle, short channel, float power, short CMScore, short beam, char *param1,
                   void *param2,
                   void *userData) override {
    }

    void CAEIvwAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                     void *userData) override {
        LOGD("CAEAudio ------------------> length:%d", audioLen);
        //直接写音频数据
    }

    void CAEAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                  void *userData) override {
        LOGD("CAEAudio ------------------> length:%d", audioLen);
    }

    void onPCM(const char *audio, int audioLen) override {
        LOGD("onPCM ------------------> length:%d", audioLen);
    }
};

int main() {
    using namespace std;
    cout << "ver: 1.0.1.190719_beta" << endl;
    Recorder::getInstance().init("/sdcard/ivw/res.bin");
    Recorder::getInstance().setCAECallback(new RecordCallback);
    Recorder::getInstance().startRecord();
//    sleep(100000);
    pthread_join(Recorder::getInstance().getPid(), nullptr);

}

