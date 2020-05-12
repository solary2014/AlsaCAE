/*
 * JniLog.h
 *
 *  Created on: 2015年10月9日
 *      Author: admin
 */

#ifndef JNI_SRC_LOG_JNILOG_H_
#define JNI_SRC_LOG_JNILOG_H_

#include <android/log.h>
#include <jni.h>

#define LOG_TAG "Alsa-Jni"

extern bool g_show_log;
#define LOGV(...)  if (g_show_log) { \
						__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__); \
					}
#define LOGI(...)  if (g_show_log) { \
						__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__); \
					}
#define LOGD(...)  if (g_show_log) { \
						__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__); \
					}
#define LOGE(...)  if (g_show_log) { \
						__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__); \
					}

void setLogDebug(bool show);

#endif /* JNI_SRC_LOG_JNILOG_H_ */
