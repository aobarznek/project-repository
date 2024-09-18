#pragma once
#include<string>
class ConfReader
{
public:
	int port;
	char addrIp[20];
	//std::string addrIp;
	void load();
	ConfReader();
};
