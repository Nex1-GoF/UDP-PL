// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
  
#define PORT     8000 
#define MAXLINE 1024 
#define MAX_EVENTS 100


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


    // epoll 생성 
    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    epoll_event event{};
    event.events = EPOLLIN;          // 읽기 이벤트 감시
	event.data.fd = sockfd;          // 어떤 FD인지 넣어둠

	// 파일 디스크립터를 epoll 인스턴스에 추가
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        perror("epoll_ctl");
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    
    epoll_event events[MAX_EVENTS];


    while(true){    	
    	int nready = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    	if (nready == -1) {
        	if (errno == EINTR) continue;
        	perror("epoll_wait");
        	close(epollfd);
        	exit(EXIT_FAILURE);
    	}

    	for(int i=0; i<nready; i++){
    		int fd = events[i].data.fd;

    		if (events[i].events & EPOLLIN) {

    			while (true) {
    				socklen_t len;
  					int n; 
  
    				len = sizeof(cliaddr);  //len is value/result

			        n = recvfrom(fd, (char *)buffer, MAXLINE,
			                     0, (struct sockaddr *)&cliaddr, &len);

			        if (n > 0) {
			            buffer[n] = '\0';
			            printf("Client : %s\n", buffer);

			            sendto(fd, hello, strlen(hello),
			                   0, (const struct sockaddr *)&cliaddr, len);
			            std::cout << "Hello message sent." << std::endl;

			            // 종료 신호 예시
			            if (strcmp(buffer, "end") == 0) break;

			        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			            // ★ 받을 데이터가 현재 없음: 블로킹하지 말고 잠깐 쉬었다가 다시 시도
			            continue;

			        } else if (n == -1) {
			            perror("recvfrom");
			            break;
			        }

			    }
    		}
    	}
    }

    close(sockfd);
    close(epollfd);
      
    return 0; 
}