#pragma once
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <string>
#include <string.h>

#include "base_hook.h"
#include "filter.h"

using std::string;
using std::shared_ptr;

class memory_file : public file_filter {
    string path;
    int alllen;
    int len;
    int off;
    shared_ptr<char> share_data;
    void *file_handle;
private:
    size_t read(char *buffer, size_t rsize, bool update = true) {
        if (this->off >= this->len)
            return 0;
        int rlen = this->len - off > rsize ? rsize : this->len - off;
        memcpy(buffer, this->share_data.get() + off, rlen);
        if (update) {
            this->off += rlen;
        }
        return rlen;
    }

    size_t write(const char *buffer, size_t wsize, bool update = true) {
        int rlen = this->alllen - this->off > wsize ? wsize : this->alllen - this->off;
        if (rlen <= 0) {
            this->alllen = this->alllen - rlen + this->alllen * 0.5;
            shared_ptr<char> nbuf(new char[this->alllen]());
            memcpy(nbuf.get(), this->share_data.get(), this->len);
            this->share_data = nbuf;
            rlen = this->alllen - this->off > wsize ? wsize : this->alllen - this->off;
        }

        memcpy(&this->share_data.get()[this->off], buffer, rlen);
        if (update) {
            this->off += rlen;
        }
        if (this->off >= this->len)
            this->len = this->off;
        return rlen;
    }

    virtual size_t fgets(char *buffer, int max_count) {
        if (this->off >= this->len)
            return 0;
        int rlen = 0;
        while (true) {
            if (this->share_data.get()[this->off + rlen] == '\n')
                break;
            rlen++;
        }
        rlen = rlen > max_count ? max_count : rlen;
        read(buffer, rlen, this->off);
        return rlen;
    }

public:
    string get_path() { return this->path; }

    memory_file(const char *data, int len) {
        this->off = 0;
        this->len = len;
        this->alllen = len * 1.5;
        this->share_data = shared_ptr<char>(new char[this->alllen]());
        memcpy(this->share_data.get(), data, len);
    }

    memory_file(char *&&data, int len) {
        this->off = 0;
        this->len = len;
        this->alllen = len * 1.5;
        this->share_data = shared_ptr<char>(data);
    }

    memory_file() {
        this->off = 0;
        this->len = 0;
        this->alllen = 1024 * 2;
        this->share_data = shared_ptr<char>(new char[this->alllen]());
    }

protected:
    virtual bool on_set_pos(int *ret, int offset) {
        off = offset;
        *ret = 0;
        return true;
    }

    virtual int on_get_pos() {
        return off;
    }

    virtual int on_get_file_size() {
        return alllen;
    }

//    virtual int on_get_file_all_size() { return -1; };
public:
    virtual bool before_open(int type, void **ret, open_args args, open_args *new_args) {
        if (type & file_open_type::open) {
            file_handle = (void *) require_fd();
            *ret = file_handle;
        } else {
            file_handle = this;
            *ret = this;
        }
        return true;
    }

    virtual int on_close() {
        return 0;
    }

    virtual size_t before_read(void *buf, size_t read_size, bool is_fgets) {
        if (is_fgets) {
            return this->fgets((char *) buf, read_size);
        } else {
            return this->read((char *) buf, read_size);
        }
    }

    virtual size_t before_write(void *buf, size_t write_size) {
        return this->write((char *) buf, write_size);
    }


    virtual bool before_stat(int *ret, file_stat_type stat_type, struct stat *buf) {
        *ret = 0;
        memset(buf, 0, sizeof(struct stat));
        return true;
    }

    virtual bool before_rename(int *ret, const char *new_path) {
        *ret = 0;
        path = new_path;
        return true;
    }

    virtual bool before_access(int *ret, int mode) {
        *ret = 0;
        return true;
    }

    virtual size_t after_read(void *buf, size_t read_size, size_t readed_size, bool is_fgets) {
        return 0;
    }

    virtual int after_stat(int ret, struct stat *buf) {
        return -1;
    }

};
