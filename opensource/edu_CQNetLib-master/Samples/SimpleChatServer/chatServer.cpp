#include "chatServer.h"


ChatServer::ChatServer()
{

}

ChatServer::~ChatServer()
{

}

void ChatServer::Init(CQNetLib::INITCONFIG& initConfig, UINT32 dwMaxConnection)
{
	m_ConnectionMgr.CreateConnection(initConfig, 10, this);
}

bool ChatServer::OnAccept(CQNetLib::Session* lpConnection) 
{
	return true;
}

//client���� packet�� �����Ͽ� ���� �� �ִ� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
bool ChatServer::OnRecv(CQNetLib::Session* lpConnection, UINT32 dwSize, char* pRecvedMsg)
{
	return true;
}

//client���� packet�� �����Ͽ� ���� �� ���� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
bool ChatServer::OnRecvImmediately(CQNetLib::Session* lpConnection, UINT32 dwSize, char* pRecvedMsg)
{
	return true;
}

//client ���� ����� ȣ��Ǵ� �Լ�
void ChatServer::OnClose(CQNetLib::Session* lpConnection)
{

}

//�������� ProcessThread�� �ƴ� �ٸ� �����忡�� �߻���Ų 
//�޽����� ���� ���ְ� ó���Ǿ� �Ѵٸ� �� �Լ��� ���.
bool ChatServer::OnSystemMsg(void* pOwnerKey, UINT32 dwMsgType, LPARAM lParam)
{
	return true;
}