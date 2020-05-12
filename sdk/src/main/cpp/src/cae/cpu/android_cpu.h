#ifndef ANROID_CPU_H
#define ANROID_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

/* 创建耗时线程 */
int android_cpu_create();

/* 等待销毁耗时线程 */
int android_cpu_destory();

#ifdef __cplusplus
}
#endif

#endif //ANROID_CPU_H