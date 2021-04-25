# k1ll3r-stack-guard

ksg, short for k1ll3r-stack-guard, is a little small and tiny util for adding `canary`.

## Build & Install

```Shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## Example

```c
// main.c
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
```

```Shell
>> ksg-clang main.c -o main
[+] Implementation @ vul.
[+] Implementation @ main.
>> ./main
Somthing plz:
> pwn
Somthing plz:
> hacked_by_yype
[!] Stack overflow detected!
[1] 3549366 abort (core dumped)  ./main
```
