#include <jni.h>
#include <string>
#include <map>
#include <mutex>

using std::mutex;
using std::string;
using std::map;

#include "../dobby/include/dobby.h"
#include "../route_func/route_func.h"
#include "../utils/log.h"
#include "../utils/jni_helper.h"
#include "jni_hook.h"
#include "jni_env_hook.h"
#include "enum_jni_static_method.h"
#include "parse_java_sig.h"
#include "../route_func/printf_java_args.h"
#include "../route_func/printf_args_helper.h"
#include "global_code.h"

void *origin_jni_onload_ptr = nullptr;
vector<native_method_info> static_jni_func_infos;
vector<native_method_info *> reg_jni_func_infos;


#if defined(__arm64__) || defined(__aarch64__)

int64_t hook_native_func(int64_t x0,
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

uint32_t hook_native_func(uint32_t r0,
                          uint32_t r1,
                          uint32_t r2,
                          uint32_t r3,
                          void *context,
                          void *stack_args)
#endif
{
    auto *pcontext = (native_method_info *) context;
    logd("hook native call %s->%s", pcontext->class_name.c_str(), pcontext->method_name.c_str());
//    logd("reg %p %p %p %p", r0, r1, r2, r3);
    JNIEnv *origin_env = (JNIEnv *) REG_ID(0);
    push_origin_env(origin_env);

#if defined(__arm64__) || defined(__aarch64__)

    uint64_t java_args_offset = 2 * REG_BIT_SIZE;
    printf_java_func_args(origin_env, java_args_offset, x0, x1, x2, x3, x4, x5, x6, x7, pcontext,
                          stack_args, pcontext->args_type, pcontext->args_type.size());

    int64_t ret = route_jni_func((uint64_t) g_fake_env, x1, x2, x3, x4, x5, x6, x7,
                                 pcontext->args_type.size() + 2 >= 8 ?
                                 pcontext->args_type.size() - 8 + 2 : 0,
                                 stack_args,
                                 pcontext->origin_call);
#else
    uint32_t java_args_offset = 2 * REG_BIT_SIZE;
    printf_java_func_args(origin_env, java_args_offset, REG_ID(0), REG_ID(1), REG_ID(2), REG_ID(3),
                          pcontext, stack_args, pcontext->args_type, pcontext->args_type.size());

    uint32_t ret = route_jni_func((uint32_t) g_fake_env, r1, r2, r3, pcontext->args_type.size() +2>= 4 ?
                                 pcontext->args_type.size() - 4+2: 0,
                                  stack_args,
                                  pcontext->origin_call);
#endif

    logd("native call %s->%s  ret: %s", pcontext->class_name.c_str(), pcontext->method_name.c_str(),
         format_args(origin_env, pcontext->ret_type, ret).c_str());
    return ret;
}

jint hook_jni_onload(JavaVM *vm, void *reserved) {
    logd("%s", "hook native call jni_onload");
    return ((jint (*)(JavaVM *, void *)) origin_jni_onload_ptr)(g_fake_jvm, reserved);
}

jint hook_RegisterNatives(JNIEnv *env,
                          jclass c,
                          JNINativeMethod *method,
                          jint num,
                          func_type_RegisterNatives origin_call) {
    logd("%s", "hook native call RegisterNatives");
    JNINativeMethod *fake_method = new JNINativeMethod[num];
    for (int index = 0; index < num; ++index) {
        fake_method[index].fnPtr = method[index].fnPtr;
        fake_method[index].signature = method[index].signature;
        fake_method[index].name = method[index].name;

        native_method_info *info = new native_method_info;
        bool is_repeat = false;
        for (const auto &item: reg_jni_func_infos) {
            if (item->addr == method[index].fnPtr) {
                is_repeat = true;
                break;
            }
        }
        if (is_repeat) {
            logd("hook_RegisterNatives parse_java_signature %s : %s is_repeat!!!",
                 method[index].name,
                 method[index].signature);
            continue;
        }
        if (!parse_java_signature(method[index].signature, info->args_type, info->ret_type)) {
            logd("hook_RegisterNatives parse_java_signature %s : %s error!!!",
                 method[index].name,
                 method[index].signature);
            continue;
        }
        info->method_name = method[index].name;
        info->addr = method[index].fnPtr;
        info->origin_call = method[index].fnPtr;
        unsigned char *shell_code = create_shellcode_hooking_func(hook_native_func, info);
        logd("hook_RegisterNatives  %s : %s %p -> %p",
             method[index].name,
             method[index].signature,
             method[index].fnPtr,
             shell_code);

        fake_method[index].fnPtr = shell_code;
        reg_jni_func_infos.push_back(info);
    }

    jint ret = origin_call(env, c, fake_method, num);
    delete fake_method;
    return ret;
}


void init_jni_hook(JNIEnv *env, const string &reg_module_name) {
    static_jni_func_infos = enum_static_jni_func(env, reg_module_name);
    g_fake_env = create_hook_jni_env(env, &hook_RegisterNatives);
    for (auto &item :static_jni_func_infos) {
        if (!item.available) {
            continue;
        }
        unsigned char *shell_code = create_shellcode_hooking_func(hook_native_func, &item);
        DobbyHook(item.addr, shell_code, &item.origin_call);
    }

    env->GetJavaVM(&g_origin_jvm);
    g_fake_jvm = create_hook_java_vm(g_origin_jvm);

    void *jni_onload_ptr = get_jni_onload_ptr(reg_module_name);
    if (jni_onload_ptr) {
        DobbyHook(jni_onload_ptr, (void *) &hook_jni_onload, &origin_jni_onload_ptr);
    }
}

extern "C" JNIEXPORT void JNICALL
init_jni_hook(JNIEnv *env, const char *module_name) {
    init_jni_hook(env, std::string(module_name));
}