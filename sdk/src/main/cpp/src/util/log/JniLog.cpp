/*
 * jni_log.cpp
 *
 *  Created on: 2019/6/4 10:58
 *      Author: liangzhu4
 *      Email : liangzhu4@iflytek.com
 */

#include "JniLog.h"

bool g_show_log = JNI_TRUE;

void setLogDebug(bool show){
    g_show_log = show;
}