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

// #define PORT     8000 
#define MAXLINE 1024 
#define MAX_EVENTS 100

int main(){

	char buffer[MAXLINE]; 
    const char *hello = "Hello from server"; 

	int rx1fd, rx2fd, rx3fd, txfd;
	struct sockaddr_in rx1addr, rx2addr, rx3addr, txaddr;


	// UDP 소켓 객체 생성 
    if((rx1fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
    	perror("rx1 creation failed"); 
        exit(EXIT_FAILURE); 
    }
    if((rx2fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
    	perror("rx2 creation failed"); 
        exit(EXIT_FAILURE); 
    }
    if((rx3fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
    	perror("rx3 creation failed"); 
        exit(EXIT_FAILURE); 
    }
    if((txfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
    	perror("tx creation failed"); 
        exit(EXIT_FAILURE); 
    }

    // 소켓 객체 논블로킹으로 설정
    int flags = fcntl(rx1fd, F_GETFL, 0);
    if (flags < 0) {
        perror("socket non-block failed 1"); 
        exit(EXIT_FAILURE); 
    }
    if (fcntl(rx1fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("socket non-block failed 2"); 
        exit(EXIT_FAILURE); 
    }

    flags = fcntl(rx2fd, F_GETFL, 0);
    if (flags < 0) {
        perror("socket non-block failed 1"); 
        exit(EXIT_FAILURE); 
    }
    if (fcntl(rx2fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("socket non-block failed 2"); 
        exit(EXIT_FAILURE); 
    }

    flags = fcntl(rx3fd, F_GETFL, 0);
    if (flags < 0) {
        perror("socket non-block failed 1"); 
        exit(EXIT_FAILURE); 
    }
    if (fcntl(rx3fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("socket non-block failed 2"); 
        exit(EXIT_FAILURE); 
    }

    flags = fcntl(txfd, F_GETFL, 0);
    if (flags < 0) {
        perror("socket non-block failed 1"); 
        exit(EXIT_FAILURE); 
    }
    if (fcntl(txfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("socket non-block failed 2"); 
        exit(EXIT_FAILURE); 
    }


    // UDP 소켓 포트 번호 설정
    memset(&rx1addr, 0, sizeof(rx1addr)); 
    memset(&rx2addr, 0, sizeof(rx2addr)); 
    memset(&rx3addr, 0, sizeof(rx3addr)); 
    memset(&txaddr, 0, sizeof(txaddr)); 

    rx1addr.sin_family    = AF_INET; // IPv4 
    rx1addr.sin_addr.s_addr = INADDR_ANY; 
    rx1addr.sin_port = htons(9001); 

    rx2addr.sin_family    = AF_INET; // IPv4 
    rx2addr.sin_addr.s_addr = INADDR_ANY; 
    rx2addr.sin_port = htons(9002); 

    rx3addr.sin_family    = AF_INET; // IPv4 
    rx3addr.sin_addr.s_addr = INADDR_ANY; 
    rx3addr.sin_port = htons(9003); 

    txaddr.sin_family    = AF_INET; // IPv4 
    txaddr.sin_addr.s_addr = INADDR_ANY; 
    txaddr.sin_port = htons(9000); 


    // 소켓 객체와 정보 바인딩
    if ( bind(rx1fd, (const struct sockaddr *)&rx1addr, sizeof(rx1addr)) < 0 ) 
    { 
        perror("rx1 bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if ( bind(rx2fd, (const struct sockaddr *)&rx2addr, sizeof(rx2addr)) < 0 ) 
    { 
        perror("rx2 bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if ( bind(rx3fd, (const struct sockaddr *)&rx3addr, sizeof(rx3addr)) < 0 ) 
    { 
        perror("rx3 bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if ( bind(txfd, (const struct sockaddr *)&txaddr, sizeof(txaddr)) < 0 ) 
    { 
        perror("tx bind failed"); 
        exit(EXIT_FAILURE); 
    } 


    // epoll 인스턴스 생성
    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_creation failed");
        exit(EXIT_FAILURE);
    }


    // 어떤 fd의 어떤 이벤트를 감시할지 설정
    epoll_event event{};
    event.events = EPOLLIN;  // 읽기 이벤트 감시
	event.data.fd = rx1fd;  // 어떤 FD인지 넣어둠

	// 파일 디스크립터를 epoll 인스턴스에 추가
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, rx1fd, &event) < 0 ) {
        perror("epoll_ctl rx1");
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    event.data.fd = rx2fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, rx2fd, &event) < 0 ) {
        perror("epoll_ctl rx2");
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    event.data.fd = rx3fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, rx3fd, &event) < 0 ) {
        perror("epoll_ctl rx3");
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    event.data.fd = txfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, txfd, &event) < 0 ) {
        perror("epoll_ctl tx");
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    // 발생한 이벤트 저장용
    epoll_event events[MAX_EVENTS];

    while(true){
    	int n = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    	if (n < 0) {
        	if (errno == EINTR) continue;
        	perror("epoll_wait error");
        	close(epollfd);
        	exit(EXIT_FAILURE);
    	}

    	for(int i=0; i<n; i++){
    		int curfd = events[i].data.fd;
    		int curevent = events[i].events;

    		if (curevent & EPOLLIN) {
    			while (true) {
    				socklen_t len = sizeof(txaddr);
    				int recvsize = recvfrom(curfd, (char *)buffer, MAXLINE,
			                     0, (struct sockaddr *)&txaddr, &len);

			        if (recvsize > 0) {
			            buffer[recvsize] = '\0';
			            printf("Client : %s\n", buffer);

			            sendto(curfd, hello, strlen(hello),
			                   0, (const struct sockaddr *)&txaddr, len);
			            std::cout << "Hello message sent." << std::endl;

			            // 종료 신호 예시
			            if (strcmp(buffer, "end") == 0) break;

			        } else if (recvsize == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			            // ★ 받을 데이터가 현재 없음: 블로킹하지 말고 잠깐 쉬었다가 다시 시도
			            break;
			        } else if (recvsize == -1) {
			            perror("recvfrom");
			            break;
			        }

			    }
    		}
    	}

    }

    close(rx1fd);
    close(rx2fd);
    close(rx3fd);
    close(txfd);
    close(epollfd);

	return 0;
}