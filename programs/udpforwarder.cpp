#include "udpv4socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace nut;

#define BUFSIZE 66000

void usage()
{
	std::cerr << std::endl;
	std::cerr << "Usage:" << std::endl;
	std::cerr << "udpforwarder [bindaddress:]bindport forwardport1 [forwardport2 ...]" << std::endl;
	std::cerr << std::endl;
	exit(-1);
}

void checkValue(int x, int min, int max)
{
	if (x < min || x > max)
		usage();
}

bool checkMulticast(const IPv4Address &addr)
{
	uint32_t x = addr.getAddress();

	if (((x)&0xF0000000) == 0xE0000000)
		return true;
	return false;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
		usage();

	IPv4Address bindAddress(0,0,0,0);
	uint16_t bindPort = 0;
	bool isMulticast = false;

	// check bind address and port
	{
		int ip0, ip1, ip2, ip3;
		int port;

		if (sscanf(argv[1], "%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port) == 5)
		{
			checkValue(ip0, 0, 255);
			checkValue(ip1, 0, 255);
			checkValue(ip2, 0, 255);
			checkValue(ip3, 0, 255);
			checkValue(port, 0, 65535);

			bindAddress = IPv4Address((uint8_t)ip0, (uint8_t)ip1, (uint8_t)ip2, (uint8_t)ip3);
			isMulticast = checkMulticast(bindAddress);
			bindPort = (uint16_t)port;
		}
		else if (sscanf(argv[1], "%d", &port) == 1)
		{
			checkValue(port, 0, 65535);
			bindPort = (uint16_t)port;
		}
		else
			usage();
	}

	std::vector<uint16_t> destPorts;

	for (int i = 2 ; i < argc ; i++)
	{
		int port;

		if (sscanf(argv[i], "%d", &port) == 1)
		{
			checkValue(port, 0, 65535);
			destPorts.push_back((uint16_t)port);
		}
		else
			usage();
	}

	UDPv4Socket sock;

	if (!sock.create(bindAddress, bindPort, false))
	{
		std::cerr << "Couldn't create UDP socket on port " << bindPort << " and bound to address " << bindAddress.getAddressString() << ":" << std::endl;
		std::cerr << sock.getErrorString() << std::endl;
		return -1;
	}

	std::cout << "Created UDP socket on port " << bindPort << " and bound to address " << bindAddress.getAddressString() << std::endl;

	int value = 1;

#ifdef SO_REUSEPORT 
	if (!sock.setSocketOption(SOL_SOCKET, SO_REUSEPORT, &value, sizeof(int)))
	{
		std::cerr << "Couldn't set SO_REUSEPORT on UDP socket:" << std::endl;
		std::cerr << sock.getErrorString() << std::endl;
		return -1;
	}
	
	std::cout << "Succesfully set SO_REUSEPORT on socket" << std::endl;
#else
	std::cout << "WARNING: SO_REUSEPORT not available on current platform" << std::endl;
#endif // SO_REUSEPORT

	if (isMulticast)
	{
		if (!sock.joinMulticastGroup(bindAddress))
		{
			std::cerr << "Couldn't join specified multicast address:" << std::endl;
			std::cerr << sock.getErrorString() << std::endl;
			return -1;
		}
		std::cout << "Joined specified multicast group" << std::endl;
	}

	bool done = false;
	uint16_t buffer[BUFSIZE];
	IPv4Address localAddress(127,0,0,1);

	std::cout << "Waiting for incoming packets" << std::endl;

	while (!done)
	{
		size_t numBytes = BUFSIZE;

		if (sock.read(buffer, numBytes))
		{
	//		std::cout << "Read " << numBytes << std::endl;

			for (int i = 0 ; i < destPorts.size() ; i++)
			{
				size_t s = numBytes;

				sock.write(buffer, s, localAddress, destPorts[i]);
			}
		}
		else
		{
			std::cerr << "Error reading from socket:" << std::endl;
			std::cerr << sock.getErrorString() << std::endl;
			return -1;
		}
	}

	return 0;
}
