/*
 * com_iflytek_iflyos_AlsaCAE.cpp
 *
 *  Created on: 2019/7/8 11:37
 *      Author: liangzhu4
 *      Email : liangzhu4@iflytek.com
 */

#include <math.h>
#include "com_iflytek_iflyos_AlsaCAE.h"
#include "../Recorder.h"
#include "../util/log/JniLog.h"
#include "../util/jvm/jvm.h"

JavaVM *g_JavaVM = nullptr;
jobject g_ListenerRef = nullptr;
jmethodID g_methodIdWakeup = nullptr;
jmethodID g_methodIdIvw = nullptr;
jmethodID g_methodIdAudio = nullptr;
jmethodID g_methodIdStartRecord = nullptr;
jmethodID g_methodIdStopRecord = nullptr;
jmethodID g_methodIdAudioException = nullptr;
jmethodID g_methodIdAudioTinyException = nullptr;

static  const  float MaxSquaredLevel = 32768 * 32768;
constexpr float  Minlevel = 30.f;

class RecordCallback : public IRecordCallback {
public:
    void onStartRecord(int result) override {
        bool thread = false; //记录当前是否存在jni环境
        JNIEnv *env = getEnv(&thread);
        env->CallVoidMethod(g_ListenerRef, g_methodIdStartRecord, result);
        if (thread) { //如果是新Attach线程则需要Detach
            detatchEnv();
        }

    }

    void onStopRecord(int result) override {
        bool thread = false; //记录当前是否存在jni环境
        JNIEnv *env = getEnv(&thread);
        env->CallVoidMethod(g_ListenerRef, g_methodIdStopRecord, result);
        if (thread) { //如果是新Attach线程则需要Detach
            detatchEnv();
        }

    }

    void CAEWakeup(short angle, short channel, float power, short CMScore, short beam, char *param1,
                   void *param2,
                   void *userData) override {
        JNIEnv *pNewEnv = nullptr;
        if (g_JavaVM == nullptr || g_JavaVM->AttachCurrentThread(&pNewEnv, nullptr) < 0) {
            return;
        }
        jstring jParams1 = pNewEnv->NewStringUTF(param1);

        pNewEnv->CallVoidMethod(g_ListenerRef, g_methodIdWakeup, angle, beam, jParams1);
        pNewEnv->DeleteLocalRef(jParams1);
        g_JavaVM->DetachCurrentThread();
    }

    void CAEIvwAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                     void *userData) override {
//        LOGD("CAEAudio ------------------> length:%d", audioLen);
        //直接写音频数据
        JNIEnv *pNewEnv = nullptr;
        if (g_JavaVM == nullptr || g_JavaVM->AttachCurrentThread(&pNewEnv, nullptr) < 0) {
            return;
        }
        jbyteArray byteArray = pNewEnv->NewByteArray(audioLen);
        pNewEnv->SetByteArrayRegion(byteArray, 0, audioLen, (const jbyte *) audioData);
        pNewEnv->CallVoidMethod(g_ListenerRef, g_methodIdIvw, byteArray, audioLen);
        pNewEnv->DeleteLocalRef(byteArray);
        g_JavaVM->DetachCurrentThread();
    }

    int statistics = 0;
    int statistics_exception = 0;
    void Process(short *audioData, unsigned int audioLen) {
            float  sum_square_ = 0;
            int sample_count_ = 0;
            for (int i = 0; i < audioLen ; ++i) {
                sum_square_ +=audioData[i] * audioData[i];
            }
            if (sum_square_ == 0){
                statistics_exception++;
                if (statistics_exception > 1024){
                    statistics_exception = 0;
                    CAEAudioTinyException();
                } else{
                    return;
                }

            }
            sample_count_ += audioLen;
            float  rms = sum_square_ / (sample_count_ * MaxSquaredLevel);
            rms = 10 * log10(rms);
            rms = -rms;
            if (rms > 70){
                statistics++;
                if (statistics > 1024){
                    //TODO
                    statistics = 0;
                    CAEAudioException();
                } else{
                    return;
                }
            }
    }

    void CAEAudio(const void *audioData, unsigned int audioLen, int param1, const void *param2,
                  void *userData) override {
        //LOGD("CAEAudio ------------------> length:%x", audioLen);
        JNIEnv *pNewEnv = nullptr;
        if (g_JavaVM == nullptr || g_JavaVM->AttachCurrentThread(&pNewEnv, nullptr) < 0) {
            return;
        }


        jbyteArray byteArray = pNewEnv->NewByteArray(audioLen);
        pNewEnv->SetByteArrayRegion(byteArray, 0, audioLen, (const jbyte *) audioData);
        pNewEnv->CallVoidMethod(g_ListenerRef, g_methodIdAudio, byteArray, audioLen);
        pNewEnv->DeleteLocalRef(byteArray);
        g_JavaVM->DetachCurrentThread();
        Process((short*)audioData, audioLen);
    }

    void CAEAudioException(){
        JNIEnv *pNewEnv = nullptr;
        if (g_JavaVM == nullptr || g_JavaVM->AttachCurrentThread(&pNewEnv, nullptr) < 0){
            return;
        }
        pNewEnv->CallVoidMethod(g_ListenerRef,g_methodIdAudioException);
        g_JavaVM->DetachCurrentThread();
    }

    void CAEAudioTinyException(){
        JNIEnv *pNewEnv = nullptr;
        if (g_JavaVM == nullptr || g_JavaVM->AttachCurrentThread(&pNewEnv, nullptr) < 0){
            return;
        }
        pNewEnv->CallVoidMethod(g_ListenerRef,g_methodIdAudioTinyException);
        g_JavaVM->DetachCurrentThread();
    }

};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *) {
    JNIEnv *env = nullptr;
    jint result = -1;

    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    initGlobalJvm(jvm);
    result = JNI_VERSION_1_6;
    return result;
}

extern "C" jboolean Java_com_iflytek_iflyos_AlsaCAE_init(JNIEnv *pEnv, jclass, jstring jResPath, jobject jListenr) {
    pEnv->GetJavaVM(&g_JavaVM);
    const char *resPath = pEnv->GetStringUTFChars(jResPath, nullptr);
    Recorder::getInstance().setCAECallback(new RecordCallback);
    bool init = Recorder::getInstance().init(resPath);
    pEnv->ReleaseStringUTFChars(jResPath, resPath);
    g_ListenerRef = pEnv->NewGlobalRef(jListenr);
    g_methodIdAudio = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                        "onAudioCallback", "([BI)V");
    g_methodIdIvw = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                      "onIvwAudioCallback", "([BI)V");
    g_methodIdWakeup = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                        "onWakeup", "(IILjava/lang/String;)V");
    g_methodIdStartRecord = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                              "onStartRecord", "(I)V");

    g_methodIdStopRecord = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                              "onStopRecord", "(I)V");
    g_methodIdAudioException = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                                "AudioException","()V");
    g_methodIdAudioTinyException = pEnv->GetMethodID(pEnv->GetObjectClass(g_ListenerRef),
                                                "AudioTinyException","()V");

    return (jboolean)init;
}

extern "C" void Java_com_iflytek_iflyos_AlsaCAE_destroy(JNIEnv *pEnv, jclass) {
    Recorder::getInstance().destroy();
    if (g_ListenerRef != nullptr) {
        pEnv->DeleteGlobalRef(g_ListenerRef);
        g_ListenerRef = nullptr;
    }
}

extern "C" void Java_com_iflytek_iflyos_AlsaCAE_startRecord(JNIEnv *, jclass) {
    Recorder::getInstance().startRecord();
}

extern "C" void Java_com_iflytek_iflyos_AlsaCAE_stopRecord(JNIEnv *, jclass) {
    Recorder::getInstance().stopRecord();
}

jint Java_com_iflytek_iflyos_AlsaCAE_CAEReloadResource(JNIEnv *pEnv, jclass, jstring jResPath) {
    const char *resPath = pEnv->GetStringUTFChars(jResPath, nullptr);
    int ret = Recorder::getInstance().caeReloadResource(resPath);
    pEnv->ReleaseStringUTFChars(jResPath, resPath);
    return ret;
}

jint Java_com_iflytek_iflyos_AlsaCAE_CAESetRealBeam(JNIEnv *, jclass, jint beam) {
    int ret = Recorder::getInstance().caeSetRealBeam(beam);
    return ret;
}

jstring Java_com_iflytek_iflyos_AlsaCAE_CAEGetVersion(JNIEnv *pEnv, jclass) {
    char *version = Recorder::getInstance().caeGetVersion();
    return pEnv->NewStringUTF(version);
}

jint Java_com_iflytek_iflyos_AlsaCAE_CAESetShowLog(JNIEnv *, jclass, jint log) {
    setLogDebug(log == 0);
    int ret = Recorder::getInstance().caeSetShowLog(log);
    return ret;
}

jint Java_com_iflytek_iflyos_AlsaCAE_CAEAuth(JNIEnv *pEnv, jclass, jstring jAuth) {
    const char *auth = pEnv->GetStringUTFChars(jAuth, nullptr);
    int ret = Recorder::getInstance().caeAuth(auth);
    pEnv->ReleaseStringUTFChars(jAuth, auth);
    return ret;
}

extern "C" void Java_com_iflytek_iflyos_AlsaCAE_CAEShouldClearAudioBuffer(JNIEnv *, jclass, jboolean clearBuffer) {
     Recorder::getInstance().setClearBuffer(clearBuffer);
}

