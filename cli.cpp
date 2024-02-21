#include "Neighbor.h"

#include <unistd.h> 
#include <netinet/in.h> 
#include <cstring>

void getActiveNeighbors(std::vector<Neighbor> &neighbors)
{
	int sockfd;
	char buffer[BUFFER];
	const char *hello = "Get Neighbors";
	struct sockaddr_in serverAddr;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr << "Socket Creation Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	int bytesReceived;
	socklen_t len;

	sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) & serverAddr, sizeof(serverAddr));	
	bytesReceived = recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &serverAddr, &len);
	buffer[bytesReceived] = '\0';

	std::string receivedData(buffer, bytesReceived);
	std::istringstream iss(receivedData);
	std::string line;
	while (std::getline(iss, line)) {
		if (!line.empty())
			neighbors.push_back(Neighbor::deserialize(line));
	}
	close(sockfd);
}

int main()
{
	std::vector<Neighbor> activeNeighbors;
	getActiveNeighbors(activeNeighbors);

	if (activeNeighbors.empty())
		std::cout << "Nav aktīvo kaimiņu" << std::endl;
	else {
		std::cout << "Aktīvie kaimiņi:" << std::endl;
		for (const auto & neighbor: activeNeighbors) {
			std::cout << neighbor << std::endl;
		}
	}
	return 0;
}
