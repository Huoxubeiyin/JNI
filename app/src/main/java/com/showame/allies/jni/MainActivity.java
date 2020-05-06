package com.showame.allies.jni;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class MainActivity extends AppCompatActivity {


    private JNI_Helper jniHelper = null;
    private static final String TAG = "MainActivity";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // init JNI_Helper
        jniHelper = JNI_Helper.getInstance();
        jniHelper.nativeInit();

        setContentView(R.layout.activity_main);
        initView();

    }


    private void initView() {

        findViewById(R.id.call_static).setOnClickListener(onClickListener);
        findViewById(R.id.call_normal).setOnClickListener(onClickListener);
        findViewById(R.id.modify_static).setOnClickListener(onClickListener);
        findViewById(R.id.modify_normal).setOnClickListener(onClickListener);
        findViewById(R.id.call_static_fun).setOnClickListener(onClickListener);
        findViewById(R.id.call_fun).setOnClickListener(onClickListener);
        findViewById(R.id.call_new_object).setOnClickListener(onClickListener);
        findViewById(R.id.call_super_class_fun).setOnClickListener(onClickListener);
        findViewById(R.id.call_object_array).setOnClickListener(onClickListener);
        findViewById(R.id.call_jni_exception).setOnClickListener(onClickListener);
        findViewById(R.id.call_jni_sig).setOnClickListener(onClickListener);
        findViewById(R.id.call_jni_sync).setOnClickListener(onClickListener);
        findViewById(R.id.call_jni_nio).setOnClickListener(onClickListener);
        findViewById(R.id.call_char_array).setOnClickListener(onClickListener);
        findViewById(R.id.call_string_utf).setOnClickListener(onClickListener);
        findViewById(R.id.call_jni_thread).setOnClickListener(onClickListener);

    }


    View.OnClickListener onClickListener = new View.OnClickListener() {

        ResultValue resultValue = new ResultValue("Hello World, My God!");

        @Override
        public void onClick(View view) {
            int id = view.getId();
            switch (id) {
                case R.id.call_static:
                    int i = JNI_Helper.nativeStaticCall(5);
                    printLog("nativeStaticCall: 5!=" + i);
                    break;
                case R.id.call_normal:
                    printLog("nativeCall:" + jniHelper.getTag());
                    String s = jniHelper.nativeCall("Java call");
                    printLog("nativeCall:" + jniHelper.getTag());
                    printLog(s.equals(jniHelper.getTag()) + "");
                    break;
                case R.id.modify_static:
                    boolean b = jniHelper.nativeModifyStaticFiled(true);
                    printLog("ToUpper: " + b + ", " + JNI_Helper.name);
                    break;
                case R.id.modify_normal:
                    // 传入一个对象，内部私有成员，成员签名，是否转大写，返回此对象，获取其值
                    boolean toupper = false;
                    ResultValue ret = jniHelper.nativeModifyFiled(resultValue, "mValue", "Ljava/lang/String;", toupper);
                    if (ret != null)
                        printLog("same obj: " + (resultValue == ret) + ", ToUpper: " + toupper + ", " + ret.getV());
                    break;
                case R.id.call_static_fun:
                    // 调用类的静态方法
                    jniHelper.nativceCallStaticFun("dumpJNI_Helper", "ALLIES:");
                    break;
                case R.id.call_fun:
                    // 调用类中普通方法
                    boolean b1 = jniHelper.nativeCallFun(resultValue, "setV", "(Ljava/lang/String;)Z", "Showame");
                    if (b1) printLog(resultValue.getV());
                    break;
                case R.id.call_new_object:
                    // 构造函数的签名返回值为(*)V
                    ResultValue res = jniHelper.nativeCallNewObject("com/showame/allies/jni/ResultValue", "(Ljava/lang/String;)V", "2020/4/27");
                    if (res != null)
                        printLog(res.getV());
                    break;
                case R.id.call_super_class_fun:
                    JNIInner inner = new JNIInner("JNI_Inner");
                    String s1 = inner.nativieCallSuperClassFun();
                    if (s1 != null)
                        printLog(s1);
                    break;

                case R.id.call_char_array:
                    char[] javaI = new char[]{
                            'N', 'B', 'A', 'C', 'B', 'A'
                    };
                    int[] retI = jniHelper.nativeCallCharArray(javaI);

                    int ii;
                    for (ii = 0; ii < javaI.length; ii++) {
                        printLog(javaI[ii] + "");
                    }

                    for (ii = 0; ii < retI.length; ii++) {
                        printLog(retI[ii] + "");
                    }
                    break;
                case R.id.call_object_array:
                    // 传递数组并返回数组对象
                    String str[] = {
                            "Abc",
                            "dEf",
                            "Hij",
                            "lmN",
                    };
                    ResultValue rv[] = jniHelper.nativeCallObjectArray(str);
                    if (rv == null)
                        return;
                    StringBuffer buffer = new StringBuffer();
                    for (int k = 0; k < rv.length; k++) {
                        buffer.append("ResultValue " + k + " " + rv[k].getV() + " ");
                    }
                    printLog(buffer.toString());
                    break;
                case R.id.call_jni_exception:
                    try {
                        boolean b2 = jniHelper.nativeCallJNIException("com/showame/allies/jni/ResultValue");
                    } catch (Exception e) {
                        printLog(e.getMessage());
                        e.printStackTrace();
                    }
                    break;
                case R.id.call_jni_sig:
                    int i1 = jniHelper.nativeCallJNISig(12, 0);
                    printLog(i1 + "");
                    break;
                case R.id.call_jni_sync:
                    testJniThreadSync();
                    break;
                case R.id.call_jni_nio:
                    // Java NIO 与 JNI NIO
                    testNIOJava2JNI();
                    break;
                case R.id.call_string_utf:
                    try {
                        String str1 = new String("Java String Default");
                        String str2 = new String(("Java String UTF-8").getBytes("UTF-8"), "UTF-8");
                        String[] strArray = {str1, str2};
                        String[] retS = jniHelper.nativeCallString(strArray);
                        printLog(retS[0] + "\n" + retS[1]);
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
                    break;
                case R.id.call_jni_thread:
                    jniHelper.nativeCallJNIThread(3);
                    break;
            }
        }
    };

    private void testNIOJava2JNI() {

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(128); // 可以传递到 JNI 中
        ByteBuffer byteBuffer1 = ByteBuffer.allocate(128); //不能传递到 JNI 中

        try {
            byteBuffer.put("Study hardand make progress everyday!".getBytes("UTF-8"));
            byteBuffer.flip();

            ByteBuffer retBuffer = jniHelper.nativeCallJNINIO(byteBuffer);

            StringBuffer buffer = new StringBuffer("Java ByteBuffer Size: " + retBuffer.limit() + " ");
            while (byteBuffer.hasRemaining()) {
                buffer.append((char) byteBuffer.get());
            }

            Log.i(TAG, "testNIOJava2JNI: orig " + buffer.toString());
            buffer.delete(0, buffer.length() - 1);

            while (retBuffer.hasRemaining()) {
                char c;
                if ((c = (char) retBuffer.get()) == '\\')
                    break;
                buffer.append(c);
            }

            Log.i(TAG, "testNIOJava2JNI: ret " + buffer.toString());

            retBuffer.clear();

        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }


    // Java 不做同步，多线程调用 jni
    private void testJniThreadSync() {

        int i;
        for (i = 0; i < 8; i++) {
            final int finalI = i;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    int ret = jniHelper.nativeCallJNISync(finalI, false);
                    Log.i(TAG, "run sync false ThreadId " + Thread.currentThread().getId() + ", nativeCallJNISync " + ret);
                }
            }).start();
        }

        for (i = 0; i < 8; i++) {
            final int finalI = i;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    int ret = jniHelper.nativeCallJNISync(finalI, true);
                    Log.i(TAG, "run sync true ThreadId " + Thread.currentThread().getId() + ", nativeCallJNISync " + ret);
                }
            }).start();
        }

    }


    public void printLog(String s) {
        Log.i(TAG, s);
    }

    @Override
    protected void onDestroy() {
        if (jniHelper != null)
            jniHelper.nativeDestory();

        super.onDestroy();
    }
}



