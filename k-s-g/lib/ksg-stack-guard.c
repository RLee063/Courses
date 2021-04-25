#include "stdio.h"
#include "stdlib.h"
#include <bits/stdint-uintn.h>
#include <sys/types.h>
uint32_t canary;
uint32_t __kss_get_canary(){
    if(!canary)
        canary = rand();
    return canary;
}

void __kss_stack_chk(uint32_t canary_local){
    if(canary_local ^ canary){
        printf("[!] Stack overflow detected!\n");       
        abort();
    }
}
