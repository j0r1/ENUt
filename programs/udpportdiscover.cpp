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
	if (argc != 3)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << std::string(argv[0]) << " IP port" << std::endl;
		return -1;
	}
	
	uint32_t IP = ntohl(inet_addr(argv[1]));
	uint16_t destPort = atoi(argv[2]);
	
	nut::NetworkLayerAddress *destAddr = new nut::IPv4Address(IP);
	nut::UDPSocket *sock = new nut::UDPv4Socket("UDP socket");	
	time_t prevsendtime = 0;
	bool ret;

	ret = sock->create();
	checkError(ret, sock);

	uint16_t localPort = sock->getLocalPortNumber();
	std::cout << "Local port: " << localPort << std::endl;
	
	while(1)
	{
		bool done = false;
		time_t curtime = time(0);

		if (curtime - prevsendtime >= 5)
		{
			uint8_t data[6];
			size_t length = 6;
			
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
			data[3] = 0;
			data[4] = 0;
			data[5] = 0;

			ret = sock->write(data, length, *destAddr, destPort);
			checkError(ret, sock);
			prevsendtime = curtime;
		}
		
		ret = sock->waitForData(1,0);
		checkError(ret, sock);
		
		while (!done)
		{
			size_t length = 0;
			bool avail = false;
			
			ret = sock->getAvailableDataLength(length,avail);
			checkError(ret, sock);

			if (!avail)
				done = true;
			else if (length == 6) // server sends 6 bytes
			{
				uint8_t pData[6];
				const nut::NetworkLayerAddress *addr;
				uint32_t ip;
				uint16_t port;
				
				ret = sock->read(pData, length);
				addr = sock->getLastSourceAddress();
				port = sock->getLastSourcePort();
				checkError(ret, sock);
				
				if (addr->isSameAddress(*destAddr) && port == destPort)
				{
					ip = (((uint32_t)pData[0]) << 24) | (((uint32_t)pData[1]) << 16) | (((uint32_t)pData[2]) << 8) | ((uint32_t)pData[3]);
					port = (((uint16_t)pData[4]) << 8) | ((uint16_t)pData[5]);

					nut::IPv4Address a(ip);
					std::cout << "Server detected " << a.getAddressString() << ":" << port << std::endl;
				}

				delete addr;
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

