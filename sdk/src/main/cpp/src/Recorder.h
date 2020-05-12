/*
 * Recorder.h
 *
 *  Created on 2019/6/21 11:29
 *    Author: liangzhu4
 *    Email : liangzhu4@iflytek.com
 */

#ifndef CPP_RECORDER_H
#define CPP_RECORDER_H

#include <pthread.h>
#include "apr_inf.h"

class IRecordCallback {
public:
    IRecordCallback() {};

    virtual ~IRecordCallback() {};

public:
    virtual void onStartRecord(int result) = 0;

    virtual void
    CAEWakeup(short angle, short channel, float power, short CMScore, short beam, char *param1,
              void *param2, void *userData) = 0;

    virtual void
    CAEIvwAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                void *userData) = 0;

    virtual void
    CAEAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
             void *userData) = 0;

    virtual void onStopRecord(int result) = 0;

    virtual void CAEAudioException() = 0;

    virtual void CAEAudioTinyException() = 0;
};

class Recorder {
public:
    static Recorder &getInstance();

private:
    Recorder();

    Recorder(const Recorder &rhs);

    Recorder &operator=(const Recorder &rhs);

public:
    bool init(const char *resPath);

    void destroy();

    void startRecord();

    void stopRecord();

    int caeReloadResource(const char *resPath);

    int caeSetRealBeam(int beam);

    char *caeGetVersion();

    int caeSetShowLog(int showLog);

    int caeAuth(const char *sn);

    bool isRecording() const;

    void setCAECallback(IRecordCallback *callback);

    IRecordCallback *getCAECallback() const;

    void *record();

    void setClearBuffer(bool shouldClearBuffer);

    bool shouldClearBuffer();

    pthread_t getPid() const ;

private:
    IRecordCallback *recordCallback_;
    CAE_HANDLE caeHandle_;
    bool isRecording_;
    bool isInit_;
    bool clearBuffer_; // 0  1
    pthread_t pid_;
};


#endif //CPP_RECORDER_H
