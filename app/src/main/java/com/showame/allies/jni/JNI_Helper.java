package com.showame.allies.jni;

import java.nio.ByteBuffer;

/**
 * Created by rememberme on 2020/4/27.
 * <p>
 * Email：891904833@qq.com
 * <p>
 * Description ：
 */
public class JNI_Helper {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("jni_helper");
    }

    private String Tag = JNI_Helper.class.getSimpleName();
    public static String name = "Jni_helper";


    public static void dumpJNI_Helper(String tag) {
        System.out.println(tag + " : " + name);
    }

    public String getTag() {
        return Tag;
    }

    private static JNI_Helper jniHelper = null;

    public static JNI_Helper getInstance() {
        if (jniHelper == null) {
            synchronized (JNI_Helper.class) {
                jniHelper = new JNI_Helper();
            }
        }
        return jniHelper;
    }

    JNI_Helper() {
        nativeInit();
    }

    JNI_Helper(String tag) {
        nativeInit();
        this.Tag = tag;
    }

    // 测试，Java 向 JNI 抛出异常
    private void callJavaException() throws Exception {

        Exception e = new Exception("Exception from Java: JNI_Helper");
        System.out.println("JNI_Helper throws callJavaException");
        throw e;

    }


    native void nativeInit();// JNI_Helper 初始化

    native void nativeDestory();// JNI_Helper 销毁

    static native int nativeStaticCall(int i);// 静态函数调用jni

    native String nativeCall(String s);// 公有函数调用

    native boolean nativeModifyStaticFiled(boolean upper);// 是否修改类static成员（大小写）

    native ResultValue nativeModifyFiled(ResultValue resultValue, String filedName, String filedSig, boolean b);// 修改对象成员并返回

    native void nativceCallStaticFun(String method, String tag);// 调用类的静态方法

    native boolean nativeCallFun(ResultValue resultValue, String filedName, String filedSig, String set);// 调用类非静态方法

    native ResultValue nativeCallNewObject(String s, String s1, String s2);// jni创建Java层对象返回

    native ResultValue[] nativeCallObjectArray(String[] strings);// jni接受java层数组，重新创建Java层的数组并返回

    //    native Boolean nativeCallJNIException(String className) throws Exception; // 声明的 native 函数无法看出是否能够抛出异常
    native boolean nativeCallJNIException(String className)throws Exception; // jni内部处理exception，并向java层抛出异常

    native int nativeCallJNISig(int i1, int i2); // 信号处理 JNI 异常

    native int nativeCallJNISync(int i, boolean sync); // 处理 JNI 多线程

    native void nativeCallJNIThread(int i); // JNI内多线程

    native ByteBuffer nativeCallJNINIO(ByteBuffer buffer);// 测试jni nio功能

    native int[] nativeCallCharArray(char[] c);// jni操作基本数组

    native  String[] nativeCallString(String[] s); // jni字符串操作

}

// 父类的调用
class JNIInner extends JNI_Helper {
    JNIInner(String n) {
        super(n);
    }
    native String nativieCallSuperClassFun();// 调用父类的 nativeCall
}
