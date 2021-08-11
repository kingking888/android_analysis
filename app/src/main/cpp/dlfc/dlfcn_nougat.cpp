#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <android/log.h>
#include "dlfcn_nougat.h"

#define TAG_NAME    "nougat_dlfcn"

#define log_info(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG_NAME, (const char *) fmt, ##args)
#define log_err(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG_NAME, (const char *) fmt, ##args)

#ifdef LOG_DBG
#define log_dbg log_info
#else
#define log_dbg(...)
#endif


int fake_dlclose(elf_info *elf_info) {
    if (elf_info) {
        regfree(&elf_info->reg);
        if (elf_info->dynsym) free(elf_info->dynsym);    /* we're saving dynsym and dynstr */
        if (elf_info->dynstr) free(elf_info->dynstr);    /* from library file just in case */
        free(elf_info);
    }
    return 0;
}

#define fatal(fmt, args...) do { log_err(fmt,##args); goto err_exit; } while(0)

/* flags are ignored */
elf_info *fake_dlopen(const char *libpath, int flags) {
    FILE *maps;
    char buff[256];
    elf_info *elf_info = 0;
    off_t load_addr, end_addr, size;
    int k, fd = -1, found = 0;
    char *shoff;
    Elf_Ehdr *elf = (Elf_Ehdr *) MAP_FAILED;

    elf_info = (struct elf_info *) calloc(1, sizeof(struct elf_info));
    if (!elf_info)
        fatal("no memory for %s", libpath);

    if (regcomp(&elf_info->reg, libpath, REG_EXTENDED) != 0) {
        fatal("regcomp error %s", libpath);
    }

    elf_info->reg_path = libpath;

    maps = fopen("/proc/self/maps", "r");
    if (!maps)
        fatal("failed to open maps");

    while (!found && fgets(buff, sizeof(buff), maps)) {
        if ((strstr(buff, "r-xp") || strstr(buff, "r--p")) &&
            regexec(&elf_info->reg, buff, 0, nullptr, 0) == 0) {
            found = 1;
            elf_info->full_path = strstr(buff, "/");
            size_t n = elf_info->full_path.find_last_not_of("\r\n\t");
            if (n != string::npos) {
                elf_info->full_path.erase(n + 1, elf_info->full_path.size() - n);
            }
            break;
        }
    }

    fclose(maps);

    if (!found)
        fatal("%s not found in my userspace", libpath);

    if (sscanf(buff, "%lx-%lx", &load_addr, &end_addr) != 2)
        fatal("failed to read load address for %s", libpath);

    log_info("%s loaded in Android at 0x%08lx", elf_info->full_path.c_str(), load_addr);

    /* Now, mmap the same library once again */

    fd = open(elf_info->full_path.c_str(), O_RDONLY);
    if (fd < 0)
        fatal("failed to open %s", elf_info->full_path.c_str());

    size = lseek(fd, 0, SEEK_END);
    if (size <= 0)
        fatal("lseek() failed for %s", elf_info->full_path.c_str());


    elf = (Elf_Ehdr *) mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    fd = -1;

    if (elf == MAP_FAILED)
        fatal("mmap() failed for %s", elf_info->full_path.c_str());

    elf_info->load_addr = (void *) load_addr;
    elf_info->end_addr = (void *) end_addr;
    elf_info->file_size = size;

    shoff = ((char *) elf) + elf->e_shoff;

    for (k = 0; k < elf->e_shnum; k++, shoff += elf->e_shentsize) {

        Elf_Shdr *sh = (Elf_Shdr *) shoff;
        log_dbg("%s: k=%d shdr=%p type=%x", __func__, k, sh, sh->sh_type);

        switch (sh->sh_type) {
            case SHT_DYNSYM:
                if (elf_info->dynsym)
                    fatal("%s: duplicate DYNSYM sections",
                          elf_info->full_path.c_str()); /* .dynsym */

                elf_info->dynsym = (Elf_Sym *) malloc(sh->sh_size);

                if (!elf_info->dynsym)
                    fatal("%s: no memory for .dynsym", elf_info->full_path.c_str());
                memcpy(elf_info->dynsym, ((char *) elf) + sh->sh_offset, sh->sh_size);
                elf_info->nsyms = (sh->sh_size / sizeof(Elf_Sym));
                break;

            case SHT_STRTAB:
                if (elf_info->dynstr)
                    break;    /* .dynstr is guaranteed to be the first STRTAB */
                elf_info->dynstr = malloc(sh->sh_size);
                if (!elf_info->dynstr)
                    fatal("%s: no memory for .dynstr", elf_info->full_path.c_str());
                memcpy(elf_info->dynstr, ((char *) elf) + sh->sh_offset, sh->sh_size);
                break;

            case SHT_PROGBITS:
                if (!elf_info->dynstr || !elf_info->dynsym)
                    break;
                /* won't even bother checking against the section name */
                elf_info->bias = (off_t) sh->sh_addr - (off_t) sh->sh_offset;
                k = elf->e_shnum;  /* exit for */
                break;
        }
    }

    munmap(elf, size);
    elf = 0;

    if (!elf_info->dynstr || !elf_info->dynsym)
        fatal("dynamic sections not found in %s", elf_info->full_path.c_str());

#undef fatal

    log_dbg("%s: ok, dynsym = %p, dynstr = %p", elf_info->full_path.c_str(), elf_info->dynsym,
            elf_info->dynstr);

    return elf_info;

    err_exit:
    if (fd >= 0)
        close(fd);
    if (elf != MAP_FAILED)
        munmap(elf, size);
    fake_dlclose(elf_info);
    return nullptr;
}

void *fake_dlsym(elf_info *elf_info, const char *name) {
    int k;
    auto *sym = elf_info->dynsym;
    char *strings = (char *) elf_info->dynstr;

    for (k = 0; k < elf_info->nsyms; k++, sym++)
        if (strcmp(strings + sym->st_name, name) == 0) {
            /*  NB: sym->st_value is an offset into the section for relocatables,
            but a VMA for shared libs or exe files, so we have to subtract the bias */
            void *ret = (char *) elf_info->load_addr + sym->st_value - elf_info->bias;
            log_info("%s found at %p", name, ret);
            return ret;
        }
    return nullptr;
}

const char *fake_get_sym_name(elf_info *handle, int index) {
    auto *sym = handle->dynsym;
    char *sym_name = (char *) handle->dynstr + (sym + index)->st_name;
    return sym_name;
}