#include "Neighbor.h"

#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>

void sendBroadcast()
{
	int sockfd;
	const char *hello = "Hello";
	struct sockaddr_in serverAddr;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr << "Socket Creation Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_BROADCAST;

	int broadcastEnabled = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnabled, sizeof(broadcastEnabled)) < 0) {
		std::cerr << "Setting socket option to enable broadcast failed" << std::endl;
		close(sockfd);
		return;
	}

	if (sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) & serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Sending Broadcast Failed" << std::endl;
	}	
	close(sockfd);
}

int main()
{
	std::map<std::string, Neighbor> activeNeighbors;

	int sockfd;
	char buffer[BUFFER];
	struct sockaddr_in serverAddr, cliAddr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr << "Socket Creation Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	memset(&cliAddr, 0, sizeof(cliAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(PORT);

	if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Bind Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	sendBroadcast();

	socklen_t len;
	int bytesReceived;

	len = sizeof(cliAddr);
	while (true) {
		bytesReceived = recvfrom(sockfd, (char *) buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &cliAddr, &len);
		buffer[bytesReceived] = '\0';
		if (strcmp(buffer, "Get Neighbors") == 0) {
			std::string serializedData;
			for (const auto & neighbor : activeNeighbors) {
				serializedData += neighbor.second.serialize();
				serializedData += '\n';
			}
			sendto(sockfd, serializedData.c_str(), serializedData.size(), 0, (const struct sockaddr *) & cliAddr, len);
		} else {
			std::string currentIp = inet_ntoa(cliAddr.sin_addr);
			std::cout << "Received from " << currentIp << std::endl;
		}
	}
	return 0;
}
