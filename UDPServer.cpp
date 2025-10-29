// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <fcntl.h> 
  
#define PORT     8000 
#define MAXLINE 1024 
  
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    const char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("socket non-block failed 1"); 
        exit(EXIT_FAILURE); 
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("socket non-block failed 2"); 
        exit(EXIT_FAILURE); 
    }
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    socklen_t len;
  	int n; 
  
    len = sizeof(cliaddr);  //len is value/result 
  
    // n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
    //             MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
    //             &len); 
    // buffer[n] = '\0'; 
    // printf("Client : %s\n", buffer); 
    // sendto(sockfd, (const char *)hello, strlen(hello),  
    //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
    //         len); 


    while (true) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                     0, (struct sockaddr *)&cliaddr, &len);

        if (n > 0) {
            buffer[n] = '\0';
            printf("Client : %s\n", buffer);

            sendto(sockfd, hello, strlen(hello),
                   0, (const struct sockaddr *)&cliaddr, len);
            std::cout << "Hello message sent." << std::endl;

            // 종료 신호 예시
            if (strcmp(buffer, "end") == 0) break;

        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // ★ 받을 데이터가 현재 없음: 블로킹하지 말고 잠깐 쉬었다가 다시 시도
            usleep(10 * 1000); // 10ms
            continue;

        } else if (n == -1) {
            perror("recvfrom");
            break;
        }

    }

    close(sockfd);
      
    return 0; 
}