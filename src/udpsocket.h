#ifndef NUT_UDPSOCKET_H

#define NUT_UDPSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "datagramsocket.h"
#include "networklayeraddress.h"

namespace nut
{

class UDPSocket : public DatagramSocket
{
protected:
	UDPSocket(NetworkLayerProtocol proto)						{ m_protocol = proto; }
	UDPSocket(const std::string &objName, NetworkLayerProtocol proto) : DatagramSocket(objName)
											{ m_protocol = proto; }
public:
	virtual ~UDPSocket()								{ }
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }

	// by default blocking mode should be used
	virtual bool create(uint16_t portNumber = 0, bool obtainDestination = false) = 0;
	virtual bool create(NetworkLayerAddress &bindAddress, uint16_t portNumber = 0, bool obtainDestination = false) = 0;
	virtual bool destroy() = 0;
	virtual bool setNonBlocking(bool f = true) = 0;
	virtual uint16_t getLocalPortNumber() = 0;
	virtual bool setMulticastTTL(uint8_t ttl) = 0;
private:
	NetworkLayerProtocol m_protocol;
};
	
}

#endif // NUT_UDPSOCKET_H
