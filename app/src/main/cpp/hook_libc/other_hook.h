#pragma once

#include <ctime>

struct libc_other_func_list {
    tm *(*phook_localtime_r)(const time_t *timep, tm *result);

    void *(*phook_mmap)(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

    int (*phook_socket)(int socket_family, int socket_type, int protocol);

    int (*phook_ioctl)(int fd, int request, ...);

    int (*phook__system_property_get)(const char *name, char *value);
};


