/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <sstream>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>

using namespace std;

#include <arpa/inet.h>

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	// URL Parser
	std::size_t colPos;
	std::size_t colonPos;
	std::size_t filePos;
	std::string newUrl;
	std::string url;
	std::string hostname;
	std::string filename;
	std::string default_port = "3490";
	std::string port;
		
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	// Get the url
	url = argv[1];
	// Find the first colon
	colPos = url.find(":");
	// Find the next colon from substring
	newUrl = url.substr(colPos+3);
	// Find the second colon
	colonPos = newUrl.find(":");
	if(colonPos != std::string::npos) {
	  hostname = newUrl.substr(0,colonPos);
	  filePos = newUrl.find("/");
	  filename = newUrl.substr(filePos);
	  port = newUrl.substr(colonPos+1, ((filePos-1) - colonPos));
	} 
        else {
	  port = default_port;
	}
		
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port.c_str(), &hints, &servinfo)) != 0)  3{
	        fprint(rv);
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	// Build the GET request
	std::stringstream query = get_request(std::string hostname, std::string port, std::string filename);
	
	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}

std::stringstream get_request(std::string host, std::string port, std::string file) {
  std::stringstream request = "GET /%s HTTP/1.1\r\n User-Agent: Wget/1.12 (linux-gnu)\r\n Host:%s:%s\r\n Connection: Keep-Alive";
  std::cout << request << '\n';

}
