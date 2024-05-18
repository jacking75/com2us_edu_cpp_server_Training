#pragma once

#include "../../CQNetLib/IocpServer.h"

#include "sessionManager.h"

class ChatServer : public CQNetLib::IocpServer
{
public:
	ChatServer();
	~ChatServer();

	void Init(CQNetLib::INITCONFIG& initConfig, UINT32 dwMaxConnection);
	
	virtual	bool OnAccept(CQNetLib::Session* lpConnection) override;
	

	//client���� packet�� �����Ͽ� ���� �� �ִ� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
	virtual	bool OnRecv(CQNetLib::Session* lpConnection, UINT32 dwSize, char* pRecvedMsg) override;
	
	//client���� packet�� �����Ͽ� ���� �� ���� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
	virtual	bool OnRecvImmediately(CQNetLib::Session* lpConnection, UINT32 dwSize, char* pRecvedMsg) override;
	
	//client ���� ����� ȣ��Ǵ� �Լ�
	virtual	void OnClose(CQNetLib::Session* lpConnection) override;
	
	//�������� ProcessThread�� �ƴ� �ٸ� �����忡�� �߻���Ų 
	//�޽����� ���� ���ְ� ó���Ǿ� �Ѵٸ� �� �Լ��� ���.
	virtual bool OnSystemMsg(void* pOwnerKey, UINT32 dwMsgType, LPARAM lParam) override;
	

private:
	ConnectionManager m_ConnectionMgr;

};