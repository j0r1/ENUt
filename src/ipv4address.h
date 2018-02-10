#ifndef NUT_IPV4ADDRESS_H

#define NUT_IPV4ADDRESS_H

#include "nutconfig.h"
#include "networklayeraddress.h"
#include "nuttypes.h"

#if !(defined(WIN32) || defined(_WIN32_WCE))
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif // !(WIN32 || _WIN32_WCE)

namespace nut
{

class IPv4Address : public NetworkLayerAddress
{
public:
	IPv4Address(uint8_t ip[4]) : NetworkLayerAddress(IPv4)			{ m_ip = ((uint32_t)ip[3]) | (((uint32_t)ip[2]) << 8) | (((uint32_t)ip[1]) << 16) | (((uint32_t)ip[0]) << 24); }
	IPv4Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) : NetworkLayerAddress(IPv4)
										{ m_ip = ((uint32_t)ip3) | (((uint32_t)ip2) << 8) | (((uint32_t)ip1) << 16) | (((uint32_t)ip0) << 24); }
	IPv4Address(uint32_t ip = 0, bool hostByteOrder = true) : NetworkLayerAddress(IPv4)		
										{ if (hostByteOrder) m_ip = ip; else m_ip = ntohl(ip); }
	~IPv4Address()								{ }
	uint32_t getAddress() const						{ return m_ip; }
	uint32_t getAddressNBO() const						{ return htonl(m_ip); }

	bool setAddress(const std::string &addressString);

	NetworkLayerAddress *createCopy() const					{ return new IPv4Address(m_ip); }
	bool isSameAddress(const NetworkLayerAddress &address) const;
	std::string getAddressString() const;
private:
	uint32_t m_ip;
};

inline bool IPv4Address::isSameAddress(const NetworkLayerAddress &address) const
{
	if (address.getProtocol() != IPv4)
		return false;
	
	const IPv4Address &addr = (const IPv4Address &)address;

	if (addr.m_ip != m_ip)
		return false;
	return true;
}

inline std::string IPv4Address::getAddressString() const
{
	char str[16];

#if (defined(WIN32) || defined(_WIN32_WCE))
	_snprintf(str, 16, "%d.%d.%d.%d", (int)((m_ip >> 24)&0xff), (int)((m_ip >> 16)&0xff), (int)((m_ip >> 8)&0xff), (int)(m_ip&0xff));
#else
	snprintf(str, 16, "%d.%d.%d.%d", (int)((m_ip >> 24)&0xff), (int)((m_ip >> 16)&0xff), (int)((m_ip >> 8)&0xff), (int)(m_ip&0xff));
#endif // WIN32 || _WIN32_WCE
	return std::string(str);
}

inline bool IPv4Address::setAddress(const std::string &addressString)
{
	uint32_t ip;

	ip = inet_addr(addressString.c_str());
	if (ip == INADDR_NONE)
		return false;
	m_ip = ntohl(ip);
	return true;
}
	
} // end namespace

#endif // NUT_IPV4ADDRESS_H

