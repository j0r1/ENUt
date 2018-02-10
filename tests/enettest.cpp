#include "enetsocket.h"
#include "ipv4address.h"
#include <iostream>
#include <string>

void checkError(bool ret, const nut::ErrorBase &obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj.getObjectName() << std::endl;
	std::cerr << "Error description: " << obj.getErrorString() << std::endl << std::endl;
	exit(-1);
}

int main(void)
{
	nut::ENETSocket sock;
	nut::ENETSocket sock2;
	bool ret;
	
	nut::ENETSocket::ENETStartup();
	
	ret = sock.createClient(1);
	checkError(ret, sock);

	ret = sock2.createServer(10,(uint16_t)4444);
	checkError(ret, sock2);
	
	sock.requestConnection(nut::IPv4Address(127,0,0,1),4444,255);

	std::string str;
	std::cin >> str;
		
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	sock.poll();
	sock2.poll();
	std::cin >> str;
	
	int numIDs = sock2.getNumberOfConnections();
	const uint32_t *connIDs = sock2.getConnectionIDs();

	for (int i = 0 ; i < numIDs ; i++)
	{
		nut::IPv4Address addr;
		uint16_t port;

		ret = sock2.getConnectionInfo(connIDs[i], addr, port);
		checkError(ret, sock2);

		std::cout << addr.getAddressString() << ":" << port << std::endl;
	}
	
	std::cin >> str;
	
	nut::ENETSocket::ENETCleanup();

	return 0;
}

