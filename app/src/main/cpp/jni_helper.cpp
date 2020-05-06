#include <jni.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <cctype>


#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <unistd.h>
#include <codecvt>
#include <locale>


using namespace std;

#ifndef TAG
#define TAG "JNI_Helper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
#endif

#ifdef __cplusplus
extern "C" {
#endif

static const char *className = "com/showame/allies/jni/JNI_Helper";

static jclass mClass = nullptr;

static void native_init(JNIEnv *env, jobject instance) {

    jclass clazz = env->GetObjectClass(instance);
    if (mClass == nullptr)
        mClass = (jclass) env->NewGlobalRef(clazz);// 创建全局引用，可以跨线程

    if (mClass != nullptr)
        LOGI("native_init!!!");
    return;

}

static void native_destory(JNIEnv *env, jobject instance) {

    jclass clazz = env->GetObjectClass(instance);
    bool b = env->IsSameObject(mClass, clazz);
    if (b) {
        env->DeleteGlobalRef(mClass);
//        env->DeleteWeakGlobalRef(mClass);
        mClass = nullptr;
    }
    LOGI("native_destory!!!");
    return;

}


// 静态函数，传递到JNI的不再是jobject，而是jclass
static jint native_static_call(JNIEnv *env, jclass clazz, jint ji) {
    int sum = 1;
    for (int i = 1; i <= ji; ++i) {
        sum *= i;
    }

    LOGI("native_static_call  %d, ret: %d", ji, sum);
    return sum;
}


static jstring native_normal_call(JNIEnv *env, jobject object, jstring s) {

    jclass clazz = env->GetObjectClass(object);// 获取对象类
    jfieldID tagId = env->GetFieldID(clazz, "Tag", "Ljava/lang/String;");// 获取FiledID
    jstring tagS = (jstring) env->GetObjectField(object, tagId);// 根据FiledID，获取对象成员属性

    const char *tag = env->GetStringUTFChars(tagS, JNI_FALSE);
    const char *oriChar = env->GetStringUTFChars(s, JNI_FALSE);

    char *temp = new char[100];
    strcpy(temp, oriChar);
    strcat(temp, ", ");
    strcat(temp, tag);

    jstring ret = env->NewStringUTF(temp);
    env->SetObjectField(object, tagId, ret);
    env->ReleaseStringUTFChars(tagS, tag);
    env->ReleaseStringUTFChars(s, oriChar);
    delete[]temp;

    return ret;

}

static jboolean native_modify_static_filed(JNIEnv *env, jobject object, jboolean b) {

    // 获取的正好是初始化时候绑定的同一个类
    jclass clazz = env->GetObjectClass(object);
    jfieldID tagId = env->GetStaticFieldID(clazz, "name", "Ljava/lang/String;");// 获取FiledID
    jstring nameS = (jstring) env->GetStaticObjectField(mClass, tagId);// 根据FiledID，获取类静态成员的属性

    const char *name = env->GetStringUTFChars(nameS, JNI_FALSE);
    int len = env->GetStringLength(nameS);

    // 转大写
    char *newChar = new char[len];
    for (int i = 0; i < len; ++i) {
        if (b)
            newChar[i] = toupper(name[i]);
        else
            newChar[i] = tolower(name[i]);
    }

    jstring newName = env->NewStringUTF(newChar);

    env->SetStaticObjectField(mClass, tagId, newName);
    env->ReleaseStringUTFChars(nameS, name);

    return true;

}

static jobject native_modify_class_filed(JNIEnv *env, jobject obj1,
                                         jobject obj2, jstring filedName,
                                         jstring filedSig, jboolean b) {

    // obg1 对应 java 层 JNI_Helper
    // obg2 对应传入的 java 对象


//    if(env->EnsureLocalCapacity(3) == JNI_FALSE){// 申请3个局部引用空间
//        LOGE("native_modify_static_filed EnsureLocalCapacity(3) Error!");
//        return false;
//    }

    // 申请16局部引入栈
    if (env->PushLocalFrame(16) < 0) {
        LOGE("native_modify_class_filed PushLocalFrame Error!");
        return nullptr;// 申请失败，直接退出即可，不需要pop
    }
    jclass clazz = env->GetObjectClass(obj2);
    const char *filedN = env->GetStringUTFChars(filedName, JNI_FALSE);
    const char *filedS = env->GetStringUTFChars(filedSig, JNI_FALSE);
    jfieldID fieldId = env->GetFieldID(clazz, filedN, filedS);
    env->DeleteLocalRef(clazz);

    jstring filed = (jstring) env->GetObjectField(obj2, fieldId);
    const char *old = env->GetStringUTFChars(filed, JNI_FALSE);

    LOGI("native_modify_class_filed: %s", old);

    int len = env->GetStringUTFLength(filed);
    char *f = new char[len];
    for (int i = 0; i < len; ++i) {
        if (b)
            f[i] = toupper(old[i]);
        else
            f[i] = tolower(old[i]);
    }

    jstring newF = env->NewStringUTF(f);

    env->SetObjectField(obj2, fieldId, newF);
    delete[]f;
//    env->ReleaseStringUTFChars(filedName, filedN);
//    env->ReleaseStringUTFChars(filedSig, filedS);
//    env->ReleaseStringUTFChars(filed, old);
//    env->ReleaseStringUTFChars(newF, f);
    env->PopLocalFrame(nullptr);// 局部引用出栈

    return obj2;
}

static void native_call_static_fun(JNIEnv *env, jobject obg, jstring method, jstring tag) {

    jclass clazz = env->GetObjectClass(obg);
    const char *methodC = env->GetStringUTFChars(method, JNI_FALSE);
    jmethodID jmethodId = env->GetStaticMethodID(clazz, methodC, "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(clazz, jmethodId, tag);

    env->DeleteLocalRef(clazz);
    return;

}


static jboolean native_call_fun(JNIEnv *env,
                                jobject obj1,
                                jobject obj2,
                                jstring filedName,
                                jstring filedSig,
                                jstring set) {

    jclass clazz = env->GetObjectClass(obj2);
    const char *filedN = env->GetStringUTFChars(filedName, JNI_FALSE);
    const char *filedS = env->GetStringUTFChars(filedSig, JNI_FALSE);

    jmethodID jmethodId = env->GetMethodID(clazz, filedN, filedS); // Get***方法中传递 char[]

    // 删除局部引用
    env->DeleteLocalRef(clazz);

//    const char *setC = env->GetStringUTFChars(set, JNI_FALSE); // Call***方法中直接传入jstring，不要传递 char[]

    return env->CallBooleanMethod(obj2, jmethodId, set);// 对象+方法名+参数，调用java层对应函数
}

static jobject native_call_new_object(JNIEnv *env, jobject obg1,
                                      jstring className,
                                      jstring filedSig,
                                      jstring filedArg) {

    const char *charName = env->GetStringUTFChars(className, JNI_FALSE);
    jclass clazz = env->FindClass(charName);

    if (clazz == nullptr) {
        env->ThrowNew(clazz, "New Object Exception");
        return nullptr;
    }
    LOGI("native_call_new_object %s", charName);
    const char *charS = env->GetStringUTFChars(filedSig, JNI_FALSE);
    jmethodID jmethodId = env->GetMethodID(clazz, "<init>", charS);

    // 在返回之前，删除方法体内的局部引用

    // JNI中 NewObject 对象是局部引用，返回 Java 空间即被销毁掉
//    const char *argC = env->GetStringUTFChars(filedArg,JNI_FALSE);
    return env->NewObject(clazz, jmethodId, filedArg);// 在jni中直接使用jstring来 new object

}


static jintArray native_call_char_array(JNIEnv *env,
                                        jobject obj,
                                        jcharArray charA) {

    int len = env->GetArrayLength(charA);

    // 直接操作 jcharArray
    jchar *temp = new jchar[3];

    //局部获取内容前3个
    env->GetCharArrayRegion(charA, 0, 3, temp);

    for (int i = 0; i < 3; ++i) {
        LOGI("GetCharArrayRegion data: %c", temp[i]); // 打印Java层数组
        temp[i] = (char) (temp[i] + 1); // 字符+1
    }
    // 通过jcharArray调用SetCharArrayRegion局部更新数据
    env->SetCharArrayRegion(charA, 0, 3, temp);

    // 直接操作 jcharArray 返回的数组指针
    jchar *charArray = env->GetCharArrayElements(charA,
                                                 JNI_FALSE);// 获取Java层传递的实际数组，JNI_FALSE表示不需要拷贝,必须调用ReleaseCharArrayElements生效改动

    for (int i = 0; i < 3; ++i) {
        LOGI("GetCharArrayElements data: %c", charArray[i]); // 打印Java层数组
        charArray[i] = (char) (charArray[i] - 1); // 字符 -1，恢复原来
    }
    env->ReleaseCharArrayElements(charA, charArray, JNI_FALSE); // 将修改charArray到charA，释放charArray缓存
//    env->ReleaseCharArrayElements(charA, charArray,JNI_COMMIT); // 将修改charArray到charA，不释放charArray缓存
//    env->ReleaseCharArrayElements(charA, charArray, JNI_ABORT); // 释放备份空间，不做修改

    delete[]temp;


    // 新建 JNI 层 jcharArray
    jintArray iNew = env->NewIntArray(12);
    int lenN = env->GetArrayLength(iNew);


    // 使用 SetIntArrayRegion
    jint *buf = new jint[lenN];

    for (int i = 0; i < lenN; ++i) {
        buf[i] = i * i;
    }
    // 局部更新将buf中数据赋值到iNew地址的0-8区间
    env->SetIntArrayRegion(iNew, 0, lenN, buf);

//        LOGI("jintArray %s", iNew); // jintArray类型是无法打印的


    // 使用直接地址，获取其内部数据,返回指针字符地址
    jint *cC = env->GetIntArrayElements(iNew, JNI_FALSE);

    for (int i = 0; i < lenN; ++i) {
        LOGI("cC %d", cC[i]); // 只能通过指针打印数据的
    }

    for (int i = 0; i < lenN; ++i) {
        cC[i] = lenN - i;
        LOGI("GetIntArrayElements %d， %c", i, cC[i]); // 在ReleaseIntArrayElements之前，数据是不会改动的
    }

    env->ReleaseIntArrayElements(iNew, cC, JNI_FALSE); // 提交修改，释放备份

    return iNew;

}


static jobjectArray native_call_object_array(JNIEnv *env,
                                             jobject object,
                                             jobjectArray array) {


    int len = env->GetArrayLength(array);

    jclass clazz = env->FindClass("com/showame/allies/jni/ResultValue");
    jmethodID jmethodId = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");

    jobjectArray jArray = env->NewObjectArray(len, clazz, nullptr);


    for (int i = 0; i < len; ++i) {
        jstring jItem = (jstring) env->GetObjectArrayElement(array, i);
        const char *itemS = env->GetStringUTFChars(jItem, JNI_FALSE);

        int size = env->GetStringUTFLength(jItem);
        char *newC = new char[size];
        for (int k = 0; k < size; k++) {
            newC[k] = char(itemS[k] + 1);
        }

        jstring newS = env->NewStringUTF(newC);
        jobject objItem = env->NewObject(clazz, jmethodId, newS);
        env->SetObjectArrayElement(jArray, i, objItem);

        // 清理内存
//        env->ReleaseStringUTFChars(newS, newC);
        env->ReleaseStringUTFChars(jItem, itemS);
        env->DeleteLocalRef(objItem);
        delete[]newC;

    }

    env->DeleteLocalRef(clazz);
    return jArray;

}

static int checkEnvException(JNIEnv *env) {
    if (env->ExceptionCheck()) {
//        env->ExceptionOccurred();
        env->ExceptionDescribe();// write to logcat
        env->ExceptionClear();
        return JNI_ERR;
    } else
        return JNI_OK;
}

void jni_throw_by_name(JNIEnv *env, const char *name, const char *msg) {
    // 查找异常类
    jclass cls = env->FindClass(name);
    /* 如果这个异常类没有找到，VM会抛出一个NowClassDefFoundError异常 */
    if (cls != NULL) {
        env->ThrowNew(cls, msg);  // 抛出指定名字的异常
//        env->Throw(jthrowable);
//        env->FatalError("Fatal Error From JNI");
    }
    /* 释放局部引用 */
    env->DeleteLocalRef(cls);
}

static jboolean native_call_jni_exception(JNIEnv *env,
                                          jobject object) {

    jclass clazzO = env->GetObjectClass(object);
    jmethodID jmethodId = env->GetMethodID(clazzO, "callJavaException", "()V");

    // 异常检查
    if (checkEnvException(env) == JNI_ERR) {
        env->DeleteLocalRef(clazzO);
        // 向 Java 抛出异常
        jni_throw_by_name(env, "java/lang/Exception",
                          "Exception from JNI: GetMethodID");
        return false;
    }

    env->CallVoidMethod(object, jmethodId);

    if (checkEnvException(env) == JNI_ERR) {
        env->DeleteLocalRef(clazzO);
        jni_throw_by_name(env, "java/lang/IllegalArgumentException", "Exception from JNI");
        return false;
    }


    LOGI("native_call_jni_exception success!");

    env->DeleteLocalRef(clazzO);
    return true;
}


// 使用信号机制拦截 JNI 异常

//#include <setjmp.h>
//
//int sigsetjmp(sigjmp_buf env, int savemask);
//返回值：若直接调用则返回0，若从siglongjmp调用返回则返回非0值
//
//void siglongjmp(sigjmp_buf env, int val);

sigjmp_buf JUMP_ANCHOR;

volatile sig_atomic_t err_cnt = 0;

void handler_sig_exception(int sigNum) {

    err_cnt += 1;

    LOGE("handler_sig_exception %d", sigNum);
    // DO SOME CLEAN STAFF HERE...

    // jump to main function to do exception process
    siglongjmp(JUMP_ANCHOR, 1);// 跳转到锚点 JUMP_ANCHOR 处理
}


int process(JNIEnv *env, jobject obj, jint i1, jint i2) {

    char *a = NULL;
    int val1 = a[1] - '0';

    char *b = NULL;
    int val2 = b[1] - '0';

    LOGE("val1 %d, val2 %d", val1, val2);

    return i1 / i2;

}


static jint native_call_jni_sig(JNIEnv *env,
                                jobject object,
                                jint i1,
                                jint i2) {

    // 设置代码跳转锚点 JUMP_ANCHOR
    // sigsetjmp直接调用，返回0，如果是siglongjmp跳转过来则是非0
    if (sigsetjmp(JUMP_ANCHOR, 1) != 0) {
        return -1;
    }

    struct sigaction sa;
    sigset_t newSig, oldSig;
    sigprocmask(SIG_BLOCK, &newSig, &oldSig); // 先阻塞信号屏蔽字，设置完成再解除
    sigset_t block_mask; // 信号屏蔽字
    sigemptyset(&block_mask); // 清空
    sigaddset(&block_mask, SIGABRT); //添加信号
    sigaddset(&block_mask, SIGSEGV);
    sigaddset(&block_mask, SIGFPE);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_mask = block_mask;
    sa.sa_handler = handler_sig_exception;
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);

    sigprocmask(SIG_SETMASK, &oldSig, nullptr);

    int ret = process(env, object, i1, i2);

    return ret;

}

int sync1 = 0;
int sync2 = 0;


static jint native_call_jni_sync(JNIEnv *env,
                                 jobject obj,
                                 jint i,
                                 jboolean sync) {

    // jni 同步
    if (sync) {
        env->MonitorEnter(obj);// 必须成对出现

        // sleep
        sleep(2);
        sync1++;

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            env->MonitorExit(obj);
            return -1;
        }
        env->MonitorExit(obj);
        return sync1;

    } else {
        // 不同步
        sleep(2);
        sync2++;

        return sync2;
    }

}

static jobject native_call_jni_nio(JNIEnv *env,
                                   jobject obj,
                                   jobject b) {

    // 获取 Java 传来的 DirectBuffer
    int len = env->GetDirectBufferCapacity(b);
    LOGI("len: %d", len);
    char *charB = (char *) env->GetDirectBufferAddress(b);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return b;
    }

    LOGI("jni rec: %s", charB);
    memset(charB,0,len);
    strcpy(charB,"new chars from jni \n");

    // 从 JNI 层返回 DirectByteBuffer 到 Java 层
    char *retB = (char *) malloc(64);
    memset(retB, '0', 64);
    strcpy(retB, "Hei hei hei, Study hardand make progress everyday!!! \\ this part will be throw");
    jobject retO = env->NewDirectByteBuffer(retB, 64);
    return retO;
}

static jobjectArray native_call_string(JNIEnv *env,
                                       jobject obj,
                                       jobjectArray a
) {

    jclass clazz = env->FindClass("java/lang/String");
    jmethodID jmethodId = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
    jstring s1 = (jstring) env->GetObjectArrayElement(a, 0);
    jstring s2 = (jstring) env->GetObjectArrayElement(a, 1);

    // jstring 是对应Java空间的数据，使用Unicode编码，JNI层操作需要转码，使用的API需要带有UTF标示
    // 获取jstring长度
    int len = env->GetStringUTFLength(s1);
    LOGI("GetStringUTFLength %d", len);
    // 获取jstring转码utf后的char*
    const char *suc = env->GetStringUTFChars(s1, JNI_FALSE);
    LOGI("GetStringUTFChars %s", suc);

    char *cSR = new char[len / 2];
    // 获取指定区间的字符串
    env->GetStringUTFRegion(s1, len / 4, len / 4 * 3, cSR);
    LOGI("GetStringUTFRegion %s", cSR);

    // 处理字符串数据量很大时候使用，使用完必须释放,不涉及转码（部分utf和unicode）
    const jchar *cSC = env->GetStringCritical(s1, JNI_FALSE);
    for (int i = 0; i < len; ++i) {
        LOGI("GetStringCritical cSC %c", cSC[i]);
    }
    env->ReleaseStringCritical(s1, cSC);


    // 新建Java空间的jstring
    jstring retUS = env->NewStringUTF("NewStringUTF FROM JNI SPACE");

    // 获取jstring的unicode编码的jchar
    const jchar *sc = env->GetStringChars(s2, JNI_FALSE);
    len = env->GetStringLength(s2);

    // GetStringCritical/GetStringChars无论是通过const char*还是const jchar*，通过%s连续打印，都只能打印首地址
    for (int i = 0; i < len; ++i) {
        LOGI("GetStringChars sc1 %c", sc[i]);
    }

    jchar *buf = new jchar[len];
    env->GetStringRegion(s2, 0, len, buf);

    const char *retC = "NewString FROM JNI SPACE";
    jstring retS = env->NewString((const jchar *) retC, strlen(retC));

    jobject jstr1 = env->NewObject(clazz, jmethodId, retUS);
    jobject jstr2 = env->NewObject(clazz, jmethodId, retS);

    jobjectArray array = env->NewObjectArray(2, clazz, nullptr);
    env->SetObjectArrayElement(array, 0, jstr1);
    env->SetObjectArrayElement(array, 1, jstr2);

    env->DeleteLocalRef(clazz);
    env->ReleaseStringChars(retS, buf);
    env->ReleaseStringUTFChars(retUS, cSR);

    return array;
}

// 线程体
void *jni_thread(void *args) {
    int pid = getpid();
    int sid = pthread_self();

//    int a = *(int *) args;
    long a = (long) args; // 平台不同，32位和64位下void*占用字节不同，强转会导致越界，使用long处理

    LOGI("jni_thread index: %d, pid: %u,  pthread_self: %u", a, pid, sid);
    // Choreographer: Skipped 59 frames!
    // The application may be doing too much work on its main thread.
    if (a == 0) {
        sleep(1);// jni下线程睡眠提示以上log
        pthread_exit((void *) "exit form jni_thread! 0");
    } else {
        while (1) {
            LOGI("jni_thread %d", a);
            sleep(1);
        }
    }
}

// 主线程退出返回Java空间，会清理局部内存，如果还存在子线程，会触发非法地址访问
static void native_call_jni_thread(JNIEnv *env,
                                   jobject obj,
                                   jint jI) {

    void *retVal;

    for (int i = 0; i < jI; ++i) {
        pthread_t pthread;
        int ret = pthread_create(&pthread, nullptr, jni_thread, (void *) i);
        if (ret != 0) {
            LOGI("pthread_create error! ");
        }
        if (i == 0)
            pthread_join(pthread, &retVal);
        else
            pthread_detach(pthread);
    }

    LOGI("mian thread pid: %u, pthread_self: %u", getpid(), pthread_self());

    LOGI("pthread_join %s", retVal);

    return;
}


static JNINativeMethod gMethods[] = {
        {
                "nativeInit",
                "()V",
                (void *) native_init
        },
        {
                "nativeDestory",
                "()V",
                (void *) native_destory
        },
        {
                "nativeStaticCall",
                "(I)I",
                (void *) native_static_call
        },
        {
                "nativeCall",
                "(Ljava/lang/String;)Ljava/lang/String;",
                (void *) native_normal_call
        },
        {
                "nativeModifyStaticFiled",
                "(Z)Z",
                (void *) native_modify_static_filed
        },
        {
                "nativeModifyFiled",
                "(Lcom/showame/allies/jni/ResultValue;Ljava/lang/String;Ljava/lang/String;Z)Lcom/showame/allies/jni/ResultValue;",
                (void *) native_modify_class_filed
        },
        {
                "nativceCallStaticFun",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_call_static_fun
        },
        {

                "nativeCallFun",
                "(Lcom/showame/allies/jni/ResultValue;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
                (void *) native_call_fun
        },
        {
                "nativeCallNewObject",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Lcom/showame/allies/jni/ResultValue;",
                (void *) native_call_new_object
        },
        {
                "nativeCallObjectArray",
                "([Ljava/lang/String;)[Lcom/showame/allies/jni/ResultValue;",
                (void *) native_call_object_array
        },
        {
                "nativeCallCharArray",
                "([C)[I",
                (void *) native_call_char_array
        },
        {
                "nativeCallJNIException",
                "(Ljava/lang/String;)Z",
                (void *) native_call_jni_exception
        },
        {
                "nativeCallJNISig",
                "(II)I",
                (void *) native_call_jni_sig
        },
        {
                "nativeCallJNISync",
                "(IZ)I",
                (void *) native_call_jni_sync
        },
        {
                "nativeCallJNINIO",
                "(Ljava/nio/ByteBuffer;)Ljava/nio/ByteBuffer;",
                (void *) native_call_jni_nio
        },
        {
                "nativeCallString",
                "([Ljava/lang/String;)[Ljava/lang/String;",
                (void *) native_call_string
        },
        {
                "nativeCallJNIThread",
                "(I)V",
                (void *) native_call_jni_thread
        },
};

/**
 * 调用父类，需要从新注册新的内部类
 * @param env
 * @param obj1
 * @param tag
 * @return
 */

static jstring native_call_super_class_fun(JNIEnv *env,
                                           jobject obj1) {

    if (env->PushLocalFrame(8) != JNI_OK) {
        LOGE("native_call_super_class_fun PushLocalFrame Error!");
        return nullptr;
    }

    // 获取父类的方法,此处使用全局引用 mClass 也是可以的
    jclass clazz = env->FindClass("com/showame/allies/jni/JNI_Helper");
    jmethodID jmethodId = env->GetMethodID(clazz, "getTag",
                                           "()Ljava/lang/String;");

    jobject obj = env->PopLocalFrame(clazz);
    return (jstring) env->CallNonvirtualObjectMethod(obj1, (jclass) obj, jmethodId);// 调用父类的方法

}

static JNINativeMethod gInnerMethod[] = {
        {
                "nativieCallSuperClassFun",
                "()Ljava/lang/String;",
                (void *) native_call_super_class_fun
        },
};

const char *innerClass = "com/showame/allies/jni/JNIInner";

static jint registerInnerNativeMethods(JNIEnv *env, const char *className) {

    jclass clazz = env->FindClass(className);

    if (env->RegisterNatives(clazz, gInnerMethod, sizeof(gInnerMethod) / sizeof(gInnerMethod[0])) !=
        JNI_OK) {
        LOGE("registerInnerNativeMethods error!");
        return JNI_ERR;
    }

    LOGI("registerInnerNativeMethods success!");

    return JNI_OK;
}


static jint registerNativeMethods(JNIEnv *env,
                                  const char *className) {
    jclass clazz = nullptr;
    clazz = env->FindClass(className);
    if (clazz == nullptr) {
        LOGE("registerNativeMethods FindClass null!!!");
        return JNI_ERR;
    }

    // 创建全局引用，解除注册前需要手动销毁
    mClass = (jclass) env->NewGlobalRef(clazz);
//    jweak mClazz = env->NewWeakGlobalRef(clazz);// 创建弱全局引用，需要手动释放

    if (env->RegisterNatives(clazz, gMethods,
                             sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        LOGE("RegisterNatives Error!!!");
        return JNI_ERR;
    }

    return JNI_OK;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    LOGI("JNI_OnLoad...");

    JNIEnv *env = nullptr;
    jint ret = JNI_ERR;

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        goto bail;
    }

    if (registerNativeMethods(env, className) != JNI_OK)
        goto bail;


    // 内部类的注册，和外部类不在一起，需要重新注册
    if (registerInnerNativeMethods(env, innerClass) != JNI_OK) {
        goto bail;
    }

    ret = JNI_VERSION_1_6;

    bail:
    if (env->ExceptionCheck()) {
        LOGE("JNI_OnLoad Error!!!");
        env->ExceptionClear();
    }

    return ret;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    jint ret = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (ret != JNI_OK) {
        return;
    }
    if (mClass != nullptr) {
        env->UnregisterNatives(mClass);
        env->DeleteGlobalRef(mClass);
        mClass = nullptr;
    }


    LOGI("JNI_OnUnload...");
    // 回收工作
}


#ifdef __cplusplus
}
#endif
