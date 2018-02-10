#ifndef NUT_DATAGRAMSOCKET_H

#define NUT_DATAGRAMSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "socket.h"

namespace nut
{

class NetworkLayerAddress;

class DatagramSocket : public Socket
{
protected:
	DatagramSocket()										{ }
	DatagramSocket(const std::string &objName) : Socket(objName)					{ }
public:
	~DatagramSocket()										{ }
	
	virtual bool joinMulticastGroup(const NetworkLayerAddress &groupAddress) = 0;
	virtual bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress) = 0;
	
	virtual bool waitForData(int seconds = -1, int microSeconds = -1) = 0;
	virtual bool write(const void *data, size_t &length, 
	                   const NetworkLayerAddress &destinationAddress, 
			   uint16_t destinationPort) = 0;
	virtual bool getAvailableDataLength(size_t &length, bool &available) = 0;
	virtual bool read(void *buffer, size_t &bufferSize) = 0;
	virtual const NetworkLayerAddress *getLastSourceAddress() = 0;
	virtual uint16_t getLastSourcePort() = 0;

	// should return NULL if not supported
	virtual const NetworkLayerAddress *getLastDestinationAddress() = 0;
};
	
} // end namespace

#endif // NUT_DATAGRAMSOCKET_H
