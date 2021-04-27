#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <uv.h>
#include "dns.h"

#define MAX_DNS_PACKAGE_SIZE 1024

uv_buf_t make_dns_query_msg(char *host_name, ssize_t len_host_name) {
    for(int i=0; i<len_host_name; i++){
        if(host_name[i] == '\n'){
            host_name[i] = '\0';
        }
    }
	char *buf = (char *)malloc(MAX_DNS_PACKAGE_SIZE);
    memset((void *)buf, 0, MAX_DNS_PACKAGE_SIZE);

    DNS_HEADER *dns_hdr = (DNS_HEADER *) buf;
    dns_hdr->id = htons(1);
    dns_hdr->rd = 1;
    dns_hdr->qcount = htons(1);
    dns_hdr->ancount = htons(0);
    dns_hdr->nscount = htons(0);
    dns_hdr->arcount = htons(0);

    strcpy(buf + sizeof(DNS_HEADER)+1, host_name);
    char *p = buf+sizeof(DNS_HEADER)+1;
    u_char i = 0;
    while (p < buf+sizeof(DNS_HEADER)+strlen(host_name)+1) {
        if(*p == '.'){
            p[-i-1] = i;
            i = 0;
        }
        else{
            i++;
        }
        p++;
    }
    p[-i-1] = i;

    DNS_QUESTION_SECTION_TAIL *dns_tail = (DNS_QUESTION_SECTION_TAIL *)(p+1);
    dns_tail->qtype = htons(DNS_QTYPE_A);
    dns_tail->qclass = htons(DNS_QCLASS_IN);

    return uv_buf_init(buf, sizeof(DNS_HEADER)+sizeof(DNS_QUESTION_SECTION_TAIL)\
            +strlen(host_name)+2);
}

void alloc_dns_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_dns_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,\
        const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) req, NULL);
        free(buf->base);
        return;
    }
    //char sender[17] = {0};
    // uv_ip4_name((const struct sockaddr_in*)addr, sender, 16);
    // printf("[+] Recv from %s\n", sender);

    DNS_HEADER *header = (DNS_HEADER *)buf->base;
    if (header->ancount){
        u_char *rdata = (u_char *)(buf->base+nread-4);
        printf("[*] {%u}%s -> %u.%u.%u.%u\n", ntohs(header->id), (char *)req->data, rdata[0], rdata[1],\
                rdata[2], rdata[3]);
    }
    else{
        printf("[-] DNS query failed for %s\n", (char *)req->data);
    }

    uv_udp_recv_stop(req);
    uv_close((uv_handle_t*) req, NULL);
    free(buf->base);
    free(req);
}


void on_dns_send(uv_udp_send_t *req, int status) {
    if (status) {
        fprintf(stderr, "Send error %s\n", uv_strerror(status));
        return;
    }
}

void k_get_addrinfo(const uv_buf_t *buf, ssize_t nread) {
    uv_loop_t *dns_loop;
    uv_udp_t *send_socket = (uv_udp_t *)malloc(sizeof(uv_udp_t));
    dns_loop = uv_default_loop();
    uv_udp_init(dns_loop, send_socket);
    uv_udp_send_t send_req; 
    struct sockaddr_in send_addr;
    uv_ip4_addr("8.8.8.8", 53, &send_addr);
    uv_buf_t dns_query_msg = make_dns_query_msg(buf->base, nread);
    uv_udp_send(&send_req, send_socket, &dns_query_msg, 1,\
            (const struct sockaddr *)&send_addr, on_dns_send);
    send_socket->data = buf->base;
    uv_udp_recv_start(send_socket, alloc_dns_buffer, on_dns_read);
    uv_run(dns_loop, UV_RUN_DEFAULT);
    return;
}
