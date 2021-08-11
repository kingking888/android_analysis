#pragma once

#include <dirent.h>
#include <string>


int require_fd();

class file_hook;

namespace file_open_type {
    const unsigned int open = 0b1;
    const unsigned int popen = 0b10;
    const unsigned int fopen = 0b100;
};
enum class file_stat_type {
    fstat,
    lstat,
    stat
};
union open_args {
    struct {
        char const *path;
        int flag;
        mode_t mode;
    } _open_args;
    struct {
        char const *path;
        char const *mode;
    } _fpopen_args;
};

class file_filter {
    friend file_hook;
protected:
    //返回true 则不继续执行
    virtual bool on_set_pos(int *ret, int offset) = 0;

    //返回-1则无实现
    virtual int on_get_pos() = 0;

    virtual int on_get_file_size() = 0;

//    virtual int on_get_file_all_size() { return -1; };

public:
    virtual bool before_open(int type, void **ret, open_args args, open_args *new_args) = 0;

    virtual int on_close() = 0;

    virtual size_t before_read(void *buf, size_t read_size, bool is_fgets) = 0;

    virtual size_t after_read(void *buf, size_t read_size, size_t readed_size, bool is_fgets) = 0;

    virtual size_t before_write(void *buf, size_t write_size) = 0;

    //返回true 则不继续执行
    virtual bool before_stat(int *ret, file_stat_type stat_type, struct stat *buf) = 0;

    virtual int after_stat(int ret, struct stat *buf) = 0;

    //返回true 则不继续执行
    virtual bool before_rename(int *ret, const char *new_path) = 0;

    //返回true 则不继续执行
    virtual bool before_access(int *ret, int mode) = 0;
};

class common_file_filter : public file_filter {
    friend file_hook;
protected:
    //返回true 则不继续执行
    virtual bool on_set_pos(int *ret, int offset) { return false; }

    //返回-1则无实现
    virtual int on_get_pos() { return -1; }

    virtual int on_get_file_size() { return -1; }

public:
    virtual void *before_open(int type, open_args args, open_args *new_args) {
        return nullptr;
    }

    virtual int on_close() {

    }

    virtual size_t before_read(void *buf, size_t read_size, bool is_fgets) {
        return 0;
    }

    virtual size_t after_read(void *buf, size_t read_size, size_t readed_size, bool is_fgets) {
        return readed_size;
    }

    virtual size_t before_write(void *buf, size_t write_size) {
        return 0;
    }

    //返回true 则不继续执行
    virtual bool before_stat(int *ret, file_stat_type stat_type, struct stat *buf) {
        return false;
    }

    virtual int after_stat(int ret, struct stat *buf) {
        return ret;
    }

    //返回true 则不继续执行
    virtual bool before_rename(int *ret, const char *new_path) {
        return false;
    }

    //返回true 则不继续执行
    virtual bool before_access(int *ret, int mode) {
        return false;
    }
};

class none_file_filter : public common_file_filter {
    virtual bool before_open(int type, void **ret, open_args args, open_args *new_args) {
        if (type & file_open_type::open) {
            *ret = (void *) -1;
        } else {
            *ret = (void *) 0;
        }
        return true;
    }

    virtual bool before_stat(int *ret, file_stat_type stat_type, struct stat *buf) {
        *ret = -1;
        return true;
    }

    virtual bool before_rename(int *ret, const char *new_path) {
        *ret = -1;
        return true;
    }

    virtual bool before_access(int *ret, int mode) {
        *ret = -1;
        return true;
    }
};

class base_dir_filter {
public:
    virtual DIR *opendir(const char *name) = 0;

    virtual dirent *readdir(DIR *dir) = 0;

    virtual int closedir(DIR *dir) = 0;
};



