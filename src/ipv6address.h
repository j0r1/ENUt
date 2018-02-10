#ifndef NUT_IPV6ADDRESS_H

#define NUT_IPV6ADDRESS_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "networklayeraddress.h"
#include <netinet/in.h>

namespace nut
{

class IPv6Address : public NetworkLayerAddress
{
public:
	IPv6Address() : NetworkLayerAddress(IPv6)						{ for (int i = 0 ; i < 16 ; i++) m_ip.s6_addr[i] = 0; }
	IPv6Address(in6_addr ip) : NetworkLayerAddress(IPv6)					{ ip = m_ip; }
	IPv6Address(const uint8_t ip[16]) : NetworkLayerAddress(IPv6)				{ for (int i = 0 ; i < 16 ; i++) m_ip.s6_addr[i] = ip[i]; }
	~IPv6Address()										{ }	
	in6_addr getAddress() const								{ return m_ip; }
	NetworkLayerAddress *createCopy() const							{ return new IPv6Address(m_ip); }
	bool isSameAddress(const NetworkLayerAddress &address) const;
	std::string getAddressString() const;
private:
	in6_addr m_ip;
};

inline bool IPv6Address::isSameAddress(const NetworkLayerAddress &address) const
{
	if (address.getProtocol() != IPv6)
		return false;
	
	const IPv6Address &addr = (const IPv6Address &)address;

	for (int i = 0 ; i < 16 ; i++)
	{
		if (m_ip.s6_addr[i] != addr.m_ip.s6_addr[i])
			return false;
	}
	return true;
}

inline std::string IPv6Address::getAddressString() const
{
	char str[48];
	uint16_t ip16[8];
	int i,j;

	for (i = 0,j = 0 ; j < 8 ; j++,i += 2)
	{
		ip16[j] = (((uint16_t)m_ip.s6_addr[i])<<8);
		ip16[j] |= ((uint16_t)m_ip.s6_addr[i+1]);
	}

        snprintf(str,48,"%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X",(int)ip16[0],(int)ip16[1],(int)ip16[2],(int)ip16[3],
	                                                          (int)ip16[4],(int)ip16[5],(int)ip16[6],(int)ip16[7]);
	return std::string(str);
}

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_IPV6ADDRESS_H

