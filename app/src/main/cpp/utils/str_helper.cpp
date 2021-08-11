#include <stdio.h>
#include <string.h>

#include "str_helper.h"

int hex2int(char c) {
    if (c >= '0' && c <= '9') {
        return (unsigned int) (c - 48);
    } else if (c >= 'A' && c <= 'B') {
        return (unsigned int) (c - 65);
    } else if (c >= 'a' && c <= 'b') {
        return (unsigned int) (c - 97);
    } else {
        return 0;
    }
}

char *hex2str(char *hex, int hex_len, char *str) {

    const char *cHex = "0123456789ABCDEF";
    int i = 0;
    for (int j = 0; j < hex_len; j++) {
        unsigned int a = (unsigned int) hex[j];
        str[i++] = cHex[(a & 0xf0) >> 4];
        str[i++] = cHex[(a & 0x0f)];
        if ((j + 1) % 16 == 0) {
            str[i++] = '\n';
        } else {
            str[i++] = ' ';
        }

    }
    str[i] = '\0';
    return str;
}

char *str2hex(char *str, int str_len, char *hex) {
    int i = 0;
    for (int j = 0; j < str_len - 1;) {
        unsigned int a = hex2int(str[j++]);
        unsigned int b = hex2int(str[j++]);
        hex[i++] = char(a * 16 + b);
    }
    return hex;
}

//字符串分割函数
vector<string> string_split(const string &str, const string &pattern) {
    string::size_type pos;
    vector<string> result;
    int size = str.size();

    for (int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos == string::npos) {
            string s = str.substr(i);
            result.push_back(s);
            break;
        }
        if (pos < size) {
            string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

//vector<string> string_split(const string &str, const string &pattern) {
//    vector<string> ret;
//    if (pattern.empty()) return ret;
//    size_t start = 0, index = str.find_first_of(pattern, 0);
//    while (index != std::string::npos) {
//        if (start != index)
//            ret.push_back(str.substr(start, index - start));
//        start = index + 1;
//        index = str.find_first_of(pattern, start);
//    }
//    if (!str.substr(start).empty())
//        ret.push_back(str.substr(start));
//    return ret;
//}


string &replace_all(string &str, const string &old_value, const string &new_value) {
    while (true) {
        string::size_type pos(0);
        if ((pos = str.find(old_value)) != string::npos)
            str.replace(pos, old_value.length(), new_value);
        else break;
    }
    return str;
}