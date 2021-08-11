#pragma once

#include <jni.h>


void init_jni_hook(JNIEnv *env, const string &reg_module_name);

extern "C" JNIEXPORT void JNICALL
init_jni_hook(JNIEnv *env, const char *module_name);
