#ifndef NUT_SOCKETWAITER_H

#define NUT_SOCKETWAITER_H

#include "nutconfig.h"
#include "errorbase.h"
#include <list>

namespace nut
{

class Socket;
	
class SocketWaiter : public ErrorBase
{
public:
	SocketWaiter();
	~SocketWaiter();
	void addSocket(Socket &s)								{ m_sockets.push_back(&s); }
	void clear()										{ m_sockets.clear(); }
	bool wait(int seconds = -1, int microseconds = -1);
private:
	std::string getSocketErrorString();
	std::list<Socket *> m_sockets;
};
	
} // end namespace

#endif // NUT_SOCKETWAITER_H

