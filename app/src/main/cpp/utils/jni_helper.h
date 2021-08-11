#pragma once

#include <jni.h>
#include <string>
#include <vector>

#include "../dlfc/dlfcn_nougat.h"

using std::string;
using std::vector;

jstring char2jstring(JNIEnv *env, const char *pat);

jstring string2jstring(JNIEnv *env, const string &str);

string jstring2str(JNIEnv *env, jstring jstr);

vector<jobject> get_java_list(JNIEnv *env, jobject obj_list);


enum class method_name_type {
    short_name,
    long_name,
    pretty_name
};

////    2021-05-30 18:07:28.204 17909-17909/com.local_caller.xhs D/local_call: Java_java_lang_Class_forName
////    2021-05-30 18:07:28.204 17909-17909/com.local_caller.xhs D/local_call: Java_java_lang_Class_forName__Ljava_lang_String_2
////    2021-05-30 18:07:28.204 17909-17909/com.local_caller.xhs D/local_call: java.lang.Class.forName
////    2021-05-30 18:07:28.204 17909-17909/com.local_caller.xhs D/local_call: java.lang.Class java.lang.Class.forName(java.lang.String)
string get_method_name(jmethodID method_id, method_name_type type, bool with_sign = true);

string get_object_class_name(JNIEnv *env, jobject obj);

jthrowable clean_exception(JNIEnv *env);