#include "Neighbor.h"

#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <thread>

struct CurrentDevice
{
	std::string ipAddress;
	std::string subnetMask;
	std::string broadcastAddress;
};

CurrentDevice getIpAddressAndSubnet()
{
	CurrentDevice ipAndSubnet;
	std::string command = "ifconfig | grep 'inet ' | grep -v '127.0.0.1'";
	FILE *ifconfig_pipe = popen(command.c_str(), "r");
	if (!ifconfig_pipe) {
		std::cerr << "Failed to execute ifconfig command" << std::endl;
		return ipAndSubnet;
	}

	char buffer[256];
	std::string ifconfig_output;
	while (fgets(buffer, sizeof(buffer), ifconfig_pipe) != nullptr) {
		ifconfig_output += buffer;
	}
	pclose(ifconfig_pipe);

	size_t pos = ifconfig_output.find("inet ");
	if (pos != std::string::npos) {
		pos += 5;
		size_t end_pos = ifconfig_output.find(' ', pos);
		ipAndSubnet.ipAddress = ifconfig_output.substr(pos, end_pos - pos);

		pos = ifconfig_output.find("netmask ", end_pos);
		if (pos != std::string::npos) {
			pos += 8;
			end_pos = ifconfig_output.find(' ', pos);
			ipAndSubnet.subnetMask = ifconfig_output.substr(pos, end_pos - pos);

			pos = ifconfig_output.find("broadcast ", end_pos);
			if (pos != std::string::npos) {
				pos += 10;
				end_pos = ifconfig_output.find(' ', pos);
				ipAndSubnet.broadcastAddress = ifconfig_output.substr(pos, end_pos - pos);
			}
		}
	}
	return ipAndSubnet;
}

void sendBroadcast(std::string & broadcastIp)
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
	serverAddr.sin_addr.s_addr = inet_addr(broadcastIp.c_str());

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

void sendUdpPacket(const std::string &ip)
{
	int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket < 0) {
		std::cerr << "Failed to create UDP socket" << std::endl;
		return;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

	const char *message = "UDP Discovery Packet";
	if (sendto(udpSocket, message, strlen(message), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Failed to send UDP packet" << std::endl;
		close(udpSocket);
		return;
	}
	close(udpSocket);
}

std::string getMacAddress(std::string ip)
{
	std::string macAddress;
	std::string command = "arp -a";
	FILE *arpPipe = popen(command.c_str(), "r");
	if (!arpPipe) {
		std::cerr << "Failed to execute arp -a command" << std::endl;
		return "";
	}

	char buffer[512];
	std::string arpOutput;
	while (fgets(buffer, sizeof(buffer), arpPipe) != nullptr) {
		arpOutput += buffer;
	}
	pclose(arpPipe);
	size_t pos = arpOutput.find(ip);
	if (pos != std::string::npos) {
		pos += ip.size() + 6;
		macAddress = arpOutput.substr(pos, 17);
	} else {
		sendUdpPacket(ip);
		return getMacAddress(ip);
	}
	return macAddress;
}

void receiveMessages(int sockfd, CurrentDevice ipAndSubnet, std::map<std::string, Neighbor>& activeNeighbors) {
	char buffer[BUFFER];
	int bytesReceived;
	struct sockaddr_in cliAddr;
	socklen_t len = sizeof(cliAddr);
	
	memset(&cliAddr, 0, sizeof(cliAddr));

	bytesReceived = recvfrom(sockfd, (char *) buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &cliAddr, &len);
	buffer[bytesReceived] = '\0';
	if (bytesReceived < 0) {
		std::cerr << "Receiving message failed" << std::endl;
		close(sockfd);
		return ;
	}
	if (strcmp(buffer, "Get Neighbors") == 0) {
		std::string serializedData;
		for (const auto & neighbor : activeNeighbors) {
			serializedData += neighbor.second.serialize();
			serializedData += '\n';
		}
		sendto(sockfd, serializedData.c_str(), serializedData.size(), 0, (const struct sockaddr *) & cliAddr, len);
	} else {
		std::string currentIp = inet_ntoa(cliAddr.sin_addr);
		if (currentIp != ipAndSubnet.ipAddress) {
			auto it = activeNeighbors.find(currentIp);
			if (it == activeNeighbors.end()) {
				std::string macAddress = getMacAddress(currentIp);
				activeNeighbors.emplace(currentIp, Neighbor(currentIp, macAddress, std::time(nullptr)));
			} else {
				it->second.setTime(std::time(nullptr));
			}
		}
	}
}

int main()
{
	std::map<std::string, Neighbor> activeNeighbors;

	int sockfd;
	struct sockaddr_in serverAddr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr << "Socket Creation Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(PORT);

	if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Bind Failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	CurrentDevice ipAndSubnet = getIpAddressAndSubnet();
	while (true) {
		sendBroadcast(ipAndSubnet.broadcastAddress);
		receiveMessages(sockfd, ipAndSubnet, activeNeighbors);
		auto currentTime = std::time(nullptr);
		for (auto it = activeNeighbors.begin(); it != activeNeighbors.end();) {
			if (currentTime - it->second.getTime() > 30) {
				it = activeNeighbors.erase(it);
			} else
			++it;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return 0;
}
