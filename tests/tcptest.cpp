#include "tcpv4socket.h"
#include "ipv4address.h"
#include <iostream>

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
	nut::TCPv4Socket sock("Server socket");
	nut::TCPv4Socket sock2("Client socket");
	bool ret;
	
	ret = sock.create();
	checkError(ret, sock);

	ret = sock.listen(0);
	checkError(ret, sock);

	nut::IPv4Address addr(127,0,0,1);

	nut::TCPSocket *s;

	ret = sock2.create();
	checkError(ret, sock2);
	
	ret = sock2.connect(addr,sock.getLocalPortNumber());
	checkError(ret, sock2);

	ret = sock.accept(&s);
	checkError(ret, sock2);

	size_t len = 4;
	char str[4];
	ret = s->write("bla",len);
	checkError(ret, *s);

	len = 4;
	ret = sock2.read(str,len);
	checkError(ret, sock2);

	std::cout << str << std::endl;
	
	return 0;
}

