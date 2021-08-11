#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <android/log.h>
#include <string>
#include <regex.h>
#include <dlfcn.h>

using std::string;

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Shdr Elf64_Shdr
#define Elf_Sym  Elf64_Sym
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym  Elf32_Sym
#endif
struct elf_info {
    string reg_path;
    string full_path;
    regex_t reg;
    void *load_addr;
    void *end_addr;
    void *dynstr;
    Elf_Sym *dynsym;
    int nsyms;
    off_t bias;
    size_t file_size;
};

int fake_dlclose(elf_info *handle);

elf_info *fake_dlopen(const char *filename, int flags);

void *fake_dlsym(elf_info *elf_info, const char *name);

void fake_get_module_info(void *handle, void **base, void **end);

const char *fake_get_sym_name(elf_info *handle, int index);


