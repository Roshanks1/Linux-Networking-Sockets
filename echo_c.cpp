#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define BUF_SIZE 1024

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 5) {
		cerr << "Usage " << argv[0] << " <host> <port> <tcp|udp> <message>\n";
		return 1;
	}
	
	//Parse Arguments
	const char* host = argv[1];
	int port = atoi(argv[2]);
	string protocol = argv[3];
	string message = argv[4];

	//Struct 
	int sock;
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(static_cast<uint16_t>(port));
	inet_pton(AF_INET, host, &serverAddr.sin_addr);

	char buffer[BUF_SIZE];

	if (protocol == "tcp") {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			perror("TCP socket");
			return 1;
		}

		if (connect(sock, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
			perror("TCP Connection");
			close(sock);
			return 1;
		}

		send(sock, message.c_str(), message.size(), 0);
		ssize_t bytes = recv(sock, buffer, BUF_SIZE - 1, 0);
		if (bytes > 0) {
			buffer[bytes] = '\0';
			cout << "Received: " << buffer << endl;
		}
		close(sock);
	}
	else if (protocol == "udp") {
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock < 0) {
			perror("UDP socket"); //UDP Socket Fail
			return 1;
		}

		sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

		socklen_t addrLen = (socklen_t)sizeof(serverAddr);
		ssize_t bytes = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (struct sockaddr*)&serverAddr, &addrLen);

		if (bytes > 0) {
			buffer[bytes] = '\0';
			cout << "Received: " << buffer << endl;
		}
		close(sock);
	}
	else {
		cerr << "Invalid protocol. Use either 'tcp' or 'udp'\n";
		return 1;
	}

	return 0;
}
