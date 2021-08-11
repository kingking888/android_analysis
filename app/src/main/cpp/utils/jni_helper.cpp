//
// Created by Administrator on 2021/5/16.
//

#include "jni_helper.h"
#include "log.h"
#include "str_helper.h"

jstring char2jstring(JNIEnv *env, const char *pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte *) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

jstring string2jstring(JNIEnv *env, const string &str) {
    return char2jstring(env, str.c_str());
}

extern string format_java_in_java_parse(JNIEnv *env, uint64_t obj, string args_type);

string jstring2str(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
//    logi("---------jstr %p ", jstr);
//    logi("---------jstr len %d", env->GetStringLength(jstr));
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    auto barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
//    logi("---------barr %p jstr %p  clsstring %p strencode %p", barr, jstr, clsstring, strencode);
//    jthrowable t = clean_exception(env);
//    if (t != nullptr) {
//        logi("%s", "---------clean_exception occ!!!!");
//        string error_code = format_java_in_java_parse(env, (uint64_t) t, "jthrowable");
//        logi("---------clean_exception %s", error_code.c_str());
//    }
    jsize alen = env->GetArrayLength(barr);
//    logi("---------alen %d", alen);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen == 0) {
        env->ReleaseByteArrayElements(barr, ba, 0);
        return string();
    }
    rtn = (char *) malloc(alen + 1);
    memcpy(rtn, ba, alen);
    rtn[alen] = 0;
    env->ReleaseByteArrayElements(barr, ba, 0);
    std::string stemp(rtn);
    free(rtn);
    return stemp;
}

vector<jobject> get_java_list(JNIEnv *env, jobject obj_list) {
    jclass list_cls = env->GetObjectClass(obj_list);
    jmethodID list_get = env->GetMethodID(list_cls, "get", "(I)Ljava/lang/Object;");
    jmethodID list_size = env->GetMethodID(list_cls, "size", "()I");
    int len = env->CallIntMethod(obj_list, list_size);
    vector<jobject> ret;
    for (int i = 0; i < len; i++) {
        auto element = (jstring) (env->CallObjectMethod(obj_list, list_get, i));
        if (element == nullptr) {
            continue;
        }
        ret.push_back(element);
        env->DeleteLocalRef(element);
    }
    return ret;
}


using func_type1 = string(*)(jmethodID);
func_type1 _ZN3art9ArtMethod12JniShortNameEv = nullptr;
func_type1 _ZN3art9ArtMethod11JniLongNameEv = nullptr;
using func_type2 = string(*)(jmethodID, bool);
func_type2 _ZN3art9ArtMethod12PrettyMethodEb = nullptr;

struct auto_init_jni_helper {
    auto_init_jni_helper() {
        auto handle = fake_dlopen("libart.so", 0);
        if (handle == nullptr) {
            loge("fake_dlopen libart.so is null!", "");
            return;
        }
        _ZN3art9ArtMethod12JniShortNameEv = (func_type1) fake_dlsym(handle,
                                                                    "_ZN3art9ArtMethod12JniShortNameEv");
        if (_ZN3art9ArtMethod12JniShortNameEv == nullptr) {
            loge("fake_dlsym _ZN3art9ArtMethod12JniShortNameEv is null!", "");
        }
        _ZN3art9ArtMethod11JniLongNameEv = (func_type1) fake_dlsym(handle,
                                                                   "_ZN3art9ArtMethod11JniLongNameEv");
        if (_ZN3art9ArtMethod11JniLongNameEv == nullptr) {
            loge("fake_dlsym _ZN3art9ArtMethod11JniLongNameEv is null!", "");
        }
        _ZN3art9ArtMethod12PrettyMethodEb = (func_type2) fake_dlsym(handle,
                                                                    "_ZN3art9ArtMethod12PrettyMethodEb");
        if (_ZN3art9ArtMethod12PrettyMethodEb == nullptr) {
            loge("fake_dlsym _ZN3art9ArtMethod12PrettyMethodEb is null!", "");
        }
        fake_dlclose(handle);
    }
} init_jni_help;

string get_method_name(jmethodID method_id, method_name_type type, bool with_sign) {
    string name;
    switch (type) {
        case method_name_type::short_name:
            name = _ZN3art9ArtMethod12JniShortNameEv(method_id);
            break;
        case method_name_type::long_name:
            name = _ZN3art9ArtMethod11JniLongNameEv(method_id);
            break;
        case method_name_type::pretty_name:
            name = _ZN3art9ArtMethod12PrettyMethodEb(method_id, with_sign);
            break;
    }
    return name;
}

string get_object_class_name(JNIEnv *env, jobject obj) {
//"class [[I"
    jclass jc = env->GetObjectClass(obj);
    jmethodID toString = env->GetMethodID(jc, "toString",
                                          "()Ljava/lang/String;");
//    if (env->ExceptionCheck()) {
//        env->ExceptionDescribe();
//        env->ExceptionClear();
//    }
    string ret = jstring2str(env, (jstring) env->CallObjectMethod(jc, toString));
    return ret.substr(ret.find(" ") + 1);
}

jthrowable clean_exception(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        jthrowable t;
        t = env->ExceptionOccurred();
        env->ExceptionDescribe();
        env->ExceptionClear();
        return t;
    }
    return nullptr;
}