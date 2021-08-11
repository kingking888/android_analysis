#pragma once

#include <android/log.h>
#include <jni.h>


#include <cstdarg>
#include <cstdio>
#include <mutex>

using std::mutex;

class log {
    static const int buffer_size = 1024 * 1024 * 10;
    static char *buffer;
    static FILE *log_file;
    static mutex lock;
public:
    log() {
        buffer = new char[buffer_size];
        log_file = fopen("/sdcard/log.txt", "w");
    }

    ~log() {
        fclose(log_file);
    }

    static void log2file(const char *fmt, ...) {
        lock.lock();
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, buffer_size, fmt, args);
        va_end(args);
        fwrite(buffer, strlen(buffer), 1, log_file);
        fwrite("\n", 1, 1, log_file);
        lock.unlock();
    }
};


#define TAG "android-anayise"

#define NDK_LOG
#ifdef LOG2FILE

#define log_print_verbose(fmt, ...) log::log2file(fmt, __VA_ARGS__)
#define log_print_debug(fmt, ...)  log::log2file(fmt, __VA_ARGS__)
#define log_print_info(fmt, ...)  log::log2file(fmt, __VA_ARGS__)
#define log_print_warn(fmt, ...)  log::log2file(fmt, __VA_ARGS__)
#define log_print_error(fmt, ...)  log::log2file(fmt, __VA_ARGS__)

#elif defined NDK_LOG
#define log_print_verbose(fmt, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, fmt, __VA_ARGS__)
#define log_print_debug(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, __VA_ARGS__)
#define log_print_info(fmt, ...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, __VA_ARGS__)
#define log_print_warn(fmt, ...) __android_log_print(ANDROID_LOG_WARN, TAG, fmt, __VA_ARGS__)
#define log_print_error(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, __VA_ARGS__)
#else
#define log_print_verbose(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#define log_print_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define log_print_info(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define log_print_warn(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#define log_print_error(fmt, ...) printf(fmt,  ##__VA_ARGS__)
#endif

#define logv(fmt, ...) log_print_verbose(fmt, ##__VA_ARGS__)
#define logd(fmt, ...) log_print_debug(fmt, ##__VA_ARGS__)
#define logi(fmt, ...) log_print_info(fmt, ##__VA_ARGS__)
#define logw(fmt, ...) log_print_warn(fmt, ##__VA_ARGS__)
#define loge(fmt, ...) log_print_error(fmt, ##__VA_ARGS__)