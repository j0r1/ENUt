#include "udpv4socket.h"
#include <iostream>

using namespace nut;

int main(void)
{
	UDPv4Socket s;

	s.create(0,true);

	IPv4Address addr(235,6,9,10);
	size_t sz = 0;
	
	s.joinMulticastGroup(addr);
	s.write("",sz,addr,s.getLocalPortNumber());
	sleep(1);

	size_t len;
	bool avail;

	s.getAvailableDataLength(len, avail);
	
	std::cout << len << " " << ((avail)?"yes":"no") << std::endl;
	
	uint8_t bla[10];
	len = 10;
	if (s.read(bla,len))
	{
		std::cout << s.getLastDestinationAddress()->getAddressString() << std::endl;
		std::cout << len << std::endl;
	}
	return 0;
}
