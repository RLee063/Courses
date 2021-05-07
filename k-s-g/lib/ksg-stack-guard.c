#include "stdio.h"
#include "stdlib.h"
#include <bits/stdint-uintn.h>
#include <sys/types.h>
uint32_t __kss_canary;
void __kss_get_canary(){
    if (!__kss_canary){
        __kss_canary = rand();
    }
}

void __kss_stack_chk_fail(){
    printf("[!] Stack overflow detected!\n");
    abort();
}

void __kss_stack_chk(uint32_t canary_local){
    if(canary_local ^ __kss_canary){
        printf("[!] Stack overflow detected!\n");       
        abort();
    }
}
