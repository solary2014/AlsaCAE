/*
 * android_help.h
 *
 *  Created on: 2019/6/14 15:06
 *      Author: liangzhu4
 *      Email : liangzhu4@iflytek.com
 */

#include <jni.h>

#ifndef ALSARECORDER_ANDROIDHELPER_H
#define ALSARECORDER_ANDROIDHELPER_H


class AndroidHelper {
public:
    static jobject getGlobalContext(JNIEnv *env);

    static jstring getPackageName(JNIEnv *env);

    static jstring getNativeLibraryDir(JNIEnv *env);

};


#endif //ALSARECORDER_ANDROIDHELPER_H
