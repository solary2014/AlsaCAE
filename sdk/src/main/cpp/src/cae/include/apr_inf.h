#ifndef __CAE_INTF_H__
#define __CAE_INTF_H__

typedef void *CAE_HANDLE;

typedef void (*cae_ivw_fn)(short angle, short channel, float power, short CMScore, short beam, char *param1, void *param2, void *userData);

typedef void (*cae_audio_fn)(const void *audioData, unsigned int audioLen, int param1, const void *param2, void *userData);

typedef void (*cae_ivw_audio_fn)(const void *audioData, unsigned int audioLen, int param1, const void *param2, void *userData);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int CAENew(CAE_HANDLE *cae, const char *resPath, cae_ivw_fn ivwCb, cae_ivw_audio_fn ivwAudioCb, cae_audio_fn audioCb, const char *param, void *userData);
typedef int (*Proc_CAENew)(CAE_HANDLE *cae, const char *resPath, cae_ivw_fn ivwCb, cae_ivw_audio_fn ivwAudioCb, cae_audio_fn audioCb, const char *param, void *userData);

int CAEReloadResource(CAE_HANDLE cae, const char *resPath);
typedef int (*Proc_CAEReloadResource)(CAE_HANDLE cae, const char *resPath);

int CAEAudioWrite(CAE_HANDLE cae, const void *audioData, unsigned int audioLen);
typedef int (*Proc_CAEAudioWrite)(CAE_HANDLE cae, const void *audioData, unsigned int audioLen);

int CAESetRealBeam(CAE_HANDLE cae, int beam);
typedef int (*Proc_CAESetRealBeam)(CAE_HANDLE cae, int beam);

char *CAEGetVersion();
typedef char (*Proc_CAEGetVersion)();

int CAEDestroy(CAE_HANDLE cae);
typedef int (*Proc_CAEDestroy)(CAE_HANDLE cae);

int CAESetShowLog(int show_log);
typedef int (*Proc_CAESetShowLog)(int show_log);

int CAEAuth(char *sn);
typedef int (*Proc_CAEAuth)(char *sn);

void CAEShouldClearAudioBuffer(bool shouldClearBuffer);
typedef  void(*Proc_CAEShouldClearAudioBuffer)(bool shouldClearBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CAE_INTF_H__ */