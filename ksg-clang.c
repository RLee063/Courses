#include "malloc.h"
#include "unistd.h"
int main(int argc, char *argv[]){
    char **argvv = (char**)malloc((argc+7)*sizeof(char*));
    argvv[0] = "clang";
    int i=1;
    for(i=1; i<argc; i++){
        argvv[i] = argv[i];
    }
    argvv[i++] = "-Xclang";
    argvv[i++] = "-load";
    argvv[i++] = "-Xclang";
    argvv[i++] = "./kss-llvm-pass.so";
    argvv[i++] = "-L.";
    argvv[i++] = "-lkss-stack-guard";
    argvv[i] = NULL;
    execv("/bin/clang", argvv);
}
