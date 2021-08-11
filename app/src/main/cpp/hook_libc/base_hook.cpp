#include<sys/types.h>
#include<unistd.h>
#include <map>
#include <atomic>
#include <mutex>
#include <vector>
#include <unwind.h>

#include "base_hook.h"

using std::map;
using std::atomic;
using std::pair;
using std::mutex;
using std::vector;

struct module_info {
    void *base;
    void *end;
    string name;
};

int g_pid;
atomic<int> fd_index;
mutex lock_target;
mutex lock_handle;

vector<module_info> target_module;
map<void *, base_hook_handle *> g_hook;


bool hook_global_init() {
//    FILE *fp = popen("su -c cat /proc/sys/fs/file-max", "r");
//    char buf[256] = {0};
//    fgets(buf, 256, fp);
//    pclose(fp);
//    fd_index = atoi(buf);
    fd_index = 65555;
    g_pid = getpid();
    return true;
}

bool _is_hook_handle(void *handle) {
    lock_handle.lock();
    bool ret = g_hook.find(handle) != g_hook.end();
    lock_handle.unlock();
    return ret;
}

void _remove_hook_handle(void *handle) {
    lock_handle.lock();
    g_hook.erase(g_hook.find(handle));
    lock_handle.unlock();
}

void _insert_hook_handle(void *handle, base_hook_handle *phook) {
    lock_handle.lock();
    g_hook.insert(pair<void *, base_hook_handle *>(handle, phook));
    lock_handle.unlock();
}

base_hook_handle *_get_hook_handle(void *handle) {
    base_hook_handle *ret = nullptr;
    lock_handle.lock();
    auto item = g_hook.find(handle);
    if (item != g_hook.end()) {
        ret = item->second;
    }
    lock_handle.unlock();
    return ret;
}

int get_pid() {
    return g_pid;
}

int require_fd() {
    return fd_index.fetch_add(1);
}

bool add_target_module(const string &name) {
//    module_info info;
//    info.name = name;
//    void *mhandle = fake_dlopen(name.c_str(), RTLD_NOW);
//    if (mhandle == nullptr) {
//        return false;
//    }
//    fake_get_module_info(mhandle, &info.base, &info.end);
//    fake_dlclose(mhandle);
//
//    lock_target.lock();
//    target_module.push_back(info);
//    lock_target.unlock();
    return true;
}

bool check_call_from() {
//    _Unwind_Backtrace();
}


void remove_target_module(const string &name) {
    lock_target.lock();
    for (auto item = target_module.begin(); item != target_module.end(); item++) {
        if (item->name == name) {
            target_module.erase(item);
            break;
        }
    }
    lock_target.unlock();
}
