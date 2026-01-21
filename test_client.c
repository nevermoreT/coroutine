#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8888
#define DEFAULT_HOST "127.0.0.1"

int main(int argc, char *argv[]) {
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    
    printf("连接到服务器 %s:%d\n", host, port);
    
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        return 1;
    }
    
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        close(sockfd);
        return 1;
    }
    
    // 连接服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        close(sockfd);
        return 1;
    }
    
    printf("连接成功！\n");
    printf("输入消息（输入 'quit' 退出）:\n");
    
    char buffer[BUFFER_SIZE];
    
    // 发送测试消息
    const char *test_messages[] = {
        "Hello, Server!",
        "This is a test message.",
        "协程 Echo Server 测试",
        "1234567890",
        "quit"
    };
    
    int num_messages = sizeof(test_messages) / sizeof(test_messages[0]);
    
    for (int i = 0; i < num_messages; i++) {
        const char *msg = test_messages[i];
        printf("\n[发送] %s\n", msg);
        
        // 发送消息
        ssize_t sent = send(sockfd, msg, strlen(msg), 0);
        if (sent < 0) {
            perror("send error");
            break;
        }
        
        // 接收回显
        ssize_t received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (received < 0) {
            perror("recv error");
            break;
        } else if (received == 0) {
            printf("服务器关闭连接\n");
            break;
        } else {
            buffer[received] = '\0';
            printf("[接收] %s\n", buffer);
        }
        
        // 检查是否退出
        if (strcmp(msg, "quit") == 0) {
            break;
        }
        
        // 短暂延迟
        usleep(100000);  // 100ms
    }
    
    printf("\n关闭连接\n");
    close(sockfd);
    
    return 0;
}
