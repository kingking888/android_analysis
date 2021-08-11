#pragma once

#include <dirent.h>
#include <string>
#include <vector>
#include <list>

#include "../utils/str_helper.h"

using std::string;
using std::vector;
using std::list;

/*	__DIRENT64_INO_T d_ino; \
		off64_t d_off; \
		unsigned short d_reclen; \
		unsigned char d_type; \
		char d_name[256]; \*/

struct dir_node {
    dirent info = {0};
    dir_node *father = nullptr;
    dir_node *child = nullptr;
    //下面顺序不能变
    dir_node *cnext = nullptr;
    dir_node *next = nullptr;
private:
    //dir_node* insert_next(dir_node* new_node) {
    //	dir_node** end = &next;
    //	while ((*end) != nullptr)
    //		end = &(*end)->next;
    //	*end = new_node;
    //	return new_node;
    //}

    void insert_child(dir_node *new_node) {
        dir_node **end = &child;
        while ((*end) != nullptr)
            end = &(*end)->next;
        *end = new_node;
        //cnext
        *(end - 1) - new_node;
    }

public:
    dir_node(string name, int type) {
        strcpy(info->d_name, name.c_str());
    }

    dir_node *insert_dir(string name) {
        dir_node *new_node = new dir_node;
        insert_child(new_node);
        return new_node;
    }

    dir_node *insert_file(string name) {
        dir_node *new_node = new dir_node;
        insert_child(new_node);
        return new_node;
    }

    void insert_dir(const vector <string> &name) {
        for (int i = 0; i < name.size(); i++) {
            dir_node *new_node = new dir_node;
            insert_child(new_node);
        }
    }

    void insert_file(const vector <string> &name) {
        for (int i = 0; i < name.size(); i++) {
            dir_node *new_node = new dir_node;
            insert_child(new_node);
        }
    }

    dir_node *father() {
        return father;
    }

    dirent *get_info() {
        return &info;
    }
};

class memory_dir {
    dir_node *root = nullptr;

    dir_node *create_root(string path = "") {
        root = new dir_node;
    }

    dir_node *root() {
        return root;
    }

    dir_node *is_contain(string path) {
        dirent *root_info = root->get_info();
        int p = path.find(root_info->d_name);
        if (p == string::npos)
            return nullptr;
        auto slist = string_split(path.substr(p + strlen(root_info->d_name)), "/");

        dir_node *p = root->child;
        for (const auto &item : slist) {
            int flag = false;
            while (p != nullptr) {
                if (item == p.get_info()->d_name) {
                    p = p->child;
                    flag = true;
                    break;
                }
                p = p->cnext;
            }
            if (!flag) {
                return nullptr;
            }
        }
        return p;
    }

    void finish() {
        list < dir_node * > plist;
        dir_node *p = root;

        plist.push_back(nullptr);
        while (p != nullptr) {
            if (p->next) {
                plist.push_front(p->next);
            }

            if (p->child) {
                p->next = p->child;
            } else {
                p->next = plist.front();
                plist.pop_front();
            }
            p = p->next;
        }
    }
};

