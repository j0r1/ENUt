#include "ipv4address.h"
#include "udpv4socket.h"
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <cmath>
#include <map>
#include <list>

#define real_t long double

real_t getCurrentTime()
{
	struct timeval tv;

	gettimeofday(&tv, 0);

	return (((real_t)tv.tv_sec)+((real_t)tv.tv_usec/1000000.0));
}

std::string getTimeString()
{
	char str[1024];
	time_t t = time(0);
	
	strcpy(str, ctime(&t));
	int len = strlen(str);

	for (int i = 0 ; i < len ; i++)
	{
		if (str[i] == '\n' || str[i] == '\r')
			str[i] = 0;
	}
	return std::string(str);
}

void checkError(bool ret, const errut::ErrorBase &obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj.getObjectName() << std::endl;
	std::cerr << "Error description: " << obj.getErrorString() << std::endl << std::endl;
	exit(-1);
}

void checkError(bool ret, const errut::ErrorBase *obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj->getObjectName() << std::endl;
	std::cerr << "Error description: " << obj->getErrorString() << std::endl << std::endl;
	exit(-1);
}

class Destination
{
public:
	Destination(uint32_t ip, uint16_t port) 				{ IP = ip; p = port; t = getCurrentTime(); }
	uint32_t GetIP() const							{ return IP; }
	uint16_t GetPort() const						{ return p; }
	real_t GetLastPacketTime() const					{ return t; }
	void SetLastPacketTime()						{ t = getCurrentTime(); }
private:
	uint16_t p;
	uint32_t IP;
	real_t t;
};

std::list<Destination> destinations;

void UpdateDestination(uint32_t ip, uint16_t port)
{
	std::list<Destination>::iterator it;

	for (it = destinations.begin() ; it != destinations.end() ; it++)
	{
		if ((*it).GetIP() == ip && (*it).GetPort() == port)
		{
			(*it).SetLastPacketTime();
			return;
		}
	}

	nut::IPv4Address addrStr(ip);

	std::cout << getTimeString() << " | Adding destination " << addrStr.getAddressString() << ":" << port << std::endl;
	destinations.push_back(Destination(ip,port));
}

void TimeoutDestinations(real_t timeout)
{
	std::list<Destination>::iterator it;
	real_t t = getCurrentTime();
	
	it = destinations.begin();
	while (it != destinations.end())
	{
		real_t diff = t - (*it).GetLastPacketTime();

		if (diff > timeout)
		{
			nut::IPv4Address addrStr((*it).GetIP());

			std::cout << getTimeString() << " | Deleting destination " << addrStr.getAddressString() << ":" << (*it).GetPort() << std::endl;
			it = destinations.erase(it);
		}
		else
			it++;
	}
}

int main(int argc, char *argv[])
{
	if (!(argc == 3 || argc == 4))
	{
		std::cout << "Usage: autoforwarder port timeout [bind_address]" << std::endl << std::endl;
		return -1;
	}
	
	uint16_t localPort = atoi(argv[1]);
	int delay = atoi(argv[2]);
	nut::IPv4Address bindAddress;

	if (argc == 4)
	{
		if (!bindAddress.setAddress(argv[3]))
		{
			std::cout << "Unable to set bind address to: " << argv[3] << std::endl;
			return -1;
		}
	}

	nut::UDPv4Socket *pSock = new nut::UDPv4Socket("Forwarding socket");	
	bool ret;

	std::cout << "Started on " << getTimeString() << std::endl;
	std::cout << "Bind address:  " << bindAddress.getAddressString() << std::endl;
	std::cout << "Bind port:     " << localPort << std::endl;
	std::cout << "Timeout delay: " << delay << " seconds" << std::endl;
	
	ret = pSock->create(bindAddress, localPort);
	checkError(ret, pSock);

	int bufSize = 2000000;

	ret = pSock->setSocketOption(SOL_SOCKET, SO_RCVBUF, (void *)&bufSize, sizeof(int));
	checkError(ret, pSock);

	ret = pSock->setSocketOption(SOL_SOCKET, SO_SNDBUF, (void *)&bufSize, sizeof(int));
	checkError(ret, pSock);

	while (1)
	{
		ret = pSock->waitForData(1,0);
		checkError(ret,pSock);

		size_t len = 0;
		bool avail = false;
		
		pSock->getAvailableDataLength(len,avail);

		while (avail)
		{
			uint8_t buf[65536];
			size_t packlen = 65535;
			
			ret = pSock->read(buf,packlen);
			checkError(ret,pSock);
			
			const nut::IPv4Address *pAddr = (const nut::IPv4Address *)pSock->getLastSourceAddress();
			uint32_t ip = pAddr->getAddress();
			uint16_t port = pSock->getLastSourcePort();

			UpdateDestination(ip,port);

			std::list<Destination>::const_iterator it;

			for (it = destinations.begin() ; it != destinations.end() ; it++)
			{
				if (!((*it).GetIP() == ip && (*it).GetPort() == port)) // don't send to sender of packet
				{
					size_t len2 = packlen;
					
					pSock->write(buf, len2, nut::IPv4Address((*it).GetIP()),(*it).GetPort());
				}
			}
	
			len = 0;
			avail = false;
			pSock->getAvailableDataLength(len,avail);
		}
		
		TimeoutDestinations(delay);
	}
	
	delete pSock;
	
	return 0;
}

