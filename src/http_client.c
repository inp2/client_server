/*
** client.c -- a stream socket client demo
*/

#include <iostream>
#include <fstream>
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

#define BUFFERSIZE 512

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

std::string read_response(int fd)
{
	int n;
	char buffer[BUFFERSIZE];
	std::stringstream response;
	bzero(buffer, BUFFERSIZE);
	bool past_header = false;
	while((n = read(fd, buffer, BUFFERSIZE-1)) != 0)
	{
		char *carriage_ret = strstr(buffer, "\r\n\r\n");
		if (carriage_ret != NULL) {
			int pos = carriage_ret - buffer;
			past_header = true;
			char *tmpbuf = buffer + pos + 4;
			response << tmpbuf;
		} else if (past_header) response << buffer;

		bzero(buffer, BUFFERSIZE);
	}
	return response.str();
}


int main(int argc, char *argv[])
{
	int sockfd;
	// char buf[BUFFERSIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	// URL Parser
	// std::size_t colonPos;
	std::size_t filePos;
	std::string newUrl;
	std::string url;
	std::string hostname;
	std::string filename;
	std::string default_port = "80";
	std::string port;

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	// Get the url
	url = argv[1];
	// find the first colon (protocol)
	std::size_t colPos = url.find(":");
	// Find the last colon (port)
        newUrl = url.substr(colPos+1);
	std::size_t colonPos = newUrl.find(":");

	if(colonPos != std::string::npos) {
	  hostname = newUrl.substr(2,colonPos-2);
	  filePos = newUrl.find_last_of("/");
	  cout << newUrl << "\n";
	  filename = newUrl.substr(filePos);
	  port = newUrl.substr(colonPos+1, ((filePos-1) - colonPos));
	}
        else {
	  filePos = newUrl.find_last_of("/");
	  hostname = newUrl.substr(2, filePos-2);
	  filename = newUrl.substr(filePos);
	  port = default_port;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
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

	// Create the request
	std::stringstream rstream;
	rstream << "GET " << filename << " HTTP/1.1\r\n\r\n";
	std::string request = rstream.str();
	printf("client: sent %s\n", request.c_str());
	printf("client waiting for response\n");

	// Send Request
	if (write(sockfd, request.c_str(), request.length()) <  0)
	{
		perror("send failed\n");
	}

	std::string get_response = read_response(sockfd);
	std::ofstream ofile;
	/* char fname[filename.size()]; */
	/* std::copy(filename.begin()+1, filename.end(), fname); */
	/* fname[filename.size()-1] = '\0'; */
	ofile.open("output", std::ios::out | std::ios::trunc | std::ios::binary);
	ofile << get_response;
	ofile.close();

	close(sockfd);

	return 0;
}
