#ifndef NUT_NETWORKLAYERADDRESS_H

#define NUT_NETWORKLAYERADDRESS_H

#include "nutconfig.h"
#include <string>

namespace nut
{
	
enum NetworkLayerProtocol { IPv4, IPv6 };
	
class NetworkLayerAddress
{
protected:
	NetworkLayerAddress(NetworkLayerProtocol p)					{ m_protocol = p; }
public:
	virtual ~NetworkLayerAddress()							{ }
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }	
		
	virtual NetworkLayerAddress *createCopy() const = 0;
	virtual bool isSameAddress(const NetworkLayerAddress &address) const = 0;
	virtual std::string getAddressString() const = 0;
private:
	NetworkLayerProtocol m_protocol;
};
	
}

#endif // NUT_NETWORKLAYERADDRESS_H

