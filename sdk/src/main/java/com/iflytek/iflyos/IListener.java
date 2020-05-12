package com.iflytek.iflyos;

public interface IListener {
    /**
     * 唤醒回调
     * @param angle
     * @param beam
     */
    void onWakeup(int angle, int beam, String params);

    /**
     * 处理后的音频
     * @param audioBuffer
     * @param length
     */
    void onAudioCallback(byte[] audioBuffer, int length);

    void onIvwAudioCallback(byte[] audioBuffer, int length);

    void onStartRecord(int resultCode);
    void onStopRecord(int resultCode);

    void AudioException();

    void AudioTinyException();
}
