#include "Neighbor.h"

Neighbor::Neighbor(const std::string &ipAddress, const std::string &macAddress, std::time_t time)
	: _ipAddress(ipAddress),
	  _macAddress(macAddress),
	  _time(time) {}

Neighbor::Neighbor(const Neighbor &other)
	: _ipAddress(other._ipAddress),
	  _macAddress(other._macAddress),
	  _time(other._time) {}

Neighbor &Neighbor::operator=(const Neighbor &other)
{
	if (this != &other)
	{
		_ipAddress = other._ipAddress;
		_macAddress = other._macAddress;
		_time = other._time;
	}
	return *this;
}

Neighbor::~Neighbor() {}

std::string Neighbor::serialize() const
{
	std::stringstream ss;
	ss << "{";
	ss << "\"ipAddress\": \"" << _ipAddress << "\", ";
	ss << "\"macAddress\": \"" << _macAddress << "\", ";
	ss << "\"lastActiveTime\": " << _time;
	ss << "}";
	return ss.str();
}

Neighbor Neighbor::deserialize(const std::string &data)
{
	std::string ipAddress;
	std::string macAddress;
	std::time_t time = 0;

	size_t pos = data.find("\"ipAddress\": \"");
	if (pos != std::string::npos)
	{
		pos += 14;
		size_t end_pos = data.find('\"', pos);
		ipAddress = data.substr(pos, end_pos - pos);
	}

	pos = data.find("\"macAddress\": \"");
	if (pos != std::string::npos)
	{
		pos += 15;
		size_t end_pos = data.find('\"', pos);
		macAddress = data.substr(pos, end_pos - pos);
	}

	pos = data.find("\"lastActiveTime\": ");
	if (pos != std::string::npos)
	{
		pos += 19;
		time = std::stoll(data.substr(pos));
	}

	return Neighbor(ipAddress, macAddress, time);
}

std::ostream & operator<<(std::ostream & os, const Neighbor & neighbor)
{
	os << "IP adrese: " << neighbor.getIpAddress() << " MAC adrese: " << neighbor.getMacAddress();
	return os;
}
