#include <fcntl.h>
#include <unistd.h>
#include "utils.h"

string get_uuid() {
    return "2aea3c86-e5e9-4930-a5d7-183dcaba007b";
    char uuid[37] = {0};
    int fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if (fd >= 0) {
        read(fd, uuid + 1, 36);
    }
    close(fd);
    return uuid;
}

