#pragma once

//#include "hook_android.h"
//#include "hook_file.h"
//#include "hook_other.h"
#include <dlfcn.h>

#include "../utils/log.h"
#include "../utils/utils.h"

#include "factory.h"
#include "filter.h"
#include "memory_file.h"
#include "only_watch.h"

bool hook_global_init();

void init_other_hook();

void init_file_hook();

int register_file_hook(const hook_params &info);

void unregister_file_hook(int mhandle);