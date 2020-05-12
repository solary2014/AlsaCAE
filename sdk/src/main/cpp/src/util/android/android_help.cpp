/*
 * android_help.cpp
 *
 *  Created on: 2019/6/14 15:06
 *      Author: liangzhu4
 *      Email : liangzhu4@iflytek.com
 */
#include <time.h>
#include "android_help.h"

jobject AndroidHelper::getGlobalContext(JNIEnv *env) {
    //获取Activity Thread的实例对象
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    //获取Application，也就是全局的Context
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                                "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(at, getApplication);
    return context;
}

jstring AndroidHelper::getPackageName(JNIEnv *env) {
    jobject context = getGlobalContext(env);
    jclass contextClass = env->FindClass("android/app/Application");
    jmethodID getApplicationId = env->GetMethodID(contextClass, "getPackageName",
                                                  "()Ljava/lang/String;");
    jstring jPackageName = (jstring) env->CallObjectMethod(context, getApplicationId);
    return jPackageName;
}

jstring AndroidHelper::getNativeLibraryDir(JNIEnv *env) {
    jobject context = getGlobalContext(env);
    jclass contextClass = env->FindClass("android/app/Application");
    jmethodID getApplicationId = env->GetMethodID(contextClass, "getApplicationInfo",
                                                  "()Landroid/content/pm/ApplicationInfo;");
    jobject applicationInfo = (jstring) env->CallObjectMethod(context, getApplicationId);
    jclass applicationClass = env->FindClass("android/content/pm/ApplicationInfo");
    jfieldID jNativeLibraryDirId = env->GetFieldID(applicationClass, "nativeLibraryDir", "Ljava/lang/String;");
    jstring jNativeLibraryDir = (jstring)env->GetObjectField(applicationInfo, jNativeLibraryDirId);
    return jNativeLibraryDir;
}

