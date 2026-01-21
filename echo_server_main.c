#include "echo_server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "无效的端口号: %s\n", argv[1]);
            fprintf(stderr, "使用方法: %s [端口号]\n", argv[0]);
            return 1;
        }
    }
    
    printf("启动 Echo Server，端口: %d\n", port);
    
    if (echo_server_start(port) < 0) {
        fprintf(stderr, "启动服务器失败\n");
        return 1;
    }
    
    return 0;
}
