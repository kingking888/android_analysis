#pragma once

#include <string>
#include <vector>
#include <jni.h>

using std::string;
using std::vector;

char *str2hex(char *str, int str_len, char *hex);

char *hex2str(char *hex, int hex_len, char *str);

vector<string> string_split(const string &str, const string &pattern);

string &replace_all(string &str, const string &old_value, const string &new_value);

