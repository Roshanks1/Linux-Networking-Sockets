#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>

#define MAX_PORTS 3
#define BUF_SIZE 1024
#define LOG_PORT 9999
#define LOG_SERVER "127.0.0.1"
#define BACKLOG 10

using namespace std;
//Latest
//2
void sendToLog(const string& clientIP, const string& message) {
	int logSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (logSock < 0) return;
	
	struct sockaddr_in logAddr;
	memset(&logAddr, 0, sizeof(logAddr));
	logAddr.sin_family = AF_INET;
	logAddr.sin_port = htons(LOG_PORT);
	inet_pton(AF_INET, LOG_SERVER, &logAddr.sin_addr);

	time_t now = time(NULL);
	char timeBuf[64];
	strftime(timeBuf, sizeof(timeBuf), + "%Y-%m-%d %H:%M:%S", localtime(&now));

	string logMsg = string(timeBuf) + " \"" + message + "\" was received from " + clientIP;

	sendto(logSock, logMsg.c_str(), logMsg.size(), 0, (struct sockaddr*)&logAddr, sizeof(logAddr));
	close(logSock);
}

void handleTCP(int clientSock, sockaddr_in clientAddr) {
	char buffer[BUF_SIZE];
	ssize_t bytes = recv(clientSock, buffer, BUF_SIZE - 1, 0);
	if (bytes > 0) {
		buffer[bytes] = '\0';
		send(clientSock, buffer, bytes, 0);

		string msg(buffer);
		string ip = inet_ntoa(clientAddr.sin_addr);
		sendToLog(ip, msg);
	}
	
	close(clientSock);
	exit(0);
}

void handleUDP(int udpSock) {
	char buffer[BUF_SIZE];
	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));

	socklen_t addrLen = (socklen_t)sizeof(clientAddr);
	ssize_t bytes = recvfrom(udpSock, buffer, BUF_SIZE - 1, 0,(struct sockaddr*)&clientAddr, &addrLen);

	if (bytes > 0) {
		buffer[bytes] = '\0';
		sendto(udpSock, buffer, bytes, 0, (struct sockaddr*)&clientAddr, addrLen); //CHANGE MADE HERE -------

		string msg(buffer);
		string ip = inet_ntoa(clientAddr.sin_addr);
		sendToLog(ip, msg);
	}
	exit(0);
}

int main(int argc, char* argv[]) {
	//Check if valid arguments have been passed
	if (argc < 2 || argc > MAX_PORTS + 1) {
		cerr << "Usage: " << argv[0] << " <port1> [<port2> <port3>]\n";
		return 1;
	}
	
	//Parse arguments and initalize sockets (Not Creation)
	vector<int> tcpSocks, udpSocks; //Holds 
	vector<int> ports;

	for (int i = 1; i < argc; ++i) {
		int port = atoi(argv[i]);
		ports.push_back(port);

		//TCP Creation
		int tcpSock = socket(AF_INET, SOCK_STREAM, 0); //AF_INET = IPv4
		if (tcpSock < 0) perror("TCP SOCKET FAIL");
		
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(static_cast<uint16_t>(port));

		//Bind TCP to address & begin listening
		if (bind(tcpSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			perror("bind TCP");
			return 1;
		}
		listen(tcpSock, BACKLOG);
		tcpSocks.push_back(tcpSock);


		//UDP Creation
		int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
		if (udpSock < 0) perror("UDP SOCKET FAIL");

		if (bind(udpSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			perror("bind UDP");
			return 1;
		}
		udpSocks.push_back(udpSock);
	}
	
	cout << "echo_s Server is running on ports: ";
	for (size_t i = 0; i < ports.size(); ++i) {
		cout << ports[i];
		if (i < ports.size() - 1) cout << ", ";
	}
	cout << "..." << endl;
	cout << "CTRL-Z to Suspend" << endl;

	while (true) {
		fd_set readFds;
		FD_ZERO(&readFds);
		int maxFd = -1;

		for (size_t i = 0; i < ports.size(); ++i) {
			FD_SET(tcpSocks[i], &readFds);
			FD_SET(udpSocks[i], &readFds);
			maxFd = max(max(maxFd, tcpSocks[i]), udpSocks[i]);
		}
		// Above code sets up select for monitoring sockets
			// Clear FDs
			// Add TCP & UDP
			// Track max socket value
		int activity = select(maxFd + 1, &readFds, NULL, NULL, NULL); //Waiting for activity
		if (activity < 0) continue; //Select success

		for (size_t i = 0; i < ports.size(); ++i) {
			//Handles TCP Socket Activity
			if (FD_ISSET(tcpSocks[i], &readFds)) {
				struct sockaddr_in clientAddr;
				memset(&clientAddr, 0, sizeof(clientAddr));
				socklen_t addrLen = (socklen_t)sizeof(clientAddr);

				int clientSock = accept(tcpSocks[i], (struct sockaddr*)&clientAddr, &addrLen);
				if (clientSock >= 0) {
					pid_t pid = fork();
					if (pid == 0) {
						for (size_t j = 0; j < tcpSocks.size(); ++j) close(tcpSocks[j]);
						for (size_t j = 0; j < udpSocks.size(); ++j) close(udpSocks[j]);
						handleTCP(clientSock, clientAddr);
					}
					close(clientSock);
				}
			}
			
			//handles UDP Socket Activity
			if (FD_ISSET(udpSocks[i], &readFds)) {
				pid_t pid = fork();
				if (pid == 0) {
					for (size_t j = 0; j < tcpSocks.size(); ++j)  close(tcpSocks[j]);
					for (size_t j = 0; j < udpSocks.size(); ++j) {
						if (j != i) close(udpSocks[j]);
					}
					handleUDP(udpSocks[i]);
				}
			}
		}
	}
	return 0;
}
