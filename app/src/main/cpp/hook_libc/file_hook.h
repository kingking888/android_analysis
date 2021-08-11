#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <fcntl.h>

#include "base_hook.h"


struct libc_file_func_list {

    int (*phook_open)(const char *path, int flag, ...) = nullptr;

    ssize_t (*phook_read)(int fd, void *buf, size_t nbytes) = nullptr;

    ssize_t (*phook_pread)(int fd, void *buf, size_t count, off_t offset) = nullptr;

    ssize_t (*phook_write)(int fd, const void *buf, size_t count) = nullptr;

    ssize_t (*phook_pwrite)(int fd, const void *buf, size_t count, off_t offset) = nullptr;

    int (*phook_fstat)(int fd, struct stat *buf) = nullptr;

    int (*phook_close)(int fd) = nullptr;

//off_t lseek(int fd, off_t offset, int whence) = nullptr;
    FILE *(*phook_popen)(const char *command, const char *mode) = nullptr;

    int (*phook_pclose)(FILE *fp) = nullptr;

    FILE *(*phook_fopen)(char const *FileName, const char *mode) = nullptr;

    int (*phook_fclose)(FILE *Stream) = nullptr;

    size_t (*phook_fread)(void *buf, size_t size, size_t count, FILE *fp) = nullptr;

    size_t (*phook_fwrite)(const void *buf, size_t size, size_t count, FILE *fp) = nullptr;

    char *(*phook_fgets)(char *buffer, int max_count, FILE *fp) = nullptr;

    int (*phook_fputs)(const char *s, FILE *fp) = nullptr;

    int (*phook_fseek)(FILE *fp, long offset, int whence) = nullptr;

    long (*phook_ftell)(FILE *fp) = nullptr;

    void (*phook_rewind)(FILE *fp) = nullptr;

    int (*phook_lstat)(const char *file, struct stat *buf) = nullptr;

    int (*phook_stat)(const char *pathname, struct stat *buf) = nullptr;

    int (*phook_access)(const char *pathname, int mode) = nullptr;

    int (*phook_rename)(const char *old_path, const char *new_path) = nullptr;

    DIR *(*phook_opendir)(const char *name) = nullptr;

    dirent *(*phook_readdir)(DIR *dir) = nullptr;

    int (*phook_closedir)(DIR *dir) = nullptr;
};

extern libc_file_func_list file_func_ptr;

class base_device_file_interface : virtual public base_hook_handle {
public:
    virtual int open(const char *path, int flag, mode_t mode) = 0;

    virtual ssize_t read(int fd, void *buf, size_t nbytes) = 0;

    virtual ssize_t pread(int fd, void *buf, size_t count, off_t offset) = 0;

    virtual ssize_t write(int fd, const void *buf, size_t count) = 0;

    virtual ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) = 0;

    virtual int fstat(int fd, struct stat *buf) = 0;

    virtual int close(int fd) = 0;
};

class base_file_interface : virtual public base_hook_handle {
public:
    virtual FILE *fopen(char const *FileName, const char *mode) = 0;

    virtual FILE *popen(const char *command, const char *mode) = 0;

    virtual char *fgets(char *Buffer, int max_count, FILE *fp) = 0;

    virtual size_t fread(void *buf, size_t size, size_t count, FILE *fp) = 0;

    virtual int fputs(const char *s, FILE *fp) = 0;

    virtual size_t fwrite(const void *buf, size_t size, size_t count, FILE *fp) = 0;

    virtual int fseek(FILE *fp, long offset, int whence) = 0;

    virtual long ftell(FILE *fp) = 0;

    virtual void rewind(FILE *fp) = 0;

    virtual int pclose(FILE *fp) = 0;

    virtual int fclose(FILE *fp) = 0;
};

class base_other_file_opt_interface : virtual public base_hook_handle {
public:
    virtual int lstat(const char *file, struct stat *buf) = 0;

    virtual int stat(const char *pathname, struct stat *buf) = 0;

    virtual int access(const char *pathname, int mode) = 0;

    virtual int rename(const char *old_path, const char *new_path) = 0;
};

class base_dir_opt : virtual public base_hook_handle {
public:
    virtual DIR *opendir(const char *name) = 0;

    virtual dirent *readdir(DIR *dir) = 0;

    virtual int closedir(DIR *dir) = 0;
};

class file_hook :
        public base_file_interface,
        public base_device_file_interface,
        public base_other_file_opt_interface,
        public base_dir_opt {
private:
    int open_type;
    file_filter *call_back;
public:
    file_hook(file_filter *cb) {
        call_back = cb;
    }

    ~file_hook() {
        delete call_back;
    }

public:
    virtual FILE *fopen(char const *path, char const *mode) {
        logi("hook fopen file %s!", path);
        open_args args;
        args._fpopen_args.path = path;
        args._fpopen_args.mode = mode;
        char buf_path[256] = {0};
        char buf_mode[256] = {0};
        open_args new_args;
        new_args._fpopen_args.path = buf_path;
        new_args._fpopen_args.mode = buf_mode;
        void *handle;
        if (call_back->before_open(file_open_type::fopen, &handle, args, &new_args)) {
            return (FILE *) handle;
        }
        if (buf_path[0] != 0) {
            return file_func_ptr.phook_fopen(buf_path, buf_mode);
        } else {
            return file_func_ptr.phook_fopen(path, mode);
        }
    }

    virtual FILE *popen(const char *command, const char *mode) {
        logi("hook popen file %s!", command);
        open_args args;
        args._fpopen_args.path = command;
        args._fpopen_args.mode = mode;
        char buf_path[256] = {0};
        char buf_mode[256] = {0};
        open_args new_args;
        new_args._fpopen_args.path = buf_path;
        new_args._fpopen_args.mode = buf_mode;
        void *handle;
        if (call_back->before_open(file_open_type::popen, &handle, args, &new_args)) {
            return (FILE *) handle;
        }
        if (buf_path[0] != 0) {
            return file_func_ptr.phook_popen(buf_path, buf_mode);
        } else {
            return file_func_ptr.phook_popen(command, mode);
        }
    }

    virtual char *fgets(char *buffer, int max_count, FILE *fp) {
        if (call_back->before_read(buffer, max_count, true) == 0) {
            file_func_ptr.phook_fgets(buffer, max_count, fp);
        } else {
            return buffer;
        }
        if (call_back->after_read(buffer, max_count, strlen(buffer), true) == 0) {
            return nullptr;
        } else {
            return buffer;
        }
    }

    virtual size_t fread(void *buffer, size_t size, size_t count, FILE *fp) {
        int rd_size = size * count;
        int rded_size = call_back->before_read(buffer, rd_size, false);
        if (rded_size == 0) {
            rded_size = file_func_ptr.phook_fread(buffer, size, count, fp);
        } else {
            return rded_size;
        }

        return call_back->after_read(buffer, rd_size, rded_size, false);
    }

    virtual int fputs(const char *buffer, FILE *fp) {
        size_t len = strlen(buffer);
        int wted_size = call_back->before_write((void *) buffer, len);
        if (wted_size != 0) {
            return wted_size;
        }
        return file_func_ptr.phook_fwrite(buffer, len, 1, fp);
    }

    virtual size_t fwrite(const void *buffer, size_t size, size_t count, FILE *fp) {
        size_t wt_size = size * count;
        size_t wted_size = call_back->before_write((void *) buffer, wt_size);
        if (wted_size != 0) {
            return wted_size;
        }
        return file_func_ptr.phook_fwrite(buffer, wt_size, 1, fp);
    }

    virtual int fseek(FILE *fp, long offset, int whence) {
        int off = 0;
        if (call_back->on_get_pos() == -1 ||
            call_back->on_get_file_size() == -1) {
            goto after;
        }
        switch (whence) {
            case SEEK_SET:
                off = offset;
                break;
            case SEEK_CUR:
                off = call_back->on_get_pos() + offset;
                break;
            case SEEK_END:
                off = call_back->on_get_file_size() - 1 + offset;
                break;
        }
        int ret;
        if (call_back->on_set_pos(&ret, off)) {
            return ret;
        }
        after:
        return file_func_ptr.phook_fseek(fp, offset, whence);
    }

    virtual long ftell(FILE *fp) {
        if (call_back->on_get_pos() == -1) {
            goto after;
        }
        int rpos;
        rpos = call_back->on_get_pos();
        if (rpos != -1) {
            return rpos;
        }
        after:
        return file_func_ptr.phook_ftell(fp);
    }

    virtual void rewind(FILE *fp) {
        int rpos;
        if (call_back->on_set_pos(&rpos, 0)) {
            return;
        }
        file_func_ptr.phook_rewind(fp);
    }

    virtual int pclose(FILE *fp) {
        call_back->on_close();
        return file_func_ptr.phook_pclose(fp);
    }

    virtual int fclose(FILE *fp) {
        call_back->on_close();
        return file_func_ptr.phook_fclose(fp);
    }

    //-----------------------------------------
    virtual int open(const char *path, int flag, mode_t mode) {
        logi("hook open file %s!", path);
        open_args args;
        args._open_args.path = path;
        args._open_args.flag = flag;
        args._open_args.mode = mode;

        char buf_path[256] = {0};
        open_args new_args;
        new_args._open_args.path = buf_path;
        new_args._open_args.flag = 0;
        new_args._open_args.mode = 0;
        void *handle;
        if (call_back->before_open(file_open_type::open, &handle, args, &new_args)) {
#if defined (__arm64__) || defined (__aarch64__)
            int aa = *((int *) handle);
            aa = *(((int *) handle) + 1);
            return *((int *) handle);
#else
            return (int) handle;
#endif
        }
        if (buf_path[0] != 0) {
            return file_func_ptr.phook_open(buf_path, new_args._open_args.flag,
                                            new_args._open_args.mode);
        } else {
            return file_func_ptr.phook_open(path, flag, mode);
        }
    }

    virtual ssize_t read(int fd, void *buffer, size_t nbytes) {
        size_t rded_size = call_back->before_read(buffer, nbytes, false);
        if (rded_size == 0) {
            rded_size = file_func_ptr.phook_read(fd, buffer, nbytes);
        } else {
            return rded_size;
        }

        return call_back->after_read(buffer, nbytes, rded_size, false);
    }

    struct auto_set_pos {
        file_filter *cb;
        int old_pos;

        auto_set_pos(file_filter *cb, int pos) {
            this->cb = cb;
            old_pos = this->cb->on_get_pos();
            int ret;
            this->cb->on_set_pos(&ret, pos);
        }

        ~auto_set_pos() {
            int ret;
            this->cb->on_set_pos(&ret, old_pos);
        }
    };

    virtual ssize_t pread(int fd, void *buffer, size_t count, off_t offset) {
        auto_set_pos asp(call_back, offset);
        return read(fd, buffer, count);
    }

    virtual ssize_t write(int fd, const void *buffer, size_t count) {
        size_t wted_size = call_back->before_write((void *) buffer, count);
        if (wted_size != 0) {
            return wted_size;
        }
        return file_func_ptr.phook_write(fd, buffer, count);
    }

    virtual ssize_t pwrite(int fd, const void *buffer, size_t count, off_t offset) {
        auto_set_pos asp(call_back, offset);
        return write(fd, buffer, count);
    }

    virtual int fstat(int fd, struct stat *buf) {
        int ret;
        if (call_back->before_stat(&ret, file_stat_type::fstat, buf)) {
            return ret;
        }
        return call_back->after_stat(file_func_ptr.phook_fstat(fd, buf), buf);
    }

    virtual int close(int fd) {
        call_back->on_close();
        return file_func_ptr.phook_close(fd);
    }

    //--------------------------------------
    virtual int lstat(const char *file, struct stat *buf) {
        logi("hook lstat file %s!", file);
        int ret;
        if (call_back->before_stat(&ret, file_stat_type::lstat, buf) == 0) {
            return ret;
        }
        return call_back->after_stat(file_func_ptr.phook_lstat(file, buf), buf);
    }

    virtual int stat(const char *pathname, struct stat *buf) {
        logi("hook stat file %s!", pathname);
        int ret;
        if (call_back->before_stat(&ret, file_stat_type::stat, buf) == 0) {
            return ret;
        }
        return call_back->after_stat(file_func_ptr.phook_stat(pathname, buf), buf);
    }

    virtual int rename(const char *old_path, const char *new_path) {
        logi("hook rename file %s -> %s!", old_path, new_path);
        int ret;
        if (call_back->before_rename(&ret, new_path) == 0) {
            return ret;
        }
        return file_func_ptr.phook_rename(old_path, new_path);
    }

    virtual int access(const char *pathname, int mode) {
        logi("hook access file %s!", pathname);
        int ret;
        if (call_back->before_access(&ret, mode) == 0) {
            return ret;
        }
        return file_func_ptr.phook_access(pathname, mode);
    }

    //----------------------------------------------
    virtual DIR *opendir(const char *name) {


    }

    virtual dirent *readdir(DIR *dir) {


    }

    virtual int closedir(DIR *dir) {


    }
};


