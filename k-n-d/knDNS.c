#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <uv.h>
#include <uv/unix.h>
#include "dns_query.c"

uv_loop_t *loop;
uv_pipe_t stdin_pipe;

void alloc_stdin_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0){
        if (nread == UV_EOF){
            uv_close((uv_handle_t *)&stdin_pipe, NULL);
        }
    } else if (nread > 0) {
        k_get_addrinfo(buf, nread);		
    }
}

void test(){ uv_buf_t buf = uv_buf_init((char*) malloc(1000), 1000);
    buf.base = "www.baidu.com";
    for(int i=0; i<100; i++){
        k_get_addrinfo(&buf, 10);
        if(i%10==1){
            uv_sleep(1000);
        }
    }
}

int main(int argc, char **argv) {
    loop = uv_default_loop();

    k_dns_init(loop);

    uv_pipe_init(loop, &stdin_pipe, 0);
    uv_pipe_open(&stdin_pipe, 0);    
    uv_read_start((uv_stream_t*)&stdin_pipe, alloc_stdin_buffer, read_stdin);

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
