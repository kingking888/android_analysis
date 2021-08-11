#include <jni.h>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include "route_func.h"


extern "C" JNIEXPORT  int32_t JNICALL
mod_for_asm(int32_t num, int32_t mod) {
    return num % mod;
}

#if defined(__arm64__) || defined(__aarch64__)

const unsigned char
       shellcode_hooking_func[] = {0xFD, 0x7B, 0xBF, 0xA9, 0xE8, 0x27, 0xBF, 0xA9, 0x88,
                                   0x88, 0x88, 0xF2, 0x68, 0x66, 0xA6, 0xF2, 0x48, 0x44,
                                   0xC4, 0xF2, 0x28, 0x22, 0xE2, 0xF2, 0xE9, 0x83, 0,
                                   0x91, 0xE8, 0x27, 0xBF, 0xA9, 0x88, 0x88, 0x88, 0xF2,
                                   0x68, 0x66, 0xA6, 0xF2, 0x48, 0x44, 0xC4, 0xF2, 0x28,
                                   0x22, 0xE2, 0xF2, 0, 1, 0x3F, 0xD6, 0xFF, 0x43, 0,
                                   0x91, 0xE8, 0x27, 0xC1, 0xA8, 0xFD, 0x7B, 0xC1, 0xA8,
                                   0xC0, 3, 0x5F, 0xD6
};

void patch_movk(unsigned char *pmovk, uint16_t num) {
   uint32_t ref_pmovk = *(uint32_t *) pmovk;
   uint32_t patch_date = num;
   uint32_t low_5 = GET_BIT(uint32_t, ref_pmovk, 0, 5);
   uint32_t hi_10 = GET_BIT(uint32_t, ref_pmovk, 21, 11);
   *(uint32_t *) pmovk = 0;
   *(uint32_t *) pmovk = hi_10 << 21 | patch_date << 5 | low_5;
}

unsigned char *create_shellcode_hooking_func(new_pfunc_t new_pfunc, void *context) {
   auto *shell_code = new unsigned char[sizeof(shellcode_hooking_func)];
   memcpy(shell_code, shellcode_hooking_func, sizeof(shellcode_hooking_func));
   //context
   patch_movk(&shell_code[8], GET_BIT(uint64_t, context, 0, 16));
   patch_movk(&shell_code[12], GET_BIT(uint64_t, context, 16, 16));
   patch_movk(&shell_code[16], GET_BIT(uint64_t, context, 32, 16));
   patch_movk(&shell_code[20], GET_BIT(uint64_t, context, 48, 16));
   //new_pfunc
   patch_movk(&shell_code[32], GET_BIT(uint64_t, new_pfunc, 0, 16));
   patch_movk(&shell_code[36], GET_BIT(uint64_t, new_pfunc, 16, 16));
   patch_movk(&shell_code[40], GET_BIT(uint64_t, new_pfunc, 32, 16));
   patch_movk(&shell_code[44], GET_BIT(uint64_t, new_pfunc, 48, 16));

   SET_MEMORY_PROTECT(shell_code, PROT_READ | PROT_WRITE | PROT_EXEC);
   return shell_code;
}

#else

const unsigned char
        shellcode_hooking_func[] = {0x70, 0x40, 0x2D, 0xE9, 0x78, 0x46, 5, 0xE3, 0x34,
                                    0x42, 0x41, 0xE3, 0x10, 0x50, 0x8D, 0xE2, 0x30, 0,
                                    0x2D, 0xE9, 0x78, 0x46, 5, 0xE3, 0x34, 0x42, 0x41,
                                    0xE3, 0x34, 0xFF, 0x2F, 0xE1, 8, 0xD0, 0x8D, 0xE2,
                                    0x70, 0x80, 0xBD, 0xE8
};


void patch_mov(unsigned char *pmov, uint16_t num) {
    uint32_t ref_pmov = *(uint32_t *) pmov;
    uint32_t patch_date_low_12 = GET_BIT(uint16_t, num, 0, 12);
    uint32_t patch_date_hi_4 = GET_BIT(uint16_t, num, 12, 4);
    uint32_t hi_20 = ref_pmov >> 20;
    uint32_t rd = GET_BIT(uint32_t, ref_pmov, 12, 4);
    *(uint32_t *) pmov = 0;
    *(uint32_t *) pmov = hi_20 << 20 | patch_date_hi_4 << 16 | rd << 12 | patch_date_low_12;
}

unsigned char *create_shellcode_hooking_func(new_pfunc_t new_pfunc, void *context) {
    auto *shell_code = new unsigned char[sizeof(shellcode_hooking_func)];
    memcpy(shell_code, shellcode_hooking_func, sizeof(shellcode_hooking_func));

    //context
    patch_mov(&shell_code[4], GET_BIT(uint32_t, context, 0, 16));
    patch_mov(&shell_code[8], GET_BIT(uint32_t, context, 16, 16));
    //new_pfunc
    patch_mov(&shell_code[20], GET_BIT(uint32_t, new_pfunc, 0, 16));
    patch_mov(&shell_code[24], GET_BIT(uint32_t, new_pfunc, 16, 16));

    SET_MEMORY_PROTECT(shell_code, PROT_READ | PROT_WRITE | PROT_EXEC);
    return shell_code;
}

#endif



