#pragma once
#include "../MGServerLib/Asios.h"
#include <map>

class MGClientReceiver : public INetworkReceiver
{
	typedef std::map<long, ASSOCKDESCEX> SessionMap;
public:
	MGClientReceiver(void);
	virtual ~MGClientReceiver(void);

	virtual void notifyRegisterSocket(ASSOCKDESCEX& sockdesc, SOCKADDR_IN& ip) override;
	virtual void notifyReleaseSocket(ASSOCKDESCEX& sockdesc) override;
	virtual void notifyMessage(ASSOCKDESCEX& sockdesc, size_t length, char* data) override;
	virtual void notifyConnectingResult(INT32 requestID, ASSOCKDESCEX& sockdesc, DWORD error);

	bool ProcessEcho();
protected:

private:
//��Ƽ������� �����ؼ�
	SessionMap m_SessionMap;
	CriticalSectionLock m_SessionLock;

	ASSOCKDESCEX m_SocketDesc;
};

