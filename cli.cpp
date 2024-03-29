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
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	int bytesReceived;
	socklen_t len;

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
		std::cerr << "Error setting socket receive timeout" << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
	}

	if (sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) & serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Receiving message failed" << std::endl;
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	bytesReceived = recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &serverAddr, &len);
	buffer[bytesReceived] = '\0';
	if (bytesReceived < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			std::cerr << "Timeout: Service is not running or available" << std::endl;
		} else {
			std::cerr << "Receiving message failed" << std::endl;
		}
		close(sockfd);
		exit(EXIT_FAILURE);
	}

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
