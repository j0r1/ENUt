#include "ipv4address.h"
#include "udpv4socket.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <jthread.h>
#include <cmath>
#include <map>
#include <list>

#define real_t long double

real_t getCurrentTime()
{
	struct timeval tv;

	gettimeofday(&tv, 0);

	return (((real_t)tv.tv_sec)+((real_t)tv.tv_usec/1000000.0));
}

void checkError(bool ret, const nut::ErrorBase &obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj.getObjectName() << std::endl;
	std::cerr << "Error description: " << obj.getErrorString() << std::endl << std::endl;
	exit(-1);
}

void checkError(bool ret, const nut::ErrorBase *obj)
{
	if (ret)
		return;
	
	std::cerr << "Error in object: " << obj->getObjectName() << std::endl;
	std::cerr << "Error description: " << obj->getErrorString() << std::endl << std::endl;
	exit(-1);
}

bool acceptPacket(real_t lossPct)
{
	double x = drand48();
	double y = (double)(lossPct/100.0);

	if (x < y)
		return false;

	return true;
}

inline double pickExponential(double lambda)
{
	double r = drand48();
	double x = -lambda * std::log(r);
	return x;
}

inline double pickGaussian(double width)
{
	double rho = pickExponential(1.0);
	double theta = drand48()*2.0*3.1415;
	double x = std::sqrt(2.0*rho) * std::cos(theta);
	return x*width;
}

inline real_t getSendTime(real_t delay, real_t jitter)
{
	return getCurrentTime() + delay + 2.0*jitter + pickGaussian(jitter);
}

class SendThread : public JThread
{
public:
	SendThread(const nut::IPv4Address &destIP, uint16_t destPort)
	{
		pipe(m_pipe);
		m_mutex.Init();
		m_stopMutex.Init();
		m_destIP = destIP;
		m_destPort = destPort;
	}
	
	~SendThread()
	{
		m_stopMutex.Lock();
		m_stopThread = true;
		m_stopMutex.Unlock();

		real_t t = getCurrentTime();
		while (IsRunning() && (getCurrentTime() - t) < 5)
		{
			m_stopMutex.Lock();
			m_stopThread = true;
			m_stopMutex.Unlock();

			sleep(1);
		}
		
		if (IsRunning())
		{
			std::cout << "Killing send thread" << std::endl;
			Kill();
		}
	}
	
	void addPacket(uint8_t *pData, size_t length, real_t sendTime)
	{
		m_mutex.Lock();
		m_packets[sendTime] = PacketInfo(pData, length);
		write(m_pipe[1],"*",1);
		m_mutex.Unlock();
	}
private:
	void *Thread()
	{
		m_stopMutex.Lock();
		m_stopThread = false;
		bool stopThread = false;
		m_stopMutex.Unlock();
		
		JThread::ThreadStarted();
		
		nut::UDPv4Socket sock("Sending socket");
		bool ret;

		ret = sock.create();
		checkError(ret, sock);

		while (!stopThread)
		{
			real_t waitTime;
			real_t curTime = getCurrentTime();

			if (m_packets.empty())
				waitTime = 0.5;
			else
			{
				m_mutex.Lock();
				waitTime = (*m_packets.begin()).first - curTime;
				m_mutex.Unlock();
				if (waitTime > 0.5)
					waitTime = 0.5;
			}
			
			if (waitTime > 0)
			{
				fd_set fdset;
				struct timeval tv;

				FD_ZERO(&fdset);
				FD_SET(m_pipe[0], &fdset);
				tv.tv_sec = (int)waitTime;
				tv.tv_usec = (int)((waitTime-(real_t)((int)waitTime))*1000000.0);
				
				select(FD_SETSIZE, &fdset, 0, 0, &tv);
				if (FD_ISSET(m_pipe[0], &fdset))
				{
					uint8_t buf[1];

					read(m_pipe[0], buf, 1);
				}
			}

			m_mutex.Lock();
			std::map<real_t, PacketInfo>::iterator it = m_packets.begin();
			bool done = false;

			while (!done && it != m_packets.end())
			{
				if (curTime < (*it).first)
					done = true;
				else
				{
					std::map<real_t, PacketInfo>::iterator it2 = it;
					it++;

					uint8_t *pData = (*it2).second.getData();
					size_t length = (*it2).second.getLength();

					m_packets.erase(it2);

					ret = sock.write(pData, length, m_destIP, m_destPort);
					checkError(ret, sock);

					delete [] pData;
				}
			}
			
			m_mutex.Unlock();

			m_stopMutex.Lock();
			stopThread = m_stopThread;
			m_stopMutex.Unlock();
		}
		
		return 0;
	}

	class PacketInfo
	{
	public:
		PacketInfo(uint8_t *pData = 0, size_t length = 0)			{ m_pData = pData; m_length = length; }
		uint8_t *getData() const						{ return m_pData; }
		size_t getLength() const						{ return m_length; }
	private:
		uint8_t *m_pData;
		size_t m_length;
	};
		
	std::map<real_t, PacketInfo> m_packets;
	int m_pipe[2];
	JMutex m_mutex, m_stopMutex;
	uint16_t m_destPort;
	nut::IPv4Address m_destIP;
	bool m_stopThread;
};

class ReceiveThread : public JThread
{
public:
	ReceiveThread(nut::UDPv4Socket *pSock, real_t delay, real_t jitter, real_t loss, real_t bandwidth, SendThread *pSendThread)
	{
		m_pSock = pSock;
		m_delay = delay;
		m_jitter = jitter;
		m_loss = loss;
		m_bandwidth = bandwidth;
		m_pSendThread = pSendThread;
	
		m_mutex.Init();
		m_stopMutex.Init();
	}

	void setDelay(real_t d)
	{
		m_mutex.Lock();
		m_delay = d;
		m_mutex.Unlock();
	}

	void setJitter(real_t j)
	{
		m_mutex.Lock();
		m_jitter = j;
		m_mutex.Unlock();
	}

	void setLoss(real_t l)
	{
		m_mutex.Lock();
		m_loss = l;
		m_mutex.Unlock();
	}
	
	void setBandwidth(real_t b)
	{
		m_mutex.Lock();
		m_bandwidth = b;
		m_mutex.Unlock();
	}
	
	ReceiveThread::~ReceiveThread()
	{
		m_stopMutex.Lock();
		m_stopThread = true;
		m_stopMutex.Unlock();

		real_t t = getCurrentTime();
		while (IsRunning() && (getCurrentTime() - t) < 5)
		{
			m_stopMutex.Lock();
			m_stopThread = true;
			m_stopMutex.Unlock();

			sleep(1);
		}
		
		if (IsRunning())
		{
			std::cout << "Killing send thread" << std::endl;
			Kill();
		}
	}
private:
	void *Thread()
	{
		m_stopMutex.Lock();
		m_stopThread = false;
		bool stopThread = false;
		m_stopMutex.Unlock();
		
		JThread::ThreadStarted();

		m_bitsAvailable = m_bandwidth;
		m_prevTime = getCurrentTime();

		while (!stopThread)
		{
			bool done = false;
			bool ret;
			
			ret = m_pSock->waitForData(0,500);
			checkError(ret, m_pSock);
		
			CheckAvailableBits();
			
			while (!done)
			{
				size_t length = 0;
				bool avail = false;
				
				ret = m_pSock->getAvailableDataLength(length,avail);
				checkError(ret, m_pSock);

				CheckAvailableBits();
	
				if (avail)
				{
					uint8_t *pData = new uint8_t [length+1]; // to avoid the length = 0 case
					
					ret = m_pSock->read(pData, length);
					checkError(ret, m_pSock);

					bool bwok = false;
					real_t curTime = getCurrentTime();

					if (m_bandwidth < 0)
						bwok = true;
					else
					{
						real_t bits = ((real_t)length)*8.0;

						if (bits < m_bitsAvailable)
							bwok = true;
						else
							bwok = false;
					}
					
					if (bwok)
					{
						m_mutex.Lock();
						bool a = acceptPacket(m_loss);
						m_mutex.Unlock();
						
						if (a)
						{
							m_mutex.Lock();
							real_t sendTime = getSendTime(m_delay, m_jitter);
							m_mutex.Unlock();
							
							m_pSendThread->addPacket(pData, length, sendTime);
					
							m_bitsAvailable -= ((real_t)length)*8.0;
						}
						else
							delete [] pData;
					}
					else
						delete [] pData;
				}
				else
					done = true;
			}

			m_stopMutex.Lock();
			stopThread = m_stopThread;
			m_stopMutex.Unlock();
		}
		
		return 0;
	}

	void CheckAvailableBits()
	{
		real_t curTime = getCurrentTime();

		if (curTime - m_prevTime >= 1.0)
		{
			m_prevTime = curTime;
			m_bitsAvailable = m_bandwidth;
		}
	}

	JMutex m_mutex, m_stopMutex;
	
	nut::UDPv4Socket *m_pSock;
	real_t m_delay;
	real_t m_jitter;
	real_t m_loss;
	real_t m_bandwidth;
	SendThread *m_pSendThread;
	bool m_stopThread;
	
	real_t m_bitsAvailable;
	real_t m_prevTime;
};

int main(void)
{
	srand48(time(NULL));
	
	std::string localPortStr, destIPStr, destPortStr;
	std::string delayStr, jitterStr, lossStr;
	std::string bandWidthStr, interactiveStr;
	
	std::cout << "Local bind port: " << std::endl;
	std::cin >> localPortStr;
	std::cout << "Destination IP: " << std::endl;
	std::cin >> destIPStr;
	std::cout << "Destination port: " << std::endl;
	std::cin >> destPortStr;
	std::cout << "Average packet delay (seconds): " << std::endl;
	std::cin >> delayStr;
	std::cout << "Packet jitter (seconds): " << std::endl;
	std::cin >> jitterStr;
	std::cout << "Packet loss (%): " << std::endl;
	std::cin >> lossStr;
	std::cout << "Bandwidth (bps): " << std::endl;
	std::cin >> bandWidthStr;
	std::cout << "Interactive? (y/n) " << std::endl;
	std::cin >> interactiveStr;

	uint16_t localPort = atoi(localPortStr.c_str());
	nut::IPv4Address destIP(ntohl(inet_addr(destIPStr.c_str())));
	uint16_t destPort = atoi(destPortStr.c_str());
	real_t delay = strtold(delayStr.c_str(), 0);
	real_t jitter = strtold(jitterStr.c_str(), 0);
	real_t loss = strtold(lossStr.c_str(), 0);
	real_t bandwidth = strtold(bandWidthStr.c_str(), 0);
	nut::UDPv4Socket *pSock = new nut::UDPv4Socket("Receiving socket");	
	bool ret, interactive;

	if (interactiveStr == std::string("y"))
		interactive = true;
	else
		interactive = false;

	std::cout << "Bind port:        " << localPort << std::endl;
	std::cout << "Destination IP:   " << destIP.getAddressString() << std::endl;
	std::cout << "Destination port: " << destPort << std::endl;
	std::cout << "Average delay:    " << delay << " seconds" << std::endl;
	std::cout << "Average jitter:   " << jitter << " seconds" << std::endl;
	std::cout << "Packet loss:      " << loss << " %" << std::endl;
	std::cout << "Bandwidth:        " << bandwidth << " bps" << std::endl;
	
	if (interactive)
		interactiveStr = std::string("yes");
	else
		interactiveStr = std::string("no");
	
	std::cout << "Interactive:      " << interactiveStr << std::endl;
	
	ret = pSock->create(localPort);
	checkError(ret, pSock);

	SendThread *pSendThread = new SendThread(destIP, destPort);
	pSendThread->Start();

	ReceiveThread *pRcvThread = new ReceiveThread(pSock, delay, jitter, loss, bandwidth, pSendThread);
	pRcvThread->Start();

	if (interactive)
	{
		bool done = false;
		while (!done)
		{
			std::cout << std::endl << std::endl;
			std::cout << "Enter value to modify" << std::endl;
			std::cout << " (b) Set bandwidth (bps)" << std::endl;
			std::cout << " (l) Set loss (%)" << std::endl;
			std::cout << " (j) Set jitter (seconds)" << std::endl;
			std::cout << " (d) Set delay (seconds)" << std::endl;
			std::cout << " (e) End interactivity" << std::endl;
			std::cout << " (q) Quit" << std::endl << std::endl;
			
			std::string str;
			
			std::cin >> str;
			
			if (std::cin.eof())
				done = true;
			else
			{
				if (str == std::string("b"))
				{
					std::cout << "Bandwidth (bps): " << std::endl;
					std::cin >> bandWidthStr;
					bandwidth = strtold(bandWidthStr.c_str(), 0);
					pRcvThread->setBandwidth(bandwidth);
					std::cout << "Bandwidth:        " << bandwidth << " bps" << std::endl;
				}
				else if (str == std::string("l"))
				{
					std::cout << "Packet loss (%): " << std::endl;
					std::cin >> lossStr;
					loss = strtold(lossStr.c_str(), 0);
					pRcvThread->setLoss(loss);
					std::cout << "Packet loss:      " << loss << " %" << std::endl;
				}
				else if (str == std::string("j"))
				{
					std::cout << "Packet jitter (seconds): " << std::endl;
					std::cin >> jitterStr;
					jitter = strtold(jitterStr.c_str(), 0);
					pRcvThread->setJitter(jitter);
					std::cout << "Average jitter:   " << jitter << " seconds" << std::endl;
				}
				else if (str == std::string("d"))
				{
					std::cout << "Average packet delay (seconds): " << std::endl;
					std::cin >> delayStr;
					delay = strtold(delayStr.c_str(), 0);
					pRcvThread->setDelay(delay);
					std::cout << "Average delay:    " << delay << " seconds" << std::endl;
				}
				else if (str == std::string("q"))
					done = true;
				else if (str == std::string("e"))
				{
					std::cout << "Interactive mode ended" << std::endl;
					while(1)
					{
						sleep(1);
					}
				}
				else
					std::cout << "INVALID INPUT!" << std::endl;
			}
		}
	}
	else
	{
		while(1)
		{
			sleep(1);
		}
	}
	
	delete pRcvThread;
	delete pSendThread;
	delete pSock;
	
	return 0;
}

				
