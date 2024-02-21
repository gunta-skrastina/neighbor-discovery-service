#include "Neighbor.h"

#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>

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

	socklen_t len;
	int n;

	len = sizeof(cliAddr);
	n = recvfrom(sockfd, (char *) buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &cliAddr, &len);
	buffer[n] = '\0';
	if (strcmp(buffer, "Get Neighbors") == 0) {
		std::string serializedData;
		for (const auto & neighbor : activeNeighbors) {
			serializedData += neighbor.second.serialize();
			serializedData += '\n';
		}
		sendto(sockfd, serializedData.c_str(), serializedData.size(), 0, (const struct sockaddr *) & cliAddr, len);
	}
	return 0;
}
