#include "log.h"

char *log::buffer;
FILE *log::log_file;
mutex log::lock;

class log auto_init;