package com.iflytek.appcaetest;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.iflytek.iflyos.AlsaCAE;
import com.iflytek.iflyos.IListener;

/**
 * 2019年3月27日15:13:51
 *
 * @author admin
 */
public class MainActivity extends Activity implements View.OnClickListener {

    private static final String TAG = MainActivity.class.getSimpleName();
    private PcmFileUtil mPcmFileAudio; //
    private PcmFileUtil mPcmFileIVWAudio; //
    private TextView mShowText;
    private IListener listener = new IListener() {
        @Override
        public void onWakeup(final int angle, final int beam, String params) {
            Log.d(TAG, String.format("wakeup-------->angle: %d, beam: %d", angle, beam));
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mShowText.setText("onWakeup angle: " + angle + " , beam: " + beam);
                }
            });
        }

        @Override
        public void onAudioCallback(byte[] audioBuffer, int length) {
            Log.d(TAG, "onAudioCallback");
//            mPcmFileAudio.write(audioBuffer, 0, length);
        }

        @Override
        public void onIvwAudioCallback(byte[] audioBuffer, int length) {
//            Log.d(TAG, "onIvwAudioCallback");
//            mPcmFileIVWAudio.write(audioBuffer, 0, length);
        }

        @Override
        public void onStartRecord(final int resultCode) {
            Log.d(TAG, "onStartRecord----------> " + resultCode);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mShowText.setText("onStartRecord result: " + resultCode);
                }
            });
        }

        @Override
        public void onStopRecord(final int resultCode) {
            Log.d(TAG, "onStopRecord----------> " + resultCode);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mShowText.setText("onStopRecord result: " + resultCode);
                }
            });
        }

        @RequiresApi(api = Build.VERSION_CODES.KITKAT_WATCH)
        @Override
        public void AudioException() {
            Log.i(TAG,"AudioException--------->");

            //android.os.Process.killProcess(android.os.Process.myPid());
            final PowerManager pm = (PowerManager) getApplicationContext().getSystemService(Context.POWER_SERVICE);
            if (!pm.isInteractive()){
                Log.i(TAG,"AudioException--------->1");
                pm.reboot("");
            }else {
                Log.i(TAG,"AudioException--------->2");
                Handler mHandler = new Handler(Looper.getMainLooper());
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(getApplicationContext(), "error",Toast.LENGTH_LONG).show();
                    }
                });
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        pm.reboot("");
                    }
                },2000);

            }
        }

        @Override
        public void AudioTinyException() {
            Log.i(TAG,"AudioTinyException--------->");
            //PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            //pm.reboot("");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate...");

        setContentView(R.layout.activity_main);
        AlsaCAE.loadLib();
        requestPermissions();

        findViewById(R.id.id_btn_startRecord).setOnClickListener(this);
        findViewById(R.id.id_btn_stopRecord).setOnClickListener(this);
        findViewById(R.id.id_btn_init).setOnClickListener(this);
        findViewById(R.id.id_btn_destory).setOnClickListener(this);
        findViewById(R.id.id_btn_resource).setOnClickListener(this);
        findViewById(R.id.id_btn_setbeam).setOnClickListener(this);
        findViewById(R.id.id_btn_getVersion).setOnClickListener(this);
        findViewById(R.id.id_btn_log).setOnClickListener(this);
        findViewById(R.id.id_btn_auth).setOnClickListener(this);
        getApplicationInfo();
        mShowText = findViewById(R.id.id_tv_show);
        mPcmFileAudio = new PcmFileUtil("/sdcard/PcmAudio/");
        mPcmFileIVWAudio = new PcmFileUtil("/sdcard/PcmIVWAudio/");
    }



    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.id_btn_init:
                boolean isInit = AlsaCAE.init("/sdcard/ivw/res.bin", listener);
                mShowText.setText("Init OK: " + isInit);
                break;
            case R.id.id_btn_destory:
                AlsaCAE.destroy();
                break;
            case R.id.id_btn_resource:
                AlsaCAE.CAEReloadResource("/sdcard/ivw/res.bin");
                break;
            case R.id.id_btn_setbeam:
                AlsaCAE.CAESetRealBeam(0);
                break;
            case R.id.id_btn_getVersion:
                String ver = AlsaCAE.CAEGetVersion();
                mShowText.setText(ver);
                break;
            case R.id.id_btn_log:
                AlsaCAE.CAESetShowLog(0);
                break;
            case R.id.id_btn_startRecord:
                Log.i(TAG, "start CAE");
                mShowText.setText("start CAE");
                mPcmFileAudio.createPcmFile();
                mPcmFileIVWAudio.createPcmFile();
                AlsaCAE.startRecord();

                break;
            case R.id.id_btn_stopRecord:
                Log.i(TAG, "stop CAE");
                mShowText.setText("stop CAE");
                mPcmFileAudio.closeWriteFile();
                mPcmFileIVWAudio.closeWriteFile();
                AlsaCAE.stopRecord();
                break;
            case R.id.id_btn_auth:
                /*
                2fc650ce-0496-4916-9ecf-012056825988
                5bf43c15-6901-4aff-9c1e-955db854f29e
                6e96e173-fff5-4924-b6b7-e3f42b2f7629
                89349b65-7877-4bb3-8171-d65761030e99
                d7c4ad32-ec7a-4dfe-bd9b-f9d9057179aa
                 */
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        final int ret = AlsaCAE.CAEAuth("2fc650ce-0496-4916-9ecf-012056825988");
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mShowText.setText("CAEAuth result: " + ret);
                            }
                        });
                    }
                }).start();

                break;
            default:
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void requestPermissions() {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.LOCATION_HARDWARE, Manifest.permission.READ_PHONE_STATE,
                        Manifest.permission.WRITE_SETTINGS, Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.RECORD_AUDIO, Manifest.permission.READ_CONTACTS,
                        Manifest.permission.INTERNET}, 0x0010);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
