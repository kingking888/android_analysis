
#include <jni.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>

#include "../utils/log.h"
#include "../utils/jni_helper.h"
#include "../route_func/route_func.h"
#include "../route_func/methods_info.h"
#include "../route_func/printf_java_args.h"
#include "../route_func/printf_args_helper.h"
#include "jni_env_hook.h"
#include "parse_java_sig.h"
#include "global_code.h"

using std::string;
using std::vector;


#if defined(__arm64__) || defined(__aarch64__)

int64_t hook_call_methods(int64_t x0,
                          int64_t x1,
                          int64_t x2,
                          int64_t x3,
                          int64_t x4,
                          int64_t x5,
                          int64_t x6,
                          int64_t x7,
                          void *context,
                          void *stack_args)
#else

uint32_t hook_call_methods(uint32_t r0,
                           uint32_t r1,
                           uint32_t r2,
                           uint32_t r3,
                           void *context,
                           void *stack_args)
#endif
{
    auto *pcontext = (method_info_t *) context;
    logd("hook jni call methods %s", pcontext->name.c_str());
    JNIEnv *origin_env = get_origin_env();

#if defined(__arm64__) || defined(__aarch64__)
    uint64_t java_args_offset = 0;
    printf_java_func_args(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5, x6, x7, pcontext,
                          stack_args, pcontext->args_type, pcontext->args_type.size() - 1);
#else
    uint32_t java_args_offset = 0;
    printf_java_func_args(origin_env, java_args_offset, REG_ID(0), REG_ID(1), REG_ID(2), REG_ID(3),
                          pcontext,
                          stack_args, pcontext->args_type, pcontext->args_type.size() - 1);
#endif
    jmethodID method_id;
    if (pcontext->index <= 63) {
        method_id = (jmethodID) REG_ID(2);
    } else if (pcontext->index <= 93) {
        method_id = (jmethodID) REG_ID(3);
    } else if (pcontext->index <= 143) {
        method_id = (jmethodID) REG_ID(2);
    } else {
        logd("hook jni occour error!!! %s", pcontext->name.c_str());
        return 0;
    }

    string method_pretty_name = get_method_name(method_id, method_name_type::pretty_name);
    if (method_pretty_name.empty()) {
        logd("%s", "hook jni get_method_name occour error!!!");
        return 0;
    }

    string class_name;
    string method_name;
    vector<string> args_type;
    string ret_type;
    if (!parse_java_pretty_name(method_pretty_name, class_name, method_name, args_type, ret_type)) {
        logd("%s", "hook jni parse_java_pretty_name occour error!!!");
        return 0;
    }

    int mod_index = pcontext->index % 3;
    int stack_args_count;
    if (mod_index == 0) {
        stack_args_count = args_type.size() + pcontext->args_type.size() - REG_COUNT - 1;
    } else {
        stack_args_count = pcontext->args_type.size() - REG_COUNT;
    }
    if (stack_args_count < 0) {
        stack_args_count = 0;
    }
    logd("                            java method %s", method_pretty_name.c_str());
#if defined(__arm64__) || defined(__aarch64__)

    if (mod_index == 0) {
        printf_java_func_args(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5, x6, x7,
                              pcontext, stack_args, args_type, args_type.size());
    } else if (mod_index == 1) {
        printf_java_func_args_end_with_va_list(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5,
                                               x6, x7, pcontext, stack_args, args_type);
    } else {
        printf_java_func_args_end_with_array(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5,
                                             x6, x7, pcontext, stack_args, args_type);
    }

    int64_t ret = route_jni_func((int64_t) origin_env, x1, x2, x3, x4, x5, x6, x7,
                                 stack_args_count,
                                 stack_args,
                                 pcontext->origin_call);

#else

    if (mod_index == 0) {
        printf_java_func_args(origin_env, java_args_offset, REG_ID(0), REG_ID(1), REG_ID(2),
                              REG_ID(3), pcontext, stack_args, args_type, args_type.size());
    } else if (mod_index == 1) {
        printf_java_func_args_end_with_va_list(origin_env, java_args_offset, REG_ID(0), REG_ID(1),
                                               REG_ID(2), REG_ID(3), pcontext, stack_args,
                                               args_type);
    } else {
        printf_java_func_args_end_with_array(origin_env, java_args_offset, REG_ID(0), REG_ID(1),
                                             REG_ID(2), REG_ID(3), pcontext, stack_args, args_type);
    }
    uint32_t ret = route_jni_func((uint32_t) origin_env, REG_ID(1), REG_ID(2), REG_ID(3),
                                  stack_args_count, stack_args, pcontext->origin_call);
#endif

    logd("                            ret: %s",
         format_args(origin_env, ret_type, ret).c_str());
    return ret;
}

func_type_call_back_RegisterNatives call_back_JNINativeMethod = nullptr;

#if defined(__arm64__) || defined(__aarch64__)

int64_t hook_other_methods(int64_t x0,
                           int64_t x1,
                           int64_t x2,
                           int64_t x3,
                           int64_t x4,
                           int64_t x5,
                           int64_t x6,
                           int64_t x7,
                           void *context,
                           void *stack_args)
#else

uint32_t hook_other_methods(uint32_t r0,
                            uint32_t r1,
                            uint32_t r2,
                            uint32_t r3,
                            void *context,
                            void *stack_args)
#endif
{
    method_info_t *pcontext = (method_info_t *) context;
    logd("hook jni call methods %s", pcontext->name.c_str());
//    logd("reg %p %p %p %p", r0, r1, r2, r3);
    JNIEnv *origin_env = get_origin_env();

#if defined(__arm64__) || defined(__aarch64__)
    uint64_t java_args_offset = 0;
    printf_java_func_args(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5, x6, x7, pcontext,
                          stack_args, pcontext->args_type, pcontext->args_type.size());
#else
    uint32_t java_args_offset = 0;
    printf_java_func_args(origin_env, java_args_offset, REG_ID(0), REG_ID(1), REG_ID(2), REG_ID(3),
                          pcontext, stack_args, pcontext->args_type, pcontext->args_type.size());
#endif

    if (pcontext->index == 215) {
//      jint JNINativeMethod(JNIEnv *, jclass, JNINativeMethod *, jint)
        return call_back_JNINativeMethod((JNIEnv *) origin_env,
                                         (jclass) REG_ID(1),
                                         (JNINativeMethod *) REG_ID(2),
                                         (jint) REG_ID(3),
                                         (func_type_RegisterNatives) pcontext->origin_call);
    }

#if defined(__arm64__) || defined(__aarch64__)
    int64_t ret = route_jni_func((int64_t) origin_env, x1, x2, x3, x4, x5, x6, x7,
                                 pcontext->args_type.size() >= 8 ?
                                 pcontext->args_type.size() - 8 : 0,
                                 stack_args,
                                 pcontext->origin_call);
#else
    uint32_t ret = route_jni_func((int32_t) origin_env, r1, r2, r3, pcontext->args_type.size() >= 4 ?
                                 pcontext->args_type.size() - 4: 0,
                                  stack_args,
                                  pcontext->origin_call);
#endif

    logd("                       ret: %s",
         format_args(origin_env, pcontext->ret_type, ret).c_str());
    return ret;
}

JNIEnv *
create_hook_jni_env(JNIEnv *tar_env, func_type_call_back_RegisterNatives call_back) {
    call_back_JNINativeMethod = call_back;
    void **tar_interface = (void **) tar_env->functions;
    JNIEnv *new_jni = new JNIEnv;
    JNINativeInterface *new_interface = new JNINativeInterface;
    new_interface->reserved0 = tar_env->functions->reserved0;
    new_interface->reserved1 = tar_env->functions->reserved1;
    new_interface->reserved2 = tar_env->functions->reserved2;
    new_interface->reserved3 = tar_env->functions->reserved3;

    new_pfunc_t hook_funcs[] = {hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_call_methods};

    for (int out_index = 0; jni_all_methods[out_index] != nullptr; ++out_index) {
        for (int index = 0; jni_all_methods[out_index][index].index != 0; index++) {
            auto shell_code = create_shellcode_hooking_func(hook_funcs[out_index],
                                                            &jni_all_methods[out_index][index]);
            jni_all_methods[out_index][index].origin_call = tar_interface[jni_all_methods[out_index][index].index];
            ((void **) new_interface)[jni_all_methods[out_index][index].index] = shell_code;
        }
    }
    new_jni->functions = new_interface;
    return new_jni;
}

//jni_get_value_method_info,
//jni_set_value_method_info,
//jni_release_method_info,
//jni_new_method_info,
//jni_class_method_info,
//jni_jni_method_info,
//jni_except_method_info,
//jni_other_method_info,
//jni_call_method_info,

void init_global_hook_jni_env(JNIEnv *tar_env, func_type_call_back_RegisterNatives call_back) {
    call_back_JNINativeMethod = call_back;
    void **tar_interface = (void **) tar_env->functions;
    SET_MEMORY_PROTECT(tar_interface, PROT_READ | PROT_WRITE);

    new_pfunc_t hook_funcs[] = {hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_other_methods,
                                hook_call_methods};

    for (int out_index = 0; jni_all_methods[out_index] != nullptr; ++out_index) {
        for (int index = 0; jni_all_methods[out_index][index].index != 0; index++) {
            auto shell_code = create_shellcode_hooking_func(hook_funcs[out_index],
                                                            &jni_all_methods[out_index][index]);
            jni_all_methods[out_index][index].origin_call = tar_interface[jni_all_methods[out_index][index].index];
            tar_interface[jni_all_methods[out_index][index].index] = shell_code;
        }
    }
}

//struct JNIInvokeInterface {
//    void*       reserved0;
//    void*       reserved1;
//    void*       reserved2;
//
//    jint        (*DestroyJavaVM)(JavaVM*);
//    jint        (*AttachCurrentThread)(JavaVM*, JNIEnv**, void*);
//    jint        (*DetachCurrentThread)(JavaVM*);
//    jint        (*GetEnv)(JavaVM*, void**, jint);
//    jint        (*AttachCurrentThreadAsDaemon)(JavaVM*, JNIEnv**, void*);
//};


#if defined(__arm64__) || defined(__aarch64__)

int64_t hook_jvm_methods(int64_t x0,
                         int64_t x1,
                         int64_t x2,
                         int64_t x3,
                         int64_t x4,
                         int64_t x5,
                         int64_t x6,
                         int64_t x7,
                         void *context,
                         void *stack_args)
#else

uint32_t hook_jvm_methods(uint32_t r0,
                          uint32_t r1,
                          uint32_t r2,
                          uint32_t r3,
                          void *context,
                          void *stack_args)
#endif
{
    method_info_t *pcontext = (method_info_t *) context;
    logd("hook jvm call methods %s", pcontext->name.c_str());
    //      jint (*GetEnv)(JavaVM *, void **, jint);
    if (pcontext->index == 6) {
        JNIEnv *new_env = nullptr;
        if (g_origin_jvm->GetEnv((void **) &new_env, REG_ID(2)) != JNI_OK) {
            logi("%s", "hook_jvm_methods  jvm->GetEnv error!!!");
        }
        push_origin_env(new_env);
        *((JNIEnv **) REG_ID(1)) = g_fake_env;
        return JNI_OK;
    }

#if defined(__arm64__) || defined(__aarch64__)
    int64_t ret = route_jni_func(x0, x1, x2, x3, x4, x5, x6, x7,
                                 pcontext->args_type.size() >= 8 ?
                                 pcontext->args_type.size() - 8 : 0,
                                 stack_args,
                                 pcontext->origin_call);
#else
    uint32_t ret = route_jni_func(r0, r1, r2, r3, pcontext->args_type.size() >= 4 ?
                                 pcontext->args_type.size() - 4: 0,
                                  stack_args,
                                  pcontext->origin_call);
#endif

    return ret;
}

JavaVM *create_hook_java_vm(JavaVM *jvm) {
    JNIInvokeInterface *tar_jvm = (JNIInvokeInterface *) jvm->functions;
    JNIInvokeInterface *new_jvm_interface = new JNIInvokeInterface;

    JavaVM *new_jvm = new JavaVM;
    new_jvm_interface->reserved0 = tar_jvm->reserved0;
    new_jvm_interface->reserved1 = tar_jvm->reserved1;
    new_jvm_interface->reserved2 = tar_jvm->reserved2;

    for (int index = 0; jni_jvm_method_info[index].index != 0; index++) {
        auto shell_code = create_shellcode_hooking_func(hook_jvm_methods,
                                                        &jni_jvm_method_info[index]);

        jni_jvm_method_info[index].origin_call = ((void **) tar_jvm)[jni_jvm_method_info[index].index];
        ((void **) new_jvm_interface)[jni_jvm_method_info[index].index] = shell_code;
    }
    new_jvm->functions = new_jvm_interface;
    return new_jvm;
}

void init_hook_java_vm(JavaVM *jvm) {
    JNIInvokeInterface *tar_jvm = (JNIInvokeInterface *) jvm->functions;
    SET_MEMORY_PROTECT(tar_jvm, PROT_READ | PROT_WRITE);

    for (int index = 0; jni_jvm_method_info[index].index != 0; index++) {
        auto shell_code = create_shellcode_hooking_func(hook_jvm_methods,
                                                        &jni_jvm_method_info[index]);

        jni_jvm_method_info[index].origin_call = ((void **) tar_jvm)[jni_jvm_method_info[index].index];
        ((void **) tar_jvm)[jni_jvm_method_info[index].index] = shell_code;
    }
}







//#ifndef VIRTUALAPP_CAMERA_HOOK_H
//#define VIRTUALAPP_CAMERA_HOOK_H
//
//#include <jni.h>
//#include <stdlib.h>
//#include <android/log.h>
//#include <dlfcn.h>
//#include <stddef.h>
//#include <fcntl.h>
//#include <sys/system_properties.h>
//#include <stdio.h>
//
//#define TAG "HOOKCAMERA"
//#define JAVA_CLASS "com/baidu/freeinstall/client/ContextFixer"
//
//
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,  TAG, __VA_ARGS__)
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
//#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
//
//        __BEGIN_DECLS
//JNIEXPORT jint
//
//JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);
//
//JNIEXPORT void JNICALL
//JNI_OnUnload(JavaVM
//* vm,
//void *reserved
//);
//__END_DECLS
//
//#endif //VIRTUALAPP_CAMERA_HOOK_H
//
//        camera_hook
//.
//c
//
//#include "camera_hook.h"
//
//        JavaVM
//*
//g_vm;
//
//typedef void (*Bridge_DalvikBridgeFunc)(const void **, void *, const void *, void *);
//
//typedef jobject (*Native_nativeSetUpFunc)(JNIEnv *, jclass, jobject weak_this, jint cameraId,
//                                          jint halVersion, jstring clientPackageName);
//
//typedef jobject (*Native_nativeSetUpDalvikFunc)(JNIEnv *, jclass,
//                                                jobject weak_this, jint halVersion,
//                                                jstring clientPackageName);
//
//
//void mark() {
//    // Do nothing
//};
//
//
//static struct {
//    bool isArt;
//    int nativeOffset;
//    void *art_work_around_app_jni_bugs;
//
//    char *(*GetCstrFromString)(void *);
//
//    void *(*GetStringFromCstr)(const char *);
//
//    Bridge_DalvikBridgeFunc orig_DalvikBridgeFunc;
//    Native_nativeSetUpFunc orig_native_openDexNativeFunc;
//    Native_nativeSetUpDalvikFunc orig_native_openDexNativeDalvikFunc;
//} gOffset;
//
//char *newPkgName;
//
//static jobject new_nativeSetUpFunc(JNIEnv *env, jclass jclazz,
//                                   jobject weak_this, jint cameraId, jint halVersion,
//                                   jstring clientPackageName) {
//
//    jstring newPkg = env->NewStringUTF(newPkgName);
//    return gOffset.orig_native_openDexNativeFunc(env, jclazz, weak_this, cameraId, halVersion,
//                                                 newPkg);
//
//}
//
//static jobject new_nativeSetUpDalvikFunc(JNIEnv *env, jclass jclazz,
//                                         jobject weak_this, jint halVersion,
//                                         jstring clientPackageName) {
//
//    jstring newPkg = env->NewStringUTF(newPkgName);
//
//    return gOffset.orig_native_openDexNativeDalvikFunc(env, jclazz, weak_this, halVersion, newPkg);
//
//}
//
//static void
//new_bridge_nativeSetUpFunc(const void **args, void *pResult, const void *method, void *self) {
//
//    JNIEnv *env = NULL;
//    g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
//    g_vm->AttachCurrentThread(&env, NULL);
//
//    typedef char *(*GetCstrFromString)(void *);
//    typedef void *(*GetStringFromCstr)(const char *);
//
//    const char *origPkg0 = args[0] == NULL ? NULL : gOffset.GetCstrFromString((void *) args[0]);
//    LOGD("The original package0 is: %s", origPkg0);
//
//    const char *origPkg = args[2] == NULL ? NULL : gOffset.GetCstrFromString((void *) args[2]);
//    LOGD("The original package2 is: %s", origPkg);
//
//    const char *origPkg3 = args[3] == NULL ? NULL : gOffset.GetCstrFromString((void *) args[3]);
//    LOGD("The original package3 is: %s", origPkg3);
//
//    args[3] = gOffset.GetStringFromCstr(newPkgName);
//    gOffset.orig_DalvikBridgeFunc(args, pResult, method, self);
//}
//
//
//void searchJniOffset(JNIEnv *env, bool isArt) {
//
//    jclass g_class = env->FindClass(JAVA_CLASS);
//    jmethodID mtd_nativeHook = env->GetStaticMethodID(g_class, "nativeMark", "()V");
//
//    size_t startAddress = (size_t) mtd_nativeHook;
//    size_t targetAddress = (size_t) mark;
//    if (isArt && gOffset.art_work_around_app_jni_bugs) {
//        targetAddress = (size_t) gOffset.art_work_around_app_jni_bugs;
//    }
//
//    int offset = 0;
//    bool found = false;
//    while (true) {
//        if (*((size_t * )(startAddress + offset)) == targetAddress) {
//            found = true;
//            break;
//        }
//        offset += 4;
//        if (offset >= 100) {
//            LOGE("Ops: Unable to find the jni function.");
//            break;
//        }
//    }
//
//    if (found) {
//        gOffset.nativeOffset = offset;
//        if (!isArt) {
//            gOffset.nativeOffset += (sizeof(int) + sizeof(void *));
//        }
//        LOGD("Hoho, Get the offset : %d.", gOffset.nativeOffset);
//    }
//}
//
//inline void replaceImplementation(JNIEnv *env, jobject javaMethod, jboolean isArt) {
//
//    size_t mtd_openDexNative = (size_t) env->FromReflectedMethod(javaMethod);
//    int nativeFuncOffset = gOffset.nativeOffset;
//    void **jniFuncPtr = (void **) (mtd_openDexNative + nativeFuncOffset);
//
//    if (!isArt) {
//        LOGD("The vm is dalvik");
//        gOffset.orig_DalvikBridgeFunc = (Bridge_DalvikBridgeFunc) (*jniFuncPtr);
//        *jniFuncPtr = (void *) new_bridge_nativeSetUpFunc;
//    } else {
//        char vmSoName[4] = {0};
//        __system_property_get("ro.build.version.sdk", vmSoName);
//        int sdk;
//        sscanf(vmSoName, "%d", &sdk);
//        LOGD("The vm is art and the sdk int is %d", sdk);
//
//        if (sdk < 21) {
//            gOffset.orig_native_openDexNativeDalvikFunc = (Native_nativeSetUpDalvikFunc) (*jniFuncPtr);
//            *jniFuncPtr = (void *) new_nativeSetUpDalvikFunc;
//        } else {
//            gOffset.orig_native_openDexNativeFunc = (Native_nativeSetUpFunc) (*jniFuncPtr);
//            *jniFuncPtr = (void *) new_nativeSetUpFunc;
//        }
//
//    }
//}
//
//static JNINativeMethod gMarkMethods[] = {
//        {"nativeMark", "()V", (void *) mark}
//};
//
//void native_hook(JNIEnv *env, jclass clazz, jobject method, jstring pkg, jboolean isArt) {
//
//    newPkgName = (char *) env->GetStringUTFChars(pkg, NULL);
//    g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
//    g_vm->AttachCurrentThread(&env, NULL);
//
//    jclass g_class = env->FindClass(JAVA_CLASS);
//    if (env->RegisterNatives(g_class, gMarkMethods, 1) < 0) {
//        return;
//    }
//
//    gOffset.isArt = isArt;
//
//    char vmSoName[15] = {0};
//    __system_property_get("persist.sys.dalvik.vm.lib", vmSoName);
//    LOGD("Find the so name : %s.", strlen(vmSoName) == 0 ? "<EMPTY>" : vmSoName);
//
//    void *vmHandle = dlopen(vmSoName, 0);
//    if (!vmHandle) {
//        LOGE("Unable to open the %s.", vmSoName);
//        vmHandle = RTLD_DEFAULT;
//    }
//
//
//    if (isArt) {
//        gOffset.art_work_around_app_jni_bugs = dlsym(vmHandle, "art_work_around_app_jni_bugs");
//    } else {
//        gOffset.GetCstrFromString = (char *(*)(void *)) dlsym(vmHandle,
//                                                              "_Z23dvmCreateCstrFromStringPK12StringObject");
//        if (!gOffset.GetCstrFromString) {
//            gOffset.GetCstrFromString = (char *(*)(void *)) dlsym(vmHandle,
//                                                                  "dvmCreateCstrFromString");
//        }
//
//        gOffset.GetStringFromCstr = (void *(*)(const char *)) dlsym(vmHandle,
//                                                                    "_Z23dvmCreateStringFromCstrPKc");
//        if (!gOffset.GetStringFromCstr) {
//            gOffset.GetStringFromCstr = (void *(*)(const char *)) dlsym(vmHandle,
//                                                                        "dvmCreateStringFromCstr");
//        }
//    }
//
//    searchJniOffset(env, isArt);
//    replaceImplementation(env, method, isArt);
//}
//
//
//static JNINativeMethod gMethods[] = {
//        {"hookNativeMethod", "(Ljava/lang/Object;Ljava/lang/String;Z)Z",
//                (void *) native_hook}
//};
//
//JNIEXPORT jint
//
//JNICALL JNI_OnLoad(JavaVM * vm, void * reserved) {
//    g_vm = vm;
//    JNIEnv *env;
//
//    LOGE("JNI_Onload start");
//    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
//        LOGE("GetEnv() FAILED!!!");
//        return JNI_ERR;
//    }
//
//    jclass javaClass = env->FindClass(JAVA_CLASS);
//    LOGE("we have found the class: %s", JAVA_CLASS);
//    if (javaClass == NULL) {
//        LOGE("unable to find class: %s", JAVA_CLASS);
//        return JNI_ERR;
//    }
//
//    env->UnregisterNatives(javaClass);
//    if (env->RegisterNatives(javaClass, gMethods, 1) < 0) {
//        LOGE("register methods FAILED!!!");
//        return JNI_ERR;
//    }
//
//    env->DeleteLocalRef(javaClass);
//    LOGI("JavaVM::GetEnv() SUCCESS!");
//    return JNI_VERSION_1_6;
//}
