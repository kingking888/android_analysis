#pragma once

#include <jni.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

struct native_method_info {
    string class_name;
    string method_name;
    bool is_override = false;
    //暂未保存native函数前两个参数
    vector<string> args_type;
    bool available = false;
    void *addr;
    void *origin_call;
//    string args_sig;
    string ret_type;
};

vector<native_method_info> enum_static_jni_func(JNIEnv *env, const string &reg_module_name);

void *get_jni_onload_ptr(const string &reg_module_name);