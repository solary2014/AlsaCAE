/*
 * ErrorCode.h
 *
 *  Created on 2019/6/21 20:49
 *    Author: liangzhu4
 *    Email : liangzhu4@iflytek.com
 */

#ifndef CPP_ERROR_CODE_H
#define CPP_ERROR_CODE_H



/*******************ERRORCODES***********************/


#define CAE_FAIL                  -1 // 失败
#define CAE_SUCCESS                0 // 成功
#define CAE_ERROR_RECORDING        1 // 正在录音
#define CAE_ERROR_OPEN_PCM         2 // PCM打开失败
#define CAE_ERROR_PCM_RECORD       3 // PCM录音失败
#define CAE_ERROR_UNINIT           4 // 没有初始化
#define CAE_ERROR_NOT_RECORDING    5 // 没有正在录音


#endif //CPP_ERROR_CODE_H
