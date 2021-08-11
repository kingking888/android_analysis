#include <dlfcn.h>
#include "../utils/log.h"
#include "../route_func/printf_java_args.h"
#include "../route_func/printf_args_helper.h"
#include "../route_func/route_func.h"
#include "../route_func/methods_info.h"
#include "../dlfc/dlfcn_nougat.h"
#include "../libxhook/xhook.h"

#if defined(__arm64__) || defined(__aarch64__)

int64_t hook_libc_func(int64_t x0,
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

uint32_t hook_libc_func(uint32_t r0,
                        uint32_t r1,
                        uint32_t r2,
                        uint32_t r3,
                        void *context,
                        void *stack_args)
#endif
{
    method_info_t *pcontext = (method_info_t *) context;
    logd("hook libc methods %s", pcontext->name.c_str());
    int stack_args_count = 0;
    if (pcontext->index <= 6) {
        const char *pfmt;
        switch (pcontext->args_type.size()) {
            case 2:
                pfmt = (const char *) REG_ID(0);
                break;
            case 3:
                pfmt = (const char *) REG_ID(1);
                break;
            case 4:
                pfmt = (const char *) REG_ID(2);
                break;
        }
        while (*pfmt != 0) {
            if (*pfmt == '%') {
                if (*(pfmt + 1) == '%') {
                    pfmt += 2;
                    continue;
                }
                stack_args_count++;
            }
            pfmt++;
        }
        stack_args_count--;
    }

#if defined(__arm64__) || defined(__aarch64__)
    stack_args_count += pcontext->args_type.size() - 8;
    if (stack_args_count < 0) {
        stack_args_count = 0;
    }
    int64_t ret = route_jni_func(x0, x1, x2, x3, x4, x5, x6, x7,
                                 stack_args_count,
                                 stack_args,
                                 pcontext->origin_call);
#else
    stack_args_count += pcontext->args_type.size() - 4;
    if (stack_args_count < 0) {
        stack_args_count = 0;
    }
    uint32_t ret = route_jni_func(r0, r1, r2, r3,
                                  stack_args_count,
                                  stack_args,
                                  pcontext->origin_call);
#endif

#if defined(__arm64__) || defined(__aarch64__)
    uint64_t args_offset = 0;
    printf_java_func_args(nullptr, args_offset, x0, x1, x2, x3, x4, x5, x6, x7, pcontext,
                          stack_args, pcontext->args_type, pcontext->args_type.size());
#else
    uint32_t args_offset = 0;
    printf_java_func_args(nullptr, args_offset, REG_ID(0), REG_ID(1), REG_ID(2), REG_ID(3),
                          pcontext, stack_args, pcontext->args_type, pcontext->args_type.size());
#endif

    logd("                       ret: %s",
         format_args(nullptr, pcontext->ret_type, ret).c_str());
    return ret;
}

extern "C" JNIEXPORT void JNICALL
init_only_watch(const char *module_name) {
    for (int index = 0; libc_only_watch_method_info[index].index != 0; index++) {
        auto shell_code = create_shellcode_hooking_func(hook_libc_func,
                                                        &libc_only_watch_method_info[index]);
        xhook_register(module_name, libc_only_watch_method_info[index].name.c_str(), shell_code,
                       &libc_only_watch_method_info[index].origin_call);
    }
    xhook_refresh(0);
}