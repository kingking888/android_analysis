#pragma once

#include <jni.h>

extern "C" JNIEXPORT void JNICALL
init_only_watch(const char *module_name);