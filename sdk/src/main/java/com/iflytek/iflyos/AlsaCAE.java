package com.iflytek.iflyos;


import com.iflytek.iflyos.IListener;

public class AlsaCAE {

    public static final int CAE_FAIL = -1;// 失败
    public static final int CAE_SUCCESS = 0;//成功
    public static final int CAE_ERROR_RECORDING = 1;//正在录音
    public static final int CAE_ERROR_OPEN_PCM = 2;//PCM打开失败
    public static final int CAE_ERROR_PCM_RECORD = 3;//PCM录音失败
    public static final int CAE_ERROR_UNINIT = 4;//没有初始化
    public static final int CAE_ERROR_NOT_RECORDING = 5;//没有正在录音


    private static boolean LIB_LOADED = false;

    public static void loadLib() {
        if (!LIB_LOADED) {
            try {
                System.loadLibrary("alsacae");
                LIB_LOADED = true;
            } catch (Exception e) {
                LIB_LOADED = false;
            }
        }
    }

    public static boolean isLibLoaded() {
        return LIB_LOADED;
    }

    /**
     * 初始化服务,应用启动时调用，稍微延迟些调用
     * @param resPath 唤醒词路径
     * @param listener 回调接口
     */
    public static native boolean init(String resPath, IListener listener);

    /**
     * 销毁，应用退出时调用
     */
    public static native void destroy();

    /**
     * 开启录音
     */
    public static native void startRecord();

    /**
     * 停止录音
     */
    public static native void stopRecord();

    /**
     * 配置资源路径
     *
     * @param resPath 资源路径
     * @return 0表示成功，其余表示失败
     */
    public static native int CAEReloadResource(String resPath);

    /**
     * 设置麦克风
     *
     * @param beam 麦号
     * @return
     */
    public static native int CAESetRealBeam(int beam);

    /**
     * 获取CAE版本号
     *
     * @return
     */
    public static native String CAEGetVersion();

    /**
     * 设置日志打印
     *
     * @param showLog 0表示调试，1表示信息，2表示错误
     * @return
     */
    public static native int CAESetShowLog(int showLog);

    /**
     * 网络鉴权
     *
     * @param sn 鉴权码
     * @return
     */
    public static native int CAEAuth(String sn);

    /**
     * 通知底层唤醒事件处理完成
     *
     */
    public static native void CAEShouldClearAudioBuffer(boolean clearBuffer);
}
