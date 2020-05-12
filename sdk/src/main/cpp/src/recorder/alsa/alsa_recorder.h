//
// Created by liangzhu4 on 2019/5/31.
//
#include <sys/ioctl.h>
#ifndef ALSARECORDER_ALSARECORDER_H
#define ALSARECORDER_ALSARECORDER_H

#ifdef __cplusplus
extern "C" {
#endif

int PcmOpen(int card, int device, int channels, int sample_rate, int period_size, int period_count,
            int format);

int PcmStartRecord(int read_size, int queue_size);

int PcmBufferSize();

int PcmRead(char data[], int count, bool shouldClear);

int PcmClose();

#define ALSA_DEVICE_DIRECTORY "/dev/snd/"

#define SND_FILE_CONTROL	ALSA_DEVICE_DIRECTORY "controlC%i"
#define SNDRV_CTL_IOCTL_CARD_INFO	_IOR('U', 0x01, struct snd_ctl_card_info)

struct snd_ctl_card_info_t {
    int card;			/* card number */
    int pad;			/* reserved for future (was type) */
    unsigned char id[16];		/* ID of card (user selectable) */
    unsigned char driver[16];	/* Driver name */
    unsigned char name[32];		/* Short name of soundcard */
    unsigned char longname[80];	/* name + info text about soundcard */
    unsigned char reserved_[16];	/* reserved for future (was ID of mixer) */
    unsigned char mixername[80];	/* visual mixer identification */
    unsigned char
            components[128];	/* card components / fine identification, delimited with one space (AC97 etc..) */
};

struct snd_ctl_card_info {
    int card;			/* card number */
    int pad;			/* reserved for future (was type) */
    unsigned char id[16];		/* ID of card (user selectable) */
    unsigned char driver[16];	/* Driver name */
    unsigned char name[32];		/* Short name of soundcard */
    unsigned char longname[80];	/* name + info text about soundcard */
    unsigned char reserved_[16];	/* reserved for future (was ID of mixer) */
    unsigned char mixername[80];	/* visual mixer identification */
    unsigned char components[128];	/* card components / fine identification, delimited with one space (AC97 etc..) */
};



int get_snd_card_number(const char *card_name);

bool IsPcmOpened();

bool IsPcmRecording();

void AddPcmMicNumber(char pcm[], char pcm_mic[], int length, int channel_count);

#ifdef __cplusplus
}
#endif

#endif //ALSARECORDER_ALSARECORDER_H
