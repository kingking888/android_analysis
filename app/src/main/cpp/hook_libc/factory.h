#pragma once

#include <string>
#include "filter.h"

using std::string;

class file_filter;

class filter_factory;

struct hook_params {
    string reg_path;
//    bool has_pid = false;
    filter_factory *ffactory;
};

class filter_factory {
public:
    virtual file_filter *create() = 0;
};


class none_file_factory : public filter_factory {
public:
    virtual file_filter *create() {
        return new none_file_filter();
    }
};

using func_create_filter = file_filter *(*)();