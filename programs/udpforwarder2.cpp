#include "udpv4socket.h"
//#include <sys/socket.h>
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
	std::cerr << "udpforwarder2 bindport forwardIP forwardport" << std::endl;
	std::cerr << std::endl;
	exit(-1);
}

void checkValue(int x, int min, int max)
{
	if (x < min || x > max)
		usage();
}

int main(int argc, char *argv[])
{
	if (argc != 4)
		usage();

	IPv4Address bindAddress(0,0,0,0);
	IPv4Address destAddress(0,0,0,0);
	uint16_t bindPort = 0;
	uint16_t destPort = 0;

	// check bind port, dest IP address and dest port
	{
		int ip0, ip1, ip2, ip3;
		int port;

		if (sscanf(argv[1], "%d", &port) == 1)
		{
			checkValue(port, 0, 65535);
			bindPort = (uint16_t)port;
		}
		else
			usage();
		

		if (sscanf(argv[2], "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3) == 4)
		{
			checkValue(ip0, 0, 255);
			checkValue(ip1, 0, 255);
			checkValue(ip2, 0, 255);
			checkValue(ip3, 0, 255);

			destAddress = IPv4Address((uint8_t)ip0, (uint8_t)ip1, (uint8_t)ip2, (uint8_t)ip3);
		}
		else
			usage();

		if (sscanf(argv[3], "%d", &port) == 1)
		{
			checkValue(port, 0, 65535);
			destPort = (uint16_t)port;
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

	int val = 1024*1024;

	sock.setSocketOption(SOL_SOCKET, SO_SNDBUF, &val, sizeof(int));
	val = 1024*1024;
	sock.setSocketOption(SOL_SOCKET, SO_RCVBUF, &val, sizeof(int));

	std::cout << "Created UDP socket on port " << bindPort << " and bound to address " << bindAddress.getAddressString() << std::endl;

	bool done = false;
	uint16_t buffer[BUFSIZE];

	std::cout << "Waiting for incoming packets" << std::endl;

	while (!done)
	{
		size_t numBytes = BUFSIZE;

		if (sock.read(buffer, numBytes))
		{
			size_t s = numBytes;

			sock.write(buffer, s, destAddress, destPort);
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
