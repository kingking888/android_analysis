#include <map>

#include "../dlfc/dlfcn_nougat.h"
#include "../utils/str_helper.h"
#include "../utils/jni_helper.h"
#include "../utils/log.h"
#include "parse_java_sig.h"
#include "enum_jni_static_method.h"

using std::string;
using std::map;
using std::count;

void
get_method_info(JNIEnv *env, const string &class_name, const vector<native_method_info *> &infos) {
    jclass jClass = nullptr, jMethod = nullptr;
    jmethodID jforName = nullptr, jgetDeclaredMethods = nullptr, jgetName = nullptr, jtoString = nullptr;
    jobject jtar_Class_obj = nullptr, jmethods_list_obj = nullptr, jmethod_name = nullptr, item = nullptr;
    jstring jstring_method_decl = nullptr, jstring_tar_class_name = nullptr;

    auto release = [&]() {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        if (jstring_tar_class_name != nullptr) {
            env->DeleteLocalRef(jstring_tar_class_name);
        }
        if (jtar_Class_obj != nullptr) {
            env->DeleteLocalRef(jtar_Class_obj);
        }
        if (jmethods_list_obj != nullptr) {
            env->DeleteLocalRef(jmethods_list_obj);
        }
        if (jmethod_name != nullptr) {
            env->DeleteLocalRef(jmethod_name);
        }

    };

    jClass = env->FindClass("java/lang/Class");
    jforName = env->GetStaticMethodID(jClass, "forName",
                                      "(Ljava/lang/String;)Ljava/lang/Class;");
    jgetDeclaredMethods = env->GetMethodID(jClass, "getDeclaredMethods",
                                           "()[Ljava/lang/reflect/Method;");
    jMethod = env->FindClass("java/lang/reflect/Method");
    jgetName = env->GetMethodID(jMethod, "getName", "()Ljava/lang/String;");
    jtoString = env->GetMethodID(jMethod, "toString", "()Ljava/lang/String;");

    string jni_class_name = class_name;
//    replace_all(jni_class_name, ".", "/");

    jstring_tar_class_name = string2jstring(env, jni_class_name);
    jtar_Class_obj = env->CallStaticObjectMethod(jClass, jforName, jstring_tar_class_name);
    if (jtar_Class_obj == nullptr) {
        release();
        return;
    }
    jmethods_list_obj = env->CallObjectMethod(jtar_Class_obj, jgetDeclaredMethods);

    int len = env->GetArrayLength((jarray) jmethods_list_obj);
    for (int index = 0; index < len; index++) {
        item = env->GetObjectArrayElement((jobjectArray) jmethods_list_obj, index);
        jmethod_name = env->CallObjectMethod((jobject) item, jgetName);
        string method_name = jstring2str(env, (jstring) jmethod_name);
        for (const auto &info: infos) {
            if (method_name == info->method_name) {
                jstring_method_decl = (jstring) env->CallObjectMethod((jobject) item, jtoString);
                string method_decl = jstring2str(env, jstring_method_decl);
                env->DeleteLocalRef(jstring_method_decl);
                if (parse_java_pretty_name(method_decl, info->class_name, info->method_name,
                                           info->args_type, info->ret_type)) {
                    info->available = true;
                }
            }
        }
        if (item != nullptr) {
            env->DeleteLocalRef(item);
        }
    }
    release();
}

vector<native_method_info> enum_static_jni_func(JNIEnv *env, const string &reg_module_name) {
    auto handle = fake_dlopen(reg_module_name.c_str(), 0);
    map<string, vector<native_method_info *>> need_query;
    vector<native_method_info> infos;
    for (int index = 0; index < handle->nsyms; index++) {
        const char *sym_name = fake_get_sym_name(handle, index);
//        logi("sym   %s", sym_name);
        native_method_info info;
        if (parse_java_lang_name(sym_name, info.class_name, info.method_name, info.args_type,
                                 info.is_override)) {
            info.addr = fake_dlsym(handle, sym_name);
            infos.push_back(info);
        }
    }
    for (auto &info :infos) {
        if (!info.is_override) {
            need_query[info.class_name].push_back(&info);
        }
    }
    for (auto &kv: need_query) {
        get_method_info(env, kv.first, kv.second);
    }
    fake_dlclose(handle);
    return infos;
}

void *get_jni_onload_ptr(const string &reg_module_name) {
    auto handle = fake_dlopen(reg_module_name.c_str(), 0);
    if (handle == nullptr) {
        return nullptr;
    }
    void *ret = fake_dlsym(handle, "JNI_OnLoad");
    fake_dlclose(handle);
    return ret;
}




