#pragma once

#include <regex.h>
#include <dirent.h>
#include <string>
#include <memory>
#include <atomic>
#include <set>

#include "filter.h"
#include "factory.h"
#include "../utils/log.h"

using std::atomic;
using std::string;
using std::shared_ptr;
using std::set;

class hook_info {
public:
    int file_instance_id;
    string path;
};

class base_hook_handle {
public:
    hook_info info;

    virtual ~base_hook_handle() {};
};


class inside_hook_params {
public:
    inside_hook_params(const hook_params &params) {
        this->reg_path = params.reg_path;
        int err = regcomp(&this->reg, params.reg_path.c_str(), REG_EXTENDED);
        if (err != 0) {
            loge("%s %s", "regcomp reg error!", params.reg_path.c_str());
        }
//        this->has_pid = params.has_pid;
        this->ffactory = params.ffactory;
        this->open_index = shared_ptr<atomic<int>>(new atomic<int>(0));
    }

    ~inside_hook_params() {
        regfree(&reg);
    }

    regex_t reg;
    string reg_path;
//    bool has_pid = false;
    filter_factory *ffactory;
    shared_ptr<atomic<int>> open_index;
};

#define is_hook_handle(x) _is_hook_handle((int)x)

bool _is_hook_handle(void *handle);

#define remove_hook_handle(x) _remove_hook_handle((void *)x)

void _remove_hook_handle(void *handle);

#define insert_hook_handle(handle, phook) _insert_hook_handle((void *)handle,phook)

void _insert_hook_handle(void *handle, base_hook_handle *phook);

#define get_hook_handle(x) _get_hook_handle((void *)x)

base_hook_handle *_get_hook_handle(void *handle);

int get_pid();

#include <dlfcn.h>
//#include "../inlineHook/inlineHook.h"
#include "../dlfc/dlfcn_nougat.h"


#define INLINE_HOOK_DECL(handle, name)    void* tar_##name = fake_dlsym(handle, #name);\
                            if (ELE7EN_OK == registerInlineHook((uint32_t)tar_##name,\
                            (uint32_t)hook_##name,\
                            (uint32_t**)&phook_##name)) {\
                                if (ELE7EN_OK == inlineHook((uint32_t) tar_##name)) {\
                                    /*LOGE("inlineHook RegisterNative success");*/\
                                }\
                            }


#include "../libxhook/xhook.h"
#include "../dobby/include/dobby.h"

#define ELF_HOOK_DECL(rs, t, x) xhook_register(rs.c_str(),#x,(void*)hook_##x,(void**)&t.phook_##x)
//#define ELF_HOOK_IGNORE(rs, s) xhook_ignore(rs,s)
//#define ELF_HOOK_CLEAR  xhook_clear
#define ELF_HOOK_REFRESH(X) xhook_refresh(0)
//#define ELF_HOOK_DEBUG(x) xhook_enable_debug(x)
//#define ELF_HOOK_SFP(x) xhook_enable_sigsegv_protection(x)

