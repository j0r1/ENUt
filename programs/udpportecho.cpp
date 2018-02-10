#include "ipv4address.h"
#include "udpv4socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>

void checkError(bool ret, const nut::ErrorBase *obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj->getObjectName() << std::endl;
	std::cerr << "Error description: " << obj->getErrorString() << std::endl << std::endl;
	exit(-1);
}

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << std::string(argv[0]) << " localport [bindIP] " << std::endl;
		return -1;
	}

	uint32_t bindIP = 0;
	uint16_t localPort = atoi(argv[1]);
	nut::UDPv4Socket *sock = new nut::UDPv4Socket("UDP socket");	
	bool ret;

	if (argc == 3)
		bindIP = ntohl(inet_addr(argv[2]));
	
	nut::IPv4Address bindAddr(bindIP);
	
	ret = sock->create(bindAddr,localPort);
	checkError(ret, sock);

	while(1)
	{
		bool done = false;
		
		ret = sock->waitForData();
		checkError(ret, sock);
		
		while (!done)
		{
			size_t length = 0;
			bool avail = false;
			
			ret = sock->getAvailableDataLength(length,avail);
			checkError(ret, sock);

			if (!avail)
				done = true;
			else if (length == 6) // client should send 6 zero bytes
			{
				uint8_t pData[6];
				const nut::IPv4Address *addr;
				uint32_t ip;
				uint16_t port;
				
				ret = sock->read(pData, length);
				addr = (const nut::IPv4Address *)sock->getLastSourceAddress();
				port = sock->getLastSourcePort();
				checkError(ret, sock);
				
				ip = addr->getAddress();
				
				std::cout << "Got packet from " << addr->getAddressString() << ":" << port << std::endl;
				
				if (pData[0] == 0 && pData[1] == 0 && pData[2] == 0 && pData[3] == 0 && pData[4] == 0 && pData[5] == 0)
				{
					pData[0] = (uint8_t)((ip >> 24)&0xff);
					pData[1] = (uint8_t)((ip >> 16)&0xff);
					pData[2] = (uint8_t)((ip >> 8)&0xff);
					pData[3] = (uint8_t)(ip&0xff);
					pData[4] = (uint8_t)((port >> 8)&0xff);
					pData[5] = (uint8_t)(port&0xff);
					
					sock->write(pData,length,*addr,port);
				}

			}
			else // ignore packet
			{
				uint8_t *pData = new uint8_t [length+1]; // avoid length = 0 case
				
				ret = sock->read(pData, length);
				checkError(ret, sock);
				
				delete [] pData;
			}
		}
	}
	
	return 0;
}

