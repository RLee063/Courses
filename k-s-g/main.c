#include "stdio.h"

void vul(){
    printf("Somthing plz:\n> ");
    char tmp[] = "pwn";
    scanf("%s", tmp);
}

int main(){
    while(1){
        vul();
    }
    return 0;
}
