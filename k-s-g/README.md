# k1ll3r-stack-guard

ksg, short for k1ll3r-stack-guard, is a little small and tiny util for adding `canary`.

## Details

* Only implement those functions with array.
* All implementations trying presented through inline.

## Build & Install

Target x84(-64) support was integrated in default, if you want to try somthing on other archs, see [Example Plus](#example-plus).

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
clang main.c -o main
 ./main
Somthing plz:
> oooops_this_tring_is_too_long
[1]    897694 segmentation fault (core dumped)  ./main
 ksg-clang main.c -o main
[+] Implemente @ vul.
 ./main
Somthing plz:
> oooops_this_tring_is_too_long
[!] Stack overflow detected!
[1]    898001 abort (core dumped)  ./main
```

## Example Plus

### Build for MIPS

```Shell
mkdir build
cd build
cmake ..
make
rm libksg-stack-guard.a
clang --target=mips-linux-gnu -I /usr/mips-linux-gnu/include\
               ../lib/ksg-stack-guard.c -c -o libksg-stack-guard.o
llvm-ar rc libksg-stack-guard.a libksg-stack-guard.o
clang main.c -Xclang -load -Xclang libksg-llvm-pass.so -L. -lksg-stack-guard\
          --target=mips-linux-gnu -I /usr/mips-linux-gnu/include -o main
```

### Test for MIPS

```Shell
qemu-mips-static ./main
Somthing plz:
> ooops_too_long_string_hacked_by_k1ll3r
[!] Stack overflow detected!
qemu: uncaught target signal 6 (Aborted) - core dumped
[1]    908926 abort (core dumped)  qemu-mips-static ./main
```


