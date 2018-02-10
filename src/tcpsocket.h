#ifndef NUT_TCPSOCKET_H

#define NUT_TCPSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "socket.h"
#include "networklayeraddress.h"

namespace nut
{

class TCPSocket : public Socket
{
protected:
	TCPSocket(NetworkLayerProtocol proto)						{ m_protocol = proto; }
	TCPSocket(const std::string &objName, NetworkLayerProtocol proto) : Socket(objName)
											{ m_protocol = proto; }
public:
	virtual ~TCPSocket()								{ }
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }

	// by default blocking mode should be used
	virtual bool create(uint16_t portNumber = 0) = 0;
	virtual bool create(const NetworkLayerAddress &bindAddress, uint16_t portNumber = 0) = 0;
	virtual bool destroy() = 0;

	virtual bool setNonBlocking(bool f = true) = 0;
	
	virtual uint16_t getLocalPortNumber() = 0;

	virtual bool connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber) = 0;
	virtual bool listen(int backlog) = 0;
	virtual bool accept(TCPSocket **newsock) = 0;
	virtual bool isConnected() = 0;
	
	virtual NetworkLayerAddress *getDestinationAddress() = 0;
	virtual uint16_t getDestinationPort() = 0;
	
	virtual bool waitForData(int seconds = -1, int microSeconds = -1) = 0;
	virtual bool write(const void *data, size_t &length) = 0;
	virtual bool getAvailableDataLength(size_t &length) = 0;
	virtual bool read(void *buffer, size_t &bufferSize) = 0;
private:
	NetworkLayerProtocol m_protocol;
};

}

#endif // NUT_TCPSOCKET_H

