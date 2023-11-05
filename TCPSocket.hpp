#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

class TCPSocket
{
    protected:
        int sockfd;
        struct sockaddr_in address;  
    public:
        TCPSocket(std::string ip, int p = 21)
        {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0){
                printf("Error while creating socket\n");
            }
            
            memset(address.sin_zero, 0, sizeof(address.sin_zero));
            address.sin_family = AF_INET;
            this->peer(ip.c_str());
            this->port(p);
        };

        ~TCPSocket() { ::close(sockfd); }
        
        void close() { ::close(sockfd); }

        void connect()
        {
            if(::connect(sockfd, (sockaddr *)&address, sizeof(address)) < 0)
            {
                perror("Error while connecting to FTP server");
                exit(1);
            }
        }
        void port(int port)
        {
            address.sin_port = htons(port);
        };
        void peer(const char* address)
        {
            this->address.sin_addr.s_addr  = inet_addr(address);
        }
        sockaddr_in peer()
        {
            return this->address;
        }
        int receive(char *buf, int count)
        {
            socklen_t slen = sizeof(address) ;
            return recvfrom(sockfd, buf, count, 0, (struct sockaddr *) &address, &slen);
        }
        int send(const char *buf, int count)
        {
            return sendto(sockfd, buf, count, 0, (struct sockaddr *) &address, sizeof(address));
        }
};
