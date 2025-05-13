#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024
#define LOG_PORT 9999
#define LOG_FILE "echo.log"

using namespace std;

int main() {
	int sock = socket(AF_INET, SOCK_DGRAM, 0); //log file is only UDP
	if (sock < 0) {
		perror("UDP Socket"); //Socket Fail
		return 1;
	}

	//Create server Address structure for Bind
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(LOG_PORT);

	//Call Bind
	if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Bind Fail");
		return 1;
	}
	
	//Test Print
	cout << "Log Server running on UDP port " << LOG_PORT << "...\n";

	//Print content to echo.log
	while (true) {
		char buffer[BUF_SIZE];
		struct sockaddr_in clientAddr;
		socklen_t addrLen = (socklen_t)sizeof(clientAddr);
		ssize_t bytes = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (struct sockaddr*)&clientAddr, &addrLen);

		if (bytes > 0) {
			buffer[bytes] = '\0';

			pid_t pid = fork();
			if (pid == 0) {
				ofstream logFile(LOG_FILE, ios::app); //Create the file & write to it if it opens 
				if (logFile.is_open()) {
					logFile << buffer << endl;
					logFile.close();
				}
				exit(0);
			}
		}
	}
	close(sock);
	return 0;
}	
