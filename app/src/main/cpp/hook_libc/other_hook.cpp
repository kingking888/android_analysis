#include <unistd.h>
#include <sys/mman.h>
#include <ctime>
#include <sys/system_properties.h>

#include "../utils/log.h"
#include "base_hook.h"
#include "other_hook.h"

#define LIBNAME "libc.so"

libc_other_func_list other_func_ptr;

tm *hook_localtime_r(const time_t *timep, tm *result) {
    return other_func_ptr.phook_localtime_r(timep, result);
//    localtime_r(timep, result);
//    result->tm_sec = 0;   // seconds after the minute - [0, 60] including leap second
//    result->tm_min = 0;   // minutes after the hour - [0, 59]
//    result->tm_hour = 0;  // hours since midnight - [0, 23]
//    result->tm_mday = 1;  // day of the month - [1, 31]
//    result->tm_mon = 1;   // months since January - [0, 11]
//    result->tm_year = 0;  // years since 1900
//    result->tm_wday = 0;  // days since Sunday - [0, 6]
//    result->tm_yday = 0;  // days since January 1 - [0, 365]
//    result->tm_isdst = 1; // daylight savings time flag
//    return result;
}

void *hook_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    return other_func_ptr.phook_mmap(addr, len, prot, flags, fd, offset);
}

int hook_socket(int socket_family, int socket_type, int protocol) {
    return other_func_ptr.phook_socket(socket_family, socket_type, protocol);
}

//int hook_ioctl(int fd, int request, ...) {
//    return phook_ioctl(fd, request, ...);
//}


/*
__system_property_get
__system_property_set
__system_property_find
__system_property_read
__system_property_find_nth
__system_property_foreach
*/

int hook__system_property_get(const char *name, char *value) {
    //数据已经存储在内存中__system_property_area__ 等待读取完返回
    //const prop_info* pi = __system_property_find(name);
    //return __system_property_read(pi, 0, value);
    int ret = other_func_ptr.phook__system_property_get(name, value);
    logi("system_property_get %s %s", name, value);
    return ret;
}

void init_other_hook() {
    string module_name = "libsmsdk.so";

    ELF_HOOK_DECL(module_name, other_func_ptr, localtime_r);
    ELF_HOOK_DECL(module_name, other_func_ptr, mmap);
    ELF_HOOK_DECL(module_name, other_func_ptr, socket);
    ELF_HOOK_DECL(module_name, other_func_ptr, _system_property_get);
    ELF_HOOK_REFRESH();

    if (other_func_ptr.phook_localtime_r == nullptr) {
        loge("open addr %p", other_func_ptr.phook_localtime_r);
    }
}
