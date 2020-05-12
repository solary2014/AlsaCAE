//
// Created by liangzhu4 on 2019/5/31.
//
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <cstdio>
#include "../../util/queue/AudioQueue.h"
#include "../include/asoundlib.h"
#include "../../util/log/JniLog.h"
#include "alsa_recorder.h"
#include "Recorder.h"

#define PCM_FORMAT_16 0
#define PCM_FORMAT_32 1
#define PCM_FORMAT_8  2
#define PCM_FORMAT_24 3


// 声卡设置
struct pcm_config g_pcm_config;
// pcm设备全局指针
struct pcm *g_pcm = nullptr;
// 音频缓冲队列
struct audio_queue_t *g_audio_queue = nullptr;
// 队列在内存首地址，用于delete内存
char *g_queue_base = nullptr;
bool g_is_recording = false;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static int get_snd_card_name(int card, char *name);
void *RecordThreadFunc(void *params) {
    LOGD("RecordThreadFunc begin");
    prctl(PR_SET_NAME, "AlsaRecorder-PcmReadThead");

    int read_size = 0;
    if (nullptr == g_pcm) {
        LOGD("pcm is null");
        return nullptr;
    }
    read_size = (int) (size_t) params;

    char *buffer = new char[read_size];
    while (g_is_recording) {
//        pthread_mutex_lock(&g_mutex);
        int ret = 0;
        if (nullptr != g_pcm) {
            //LOGE("pcm_read_es| pcm_read_es size = %d", read_size);
            ret = pcm_read_es(g_pcm, buffer, read_size);
        }

        if (0 != ret) {
            LOGE("RecordThreadProc | pcm_read ret = %d", ret);
            PcmClose();
            break;
        }
        if (nullptr != g_audio_queue && !queue_write(g_audio_queue, buffer, read_size)) {
            LOGE("RecordThreadProc | queue_write failed. front: %d, rear: %d, cap: %d ",
                 g_audio_queue->front, g_audio_queue->rear, g_audio_queue->capacity);

        }
//        pthread_mutex_unlock(&g_mutex);
    }
    delete[] buffer;
    LOGD("RecordThreadFunc end");
    return nullptr;
}

int PcmOpen(int card, int device, int channels, int sample_rate, int period_size, int period_count,
            int format) {
    if (nullptr != g_pcm) {
        LOGE("PcmOpen | pcm is open already");
        return 0;
    }
    g_pcm_config.channels = channels;
    g_pcm_config.rate = sample_rate;
    g_pcm_config.period_size = period_size;
    g_pcm_config.period_count = period_count;
    switch (format) {
        case PCM_FORMAT_8:
            g_pcm_config.format = PCM_FORMAT_S8;
            break;
        case PCM_FORMAT_16:
            g_pcm_config.format = PCM_FORMAT_S16_LE;
            break;
        case PCM_FORMAT_24:
            g_pcm_config.format = PCM_FORMAT_S24_LE;
            break;
        case PCM_FORMAT_32:
            g_pcm_config.format = PCM_FORMAT_S32_LE;
            break;
        default:
            g_pcm_config.format = PCM_FORMAT_S24_LE; //-b
            break;
    }
    g_pcm_config.start_threshold = 0;
    g_pcm_config.stop_threshold = 0;
    g_pcm_config.silence_threshold = 0;

    struct pcm *pcm_handle = pcm_open(card, device, PCM_IN, &g_pcm_config);
    if (nullptr == pcm_handle || !pcm_is_ready(pcm_handle)) {
        LOGE("pcm_open | unable to open PCM IN Device %s", pcm_get_error(pcm_handle));
        LOGD("pcm_open | pcm card %d open fail. | pcm device is %d. | pcm sampleRate is %d. ", card,
             device, sample_rate);
        LOGD("pcm channels is %d. | pcm period_size is %d. | pcm bit is %d. | pcm period count is %d.",
             g_pcm_config.channels, g_pcm_config.period_size, pcm_format_to_bits(g_pcm_config.format),
             g_pcm_config.period_count);
        return -EINVAL;
    }

    LOGD("pcm_open | pcm card %d open success. | pcm device is %d. | pcm sampleRate is %d. ", card,
         device, sample_rate);
    LOGD("pcm channels is %d. | pcm period_size is %d. | pcm bit is %d. | pcm period count is %d.",
         g_pcm_config.channels, g_pcm_config.period_size, pcm_format_to_bits(g_pcm_config.format),
         g_pcm_config.period_count);
    g_pcm = pcm_handle;
    return 0;
}

int PcmStartRecord(int read_size, int queue_size) {
    if (nullptr == g_pcm) {
        LOGE("start_record | pcm handle is null");
        return -1;
    }
    if (g_is_recording) {
        LOGE("start_record | recording thread already started!");
        return -1;
    }
    LOGD("PcmStartRecord size: %d, queue size: %d", read_size, queue_size)
    g_is_recording = true;

    g_queue_base = new char[sizeof(audio_queue_t) + queue_size + 1];
    g_audio_queue = queue_init(g_queue_base, queue_size + 1);

    pthread_attr_t thread_attr;
    struct sched_param thread_param;

    pthread_attr_init(&thread_attr);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_param.sched_priority = sched_get_priority_max(SCHED_RR);
    pthread_attr_setschedparam(&thread_attr, &thread_param);

    pthread_t tid;
    int ret = pthread_create(&tid, nullptr, RecordThreadFunc, (void *) read_size);
    return ret;
}

int PcmBufferSize() {
    int size = 0;
    if (nullptr != g_pcm) {
        size = pcm_frames_to_bytes(g_pcm, pcm_get_buffer_size(g_pcm));
    } else {
        LOGE("PcmBufferSize | pcmHandle is null");
        size = -1;
    }
    return size;
}

int PcmRead(char data[], int count, bool shouldClear) {
    if (nullptr == g_audio_queue) {
        LOGE("pcm_read | audioQueue is null!");
        return -1;
    }
    if (shouldClear) {

        g_audio_queue->front = 0;
        g_audio_queue->rear = 0;

        LOGE("cae wakeup, cleaer audio_queue %d", shouldClear);
    }
    int read_length = queue_read(g_audio_queue, data, count);

//    int read_length = 0;
//    if (nullptr != g_pcm)
//        read_length = pcm_read(g_pcm, data, count);
////    LOGD("pcm read length:%d", read_length);
    return read_length;
}

bool IsPcmOpened() {
    return nullptr != g_pcm;
}

bool IsPcmRecording(){
    return g_is_recording;
};

int PcmClose() {
    g_is_recording = false;
    pthread_mutex_lock(&g_mutex);
    int ret = 0;
    if (nullptr != g_pcm) {
        ret = pcm_close(g_pcm);
        g_pcm = nullptr;
    }

    if (nullptr != g_audio_queue) {
        queue_destroy(g_audio_queue);
        g_audio_queue = nullptr;
        delete[] g_queue_base;
        g_queue_base = nullptr;
    }
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

/*
 * 添加通道号
 */
void AddPcmMicNumber(char pcm[], char pcm_mic[], int length, int channel_count) {
    int dataIndex = 0;//数据操作的位置
    int dataNum = length / 2; //2个字节一组有效数据
    int mic_index = 0, index = 0;
    while (dataIndex < dataNum) {
        for (int i = 0; i < channel_count; ++i) {
            index = 2 * dataIndex;
            mic_index = 4 * dataIndex;
            pcm_mic[mic_index] = 0;
            pcm_mic[mic_index + 1] = (char) (i + 1);
            pcm_mic[mic_index + 2] = pcm[index];
            pcm_mic[mic_index + 3] = pcm[index + 1];
            ++dataIndex;
        }
    }
}

int get_snd_card_number(const char *card_name){
    int i = 0;
    int ret = 0;
    char cur_name[64] = {0};

    //loop search card number, which is in the ascending order.
    for (i = 0; i < 32; i++) {
        ret = get_snd_card_name(i, &cur_name[0]);
        if (ret < 0)
        { break; }
        if (strcmp(cur_name, card_name) == 0) {
            LOGD("Search Completed, cur_name is %s, card_num=%d", cur_name, i);
            return i;
        }
    }
    LOGD("There is no one matched to <%s>.", card_name);
    return -1;
}

static int get_snd_card_name(int card, char *name)
{
    int fd;
    struct snd_ctl_card_info_t info;
    char control[sizeof(SND_FILE_CONTROL) + 10] = {0};
    sprintf(control, SND_FILE_CONTROL, card);

    fd = open(control, O_RDONLY);
    if (fd < 0) {
        LOGD("open snd control failed.");
        return -1;
    }
    if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, &info) < 0) {
        LOGD("SNDRV_CTL_IOCTL_CARD_INFO failed.");
        close(fd);
        return -1;
    }
    close(fd);
    LOGD("card name is %s, query card=%d", info.name, card);
    //get card name
    if (name) { strcpy(name, (char *)info.name); }
    return 0;
}

