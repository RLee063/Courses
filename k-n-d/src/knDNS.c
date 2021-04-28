#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <uv.h>
#include <uv/unix.h>
#include "dns_query.hpp"

uv_loop_t *loop;
uv_pipe_t stdin_pipe;

void alloc_stdin_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf){
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
    return;
}

void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf){
    if(nread < 0){
        if(nread == UV_EOF){
            uv_close((uv_handle_t *)&stdin_pipe, NULL);
        }
    } 
    else if(nread > 0){
        k_get_addrinfo(buf, nread);		
    }
    return;
}

int main(int argc, char **argv) {
    loop = uv_default_loop();
    
    // Init udp socket
    k_dns_init(loop);

    // Init stdin stream
    uv_pipe_init(loop, &stdin_pipe, 0);
    uv_pipe_open(&stdin_pipe, 0);    
    uv_read_start((uv_stream_t*)&stdin_pipe, alloc_stdin_buffer, read_stdin);

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
