#pragma once

#include "Session.h"

/// �ٸ� �������� ������ ���� ����
class ServerSession : public Session, public ObjectPool<ServerSession>
{
public:
	ServerSession(const char* serverAddr, unsigned short port);
	virtual ~ServerSession();

	bool ConnectRequest();
	bool ConnectCompletion();


private:
	const char*		mServerAddr;
	unsigned short	mPort;

};