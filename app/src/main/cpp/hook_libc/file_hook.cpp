#include <mutex>
#include <vector>
#include <map>
#include <atomic>

#include "../utils/log.h"
#include "../utils/utils.h"
#include "file_hook.h"

using std::vector;
using std::mutex;
using std::atomic;
using std::map;
using std::pair;

#define LIBNAME "libc.so"

map<string, libc_file_func_list> elf_func;
map<string, libc_file_func_list> inline_func;
libc_file_func_list file_func_ptr;

mutex lock_open;
atomic<int> map_hook_index(0);

using map_file_list = map<int, shared_ptr<inside_hook_params>>;
using pair_file_list = pair<int, shared_ptr<inside_hook_params>>;
map_file_list hook_file_list;

//unsalf
bool check_hook_info(const hook_params &info) {
    for (const auto &item : hook_file_list) {
        if (item.second->reg_path == info.reg_path) {
            return false;
        }
    }
    return true;
}

base_hook_handle *check_and_create_handle(const char *FileName) {
    base_hook_handle *phandle = nullptr;

    lock_open.lock();
    for (const auto &item : hook_file_list) {
        if (regexec(&item.second->reg, FileName, 0, nullptr, 0) == 0) {
            phandle = new file_hook(item.second->ffactory->create());
            phandle->info.file_instance_id = item.second->open_index->fetch_add(1);
            phandle->info.path = FileName;
            break;
        }
    }
    lock_open.unlock();
    return phandle;
}

int register_file_hook(const hook_params &info) {
    auto_lock lock(&lock_open);

    if (!check_hook_info(info)) {
        return false;
    }
    shared_ptr<inside_hook_params> iinfo(new inside_hook_params(info));
    int mhandle = map_hook_index.fetch_add(1);
    hook_file_list.insert(pair_file_list(mhandle, iinfo));
    return mhandle;
}

void unregister_file_hook(int mhandle) {
    auto_lock lock(&lock_open);
    const auto &item_open_list = hook_file_list.find(mhandle);
    if (item_open_list != hook_file_list.end()) {
        hook_file_list.erase(item_open_list);
        return;
    }
}

FILE *hook_fopen(char const *FileName, char const *mode) {
    logd("fopening %s", FileName);
//    return 0;
    auto *phandle = dynamic_cast<base_file_interface *>(check_and_create_handle(FileName));
    if (phandle != nullptr) {
        FILE *fp = phandle->fopen(FileName, mode);
        if (fp != nullptr) {
            insert_hook_handle(fp, phandle);
        }
        return fp;
    }
    return file_func_ptr.phook_fopen(FileName, mode);
}

int hook_fclose(FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        p->fclose(fp);
        remove_hook_handle(fp);
        delete p;
        return 0;
    }
    return file_func_ptr.phook_fclose(fp);
}

char *hook_fgets(char *Buffer, int MaxCount, FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->fgets(Buffer, MaxCount, fp);
    }
    return file_func_ptr.phook_fgets(Buffer, MaxCount, fp);
}

size_t hook_fwrite(const void *_buf, size_t size, size_t count, FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->fwrite(_buf, size, count, fp);
    }
    return file_func_ptr.phook_fwrite(_buf, size, count, fp);
}

int hook_fputs(const char *s, FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->fputs(s, fp);
    }
    return file_func_ptr.phook_fputs(s, fp);
}

size_t hook_fread(void *_buf, size_t size, size_t count, FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->fread(_buf, size, count, fp);
    }
    return file_func_ptr.phook_fread(_buf, size, count, fp);
}

int hook_fseek(FILE *fp, long offset, int whence) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->fseek(fp, offset, whence);
    }
    return file_func_ptr.phook_fseek(fp, offset, whence);
}

long hook_ftell(FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->ftell(fp);
    }
    return file_func_ptr.phook_ftell(fp);
}

void hook_rewind(FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        return p->rewind(fp);
    }
    return file_func_ptr.phook_rewind(fp);
}

FILE *hook_popen(const char *_command, const char *mode) {
    logd("popening %s", _command);
    auto *phandle = dynamic_cast<base_file_interface *>(check_and_create_handle(_command));
    if (phandle != nullptr) {
        FILE *fp = phandle->popen(_command, mode);
        if (fp) {
            insert_hook_handle(fp, phandle);
        }
        return fp;
    }
//    return nullptr;
    return file_func_ptr.phook_fopen(_command, mode);
}

int hook_pclose(FILE *fp) {
    auto p = dynamic_cast<base_file_interface *>(get_hook_handle(fp));
    if (p) {
        p->pclose(fp);
        remove_hook_handle(fp);
        delete p;
        return 0;
    }
    return file_func_ptr.phook_fclose(fp);
}

int hook_open(const char *_path, int oflag, ...) {
    mode_t mode;
    va_list args;

    va_start(args, oflag);
    if (oflag & O_CREAT) {
        mode = va_arg(args, mode_t);
    } else {
        mode = 0;
    }
    va_end(args);
    logd("opening %s", _path);
//    return -1;
    auto *phandle = dynamic_cast<base_device_file_interface *>(check_and_create_handle(_path));
    if (phandle != nullptr) {
        int fd = phandle->open(_path, oflag, mode);
        if (fd != -1) {
            insert_hook_handle(fd, phandle);
        }
        return fd;
    }

    if (oflag & O_CREAT) {
        return file_func_ptr.phook_open(_path, oflag, mode);
    } else {
        return file_func_ptr.phook_open(_path, oflag);
    }
}

ssize_t hook_read(int fd, void *_buf, size_t _nbytes) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        return p->read(fd, _buf, _nbytes);
    }
    return file_func_ptr.phook_read(fd, _buf, _nbytes);
}

ssize_t hook_pread(int fd, void *_buf, size_t count, off_t offset) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        return p->pread(fd, _buf, count, offset);
    }
    return file_func_ptr.phook_pread(fd, _buf, count, offset);
}

ssize_t hook_write(int fd, const void *_buf, size_t count) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        return p->write(fd, _buf, count);
    }
    return file_func_ptr.phook_write(fd, _buf, count);
}

ssize_t hook_pwrite(int fd, const void *_buf, size_t count, off_t offset) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        return p->pwrite(fd, _buf, count, offset);
    }
    return file_func_ptr.phook_pwrite(fd, _buf, count, offset);
}

int hook_fstat(int fd, struct stat *buf) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        return p->fstat(fd, buf);
    }
    return file_func_ptr.phook_fstat(fd, buf);
}

int hook_close(int fd) {
    auto p = dynamic_cast<base_device_file_interface *>(get_hook_handle(fd));
    if (p) {
        p->close(fd);
        remove_hook_handle(fd);
        delete p;
        return 0;
    }
    return file_func_ptr.phook_close(fd);
}

int hook_lstat(const char *file, struct stat *buf) {
    logd("lstat %s", file);
    auto *phandle = dynamic_cast<base_other_file_opt_interface *>(check_and_create_handle(file));
    if (phandle != nullptr) {
        int ret = phandle->lstat(file, buf);
        delete phandle;
        return ret;
    }
    return file_func_ptr.phook_lstat(file, buf);
}

int hook_stat(const char *pathname, struct stat *buf) {
    logd("stat %s", pathname);
    auto *phandle = dynamic_cast<base_other_file_opt_interface *>(check_and_create_handle(
            pathname));
    if (phandle != nullptr) {
        int ret = phandle->stat(pathname, buf);
        delete phandle;
        return ret;
    }
    return file_func_ptr.phook_stat(pathname, buf);
}

int hook_rename(const char *old_path, const char *new_path) {
    logd("rename %s -> %s", old_path, new_path);
    auto *phandle = dynamic_cast<base_other_file_opt_interface *>(check_and_create_handle(
            old_path));
    if (phandle != nullptr) {
        int ret = phandle->rename(old_path, new_path);
        delete phandle;
        return ret;
    }
    return file_func_ptr.phook_rename(old_path, new_path);
}

int hook_access(const char *pathname, int mode) {
    logd("access %s", pathname);
    auto *phandle = dynamic_cast<base_other_file_opt_interface *>(check_and_create_handle(
            pathname));
    if (phandle != nullptr) {
        int ret = phandle->access(pathname, mode);
        delete phandle;
        return ret;
    }
    return file_func_ptr.phook_access(pathname, mode);
}


void init_file_hook() {
//    ELF_HOOK_DEBUG(1);
//    string module_name = ".*libsmsdk\\.so$";
    string module_name = "libsmsdk.so";

    ELF_HOOK_DECL(module_name, file_func_ptr, open);
    ELF_HOOK_DECL(module_name, file_func_ptr, read);
    ELF_HOOK_DECL(module_name, file_func_ptr, pread);
    ELF_HOOK_DECL(module_name, file_func_ptr, write);
    ELF_HOOK_DECL(module_name, file_func_ptr, pwrite);
    ELF_HOOK_DECL(module_name, file_func_ptr, fstat);
    ELF_HOOK_DECL(module_name, file_func_ptr, close);

    ELF_HOOK_DECL(module_name, file_func_ptr, popen);
    ELF_HOOK_DECL(module_name, file_func_ptr, pclose);
    ELF_HOOK_DECL(module_name, file_func_ptr, fopen);
    ELF_HOOK_DECL(module_name, file_func_ptr, fclose);
    ELF_HOOK_DECL(module_name, file_func_ptr, fread);
    ELF_HOOK_DECL(module_name, file_func_ptr, fwrite);
    ELF_HOOK_DECL(module_name, file_func_ptr, fgets);
    ELF_HOOK_DECL(module_name, file_func_ptr, fputs);
    ELF_HOOK_DECL(module_name, file_func_ptr, fseek);
    ELF_HOOK_DECL(module_name, file_func_ptr, ftell);
    ELF_HOOK_DECL(module_name, file_func_ptr, rewind);

    ELF_HOOK_DECL(module_name, file_func_ptr, lstat);
    ELF_HOOK_DECL(module_name, file_func_ptr, stat);

    ELF_HOOK_DECL(module_name, file_func_ptr, rename);
    ELF_HOOK_DECL(module_name, file_func_ptr, access);


    ELF_HOOK_REFRESH();

    if (file_func_ptr.phook_open == nullptr) {
        loge("open addr %p", file_func_ptr.phook_fopen);
    }
}











//bool elf_hook_module(string module_name) {
//    ELF_HOOK_DEBUG(1);
////    char *reg_module = ".*libsmsdk\\.so$";
//    libc_file_func_list func;
//
//    ELF_HOOK_DECL(module_name, func, open);
//    ELF_HOOK_DECL(module_name, func, read);
//    ELF_HOOK_DECL(module_name, func, pread);
//    ELF_HOOK_DECL(module_name, func, write);
//    ELF_HOOK_DECL(module_name, func, pwrite);
//    ELF_HOOK_DECL(module_name, func, fstat);
//    ELF_HOOK_DECL(module_name, func, close);
//
//    ELF_HOOK_DECL(module_name, func, popen);
//    ELF_HOOK_DECL(module_name, func, pclose);
//    ELF_HOOK_DECL(module_name, func, fopen);
//    ELF_HOOK_DECL(module_name, func, fclose);
//    ELF_HOOK_DECL(module_name, func, fread);
//    ELF_HOOK_DECL(module_name, func, fwrite);
//    ELF_HOOK_DECL(module_name, func, fgets);
//    ELF_HOOK_DECL(module_name, func, fputs);
//    ELF_HOOK_DECL(module_name, func, fseek);
//    ELF_HOOK_DECL(module_name, func, ftell);
//    ELF_HOOK_DECL(module_name, func, rewind);
//
//    ELF_HOOK_DECL(module_name, func, opendir);
//    ELF_HOOK_DECL(module_name, func, readdir);
//    ELF_HOOK_DECL(module_name, func, closedir);
//
//    ELF_HOOK_DECL(module_name, func, lstat);
//    ELF_HOOK_DECL(module_name, func, stat);
//
//    ELF_HOOK_DECL(module_name, func, rename);
//    ELF_HOOK_DECL(module_name, func, access);
//
//    ELF_HOOK_REFRESH();
//
//    if (phook_open == nullptr) {
//        loge("open addr %p", phook_fopen);
//    }
//
//    elf_func.insert(pair<string, libc_file_func_list>(module_name, func));
//    return true;
//}

//    void *handle = INLINE_HOOK_OPEN(LIBNAME);
//
//    INLINE_HOOK_DECL(handle, open);
//    INLINE_HOOK_DECL(handle, read);
//    INLINE_HOOK_DECL(handle, pread);
//    INLINE_HOOK_DECL(handle, write);
//    INLINE_HOOK_DECL(handle, pwrite);
//    INLINE_HOOK_DECL(handle, fstat);
//    INLINE_HOOK_DECL(handle, close);
//
//    INLINE_HOOK_DECL(handle, popen);
//    INLINE_HOOK_DECL(handle, pclose);
//    INLINE_HOOK_DECL(handle, fopen);
//    INLINE_HOOK_DECL(handle, fclose);
//    INLINE_HOOK_DECL(handle, fread);
//    INLINE_HOOK_DECL(handle, fwrite);
//    INLINE_HOOK_DECL(handle, fgets);
//    INLINE_HOOK_DECL(handle, fputs);
//    INLINE_HOOK_DECL(handle, fseek);
//    INLINE_HOOK_DECL(handle, ftell);
//    INLINE_HOOK_DECL(handle, rewind);
//
//    INLINE_HOOK_DECL(handle, opendir);
//    INLINE_HOOK_DECL(handle, readdir);
//    INLINE_HOOK_DECL(handle, closedir);
//
//    INLINE_HOOK_DECL(handle, lstat);
//    INLINE_HOOK_DECL(handle, stat);
//
//    INLINE_HOOK_DECL(handle, rename);
//    INLINE_HOOK_DECL(handle, access);
//
//    INLINE_HOOK_CLOSE(handle);