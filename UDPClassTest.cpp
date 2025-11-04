#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdexcept>

constexpr size_t HEADER_PACKET_SIZE = 13;

class HeaderPacket {
private:
    char src_id[5];     // 송신자 ID (예: "D001")
    char dest_id[5];    // 수신자 ID (예: "M001")
    uint32_t seq;       // 시퀀스 번호
    uint8_t msg_size;   // 메시지 크기

public:
    HeaderPacket() = default;

    HeaderPacket(const char* s_id, const char* d_id, uint32_t s, uint8_t size)
        : seq(s), msg_size(size) {
        std::strncpy(src_id, s_id, sizeof(src_id) - 1);
        src_id[4] = '\0';
        std::strncpy(dest_id, d_id, sizeof(dest_id) - 1);
        dest_id[4] = '\0';
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer(HEADER_PACKET_SIZE);
        std::memcpy(&buffer[0], src_id, 4);
        std::memcpy(&buffer[4], dest_id, 4);
        buffer[8]  = (seq >> 24) & 0xFF;
        buffer[9]  = (seq >> 16) & 0xFF;
        buffer[10] = (seq >> 8) & 0xFF;
        buffer[11] = seq & 0xFF;
        buffer[12] = msg_size;
        return buffer;
    }

    static HeaderPacket deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < HEADER_PACKET_SIZE)
            throw std::runtime_error("Buffer too small for HeaderPacket");

        HeaderPacket pkt;
        std::memcpy(pkt.src_id, &buffer[0], 4);
        pkt.src_id[4] = '\0';
        std::memcpy(pkt.dest_id, &buffer[4], 4);
        pkt.dest_id[4] = '\0';
        pkt.seq = (static_cast<uint32_t>(buffer[8]) << 24) |
                  (static_cast<uint32_t>(buffer[9]) << 16) |
                  (static_cast<uint32_t>(buffer[10]) << 8) |
                  (static_cast<uint32_t>(buffer[11]));
        pkt.msg_size = buffer[12];
        return pkt;
    }

    void print() const {
        std::cout << "HeaderPacket("
                  << "src_id=" << src_id << ", "
                  << "dest_id=" << dest_id << ", "
                  << "seq=" << seq << ", "
                  << "msg_size=" << static_cast<unsigned>(msg_size)
                  << ")\n";
    }
};

int main(int argc, char** argv) {
    // 사용법: ./client <server_ip> [port] [seq] [src_id] [dest_id]
    const char* ip = (argc >= 2) ? argv[1] : "127.0.0.1";
    int port = (argc >= 3) ? std::stoi(argv[2]) : 9001;
    uint32_t seq = (argc >= 4) ? static_cast<uint32_t>(std::stoul(argv[3])) : 1;
    const char* src_id = (argc >= 5) ? argv[4] : "D001";
    const char* dest_id = (argc >= 6) ? argv[5] : "M001";

    int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    timeval tv{};
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_RCVTIMEO");
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv.sin_addr) != 1) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    HeaderPacket out(src_id, dest_id, seq, /*msg_size=*/0);
    std::vector<uint8_t> buf = out.serialize();

    ssize_t s = sendto(sockfd, buf.data(), buf.size(), 0,
                       reinterpret_cast<sockaddr*>(&serv), sizeof(serv));
    if (s < 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    }

    std::cout << "[Client] sent header (" << s << " bytes): ";
    out.print();

    uint8_t rbuf[1024];
    sockaddr_in from{};
    socklen_t flen = sizeof(from);
    ssize_t n = recvfrom(sockfd, rbuf, sizeof(rbuf), 0,
                         reinterpret_cast<sockaddr*>(&from), &flen);
    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        return 1;
    }

    if (n < static_cast<ssize_t>(HEADER_PACKET_SIZE)) {
        std::cerr << "[Client] too short reply: " << n << " bytes\n";
        close(sockfd);
        return 1;
    }

    std::vector<uint8_t> recvBuf(rbuf, rbuf + n);
    HeaderPacket in = HeaderPacket::deserialize(recvBuf);
    std::cout << "[Client] received header (" << n << " bytes): ";
    in.print();

    close(sockfd);
    return 0;
}
