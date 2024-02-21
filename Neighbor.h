#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <sstream>

#ifndef PORT
#define PORT 8888
#endif

#ifndef BUFFER
#define BUFFER 1024
#endif

class Neighbor
{
private:
	std::string _ipAddress;
	std::string _macAddress;
	std::time_t _time;

public:
	Neighbor(const std::string &ipAddress, const std::string &macAddress, std::time_t time);
	Neighbor(const Neighbor &other);
	Neighbor &operator=(const Neighbor &other);
	~Neighbor();

	const std::string &getIpAddress() const { return _ipAddress; };
	const std::string &getMacAddress() const { return _macAddress; };
	const std::time_t &getTime() const { return _time; };

	void setTime(const std::time_t time) { this->_time = time; };

	std::string serialize() const;
	static Neighbor deserialize(const std::string &data);
};

std::ostream & operator<<(std::ostream & os, const Neighbor & neighbor);

#endif
