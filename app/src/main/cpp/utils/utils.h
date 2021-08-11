#pragma once

#include <string>
#include <mutex>

using std::string;
using std::mutex;

struct auto_lock {
    mutex *lock;

    auto_lock(mutex *lock) {
        this->lock = lock;
        this->lock->lock();
    }

    ~auto_lock() {
        this->lock->unlock();
    }
};

string get_uuid();