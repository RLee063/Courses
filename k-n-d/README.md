# k1ll3r-nonBlock-DNSClient
knDNS, short for k1ll3r-nonblock-DNSClient, is a small tiny and useless DNS client, based on [libuv](https://github.com/libuv/libuv).

## Course requirements

* Following [RFC1035](https://tools.ietf.org/html/rfc1035) to implement a nonblock DNS client.
* Based on Linux, allowed to use existing Reactor network libraries such as muduo, libevent, libuv, etc.

## Build

```Shell
mkdir build
cd build
cmake ..
make
```

## Example

```
> ./knDNS
weibo.com
[+] Query for {0}weibo.com
[*] {0}weibo.com -> 58.205.212.119
k1ll3r.io
[+] Query for {0}k1ll3r.io
cnss.io
[+] Query for {1}cnss.io
[*] {1}cnss.io -> 172.67.171.38
```
