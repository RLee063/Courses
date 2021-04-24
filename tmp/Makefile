all: main

libkss-stack-guard.a:
	clang -c ./kss-stack-guard.c
	ar -r libkss-stack-guard.a kss-stack-chk.o

share_lib:
	clang `llvm-config --cxxflags` -Wl,-znodelete -fno-rtti -fPIC -shared kss-llvm-pass.so.cc -o kss-llvm-pass.so `llvm-config --ldflags`

main: share_lib libkss-stack-guard.a
	clang -Xclang -load -Xclang ./kss-llvm-pass.so main.c -L. -lkss-stack-guard -ggdb -o main

