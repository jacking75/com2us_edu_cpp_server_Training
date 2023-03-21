#pragma once

#include <unordered_map>

#define CQNET_USE_HEAD_ONLY 1


#include "../../CQNetLib/SpinLock.h"
#include "../../CQNetLib/IocpServer.h"


class EchoServer : public CQNetLib::IocpServer
{
	typedef std::pair<CQNetLib::Session*, UINT32>  CONN_PAIR;

public:
	EchoServer()
	{

	}

	~EchoServer()
	{

	}

	void Init(CQNetLib::INITCONFIG& initConfig, UINT32 dwMaxConnection)
	{
		CreateConnection(initConfig, dwMaxConnection, this);
	}

	virtual	bool OnAccept(CQNetLib::Session* pConnection)
	{
		CQNetLib::SpinLockGuard lock(m_Lock);

		auto conn_it = m_mapConnection.find(pConnection);

		//�̹� ���ӵǾ� �ִ� �����̶��
		if (conn_it != m_mapConnection.end())
		{
			//LOG(LOG_INFO_NORMAL, "SYSTEM | ConnectionManager::AddConnection() | index[%d]�� �̹� m_mapConnection�� �ֽ��ϴ�.", pConnection->GetIndex());
			return false;
		}

		m_mapConnection.insert(CONN_PAIR(pConnection, GetTickCount()));
		return true;
	}


	//client���� packet�� �����Ͽ� ���� �� �ִ� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
	virtual	bool OnRecv([[maybe_unused]] CQNetLib::Session* pConnection, [[maybe_unused]] UINT32 dwSize, [[maybe_unused]] char* pRecvedMsg)
	{
		/*auto pChat = (Packet_Chat*)pConnection->PrepareSendPacket(sizeof(Packet_Chat));

		if (NULL == pChat)
		{
			continue;
		}

		pChat->s_sType = PACKET_CHAT;
		strncpy_s(pChat->s_szChatMsg, szChatMsg, MAX_CHATMSG);
		strncpy_s(pChat->s_szIP, szIP, MAX_IP);
		pConnection->SendPost(pChat->s_nLength);*/
		return true;
	}

	//client���� packet�� �����Ͽ� ���� �� ���� ��Ŷ�� ó���� �� ȣ��Ǵ� �Լ�
	virtual	bool OnRecvImmediately([[maybe_unused]] CQNetLib::Session* lpConnection, [[maybe_unused]] UINT32 dwSize, [[maybe_unused]] char* pRecvedMsg)
	{
		return true;
	}

	//client ���� ����� ȣ��Ǵ� �Լ�
	virtual	void OnClose(CQNetLib::Session* pConnection)
	{
		CQNetLib::SpinLockGuard lock(m_Lock);

		auto conn_it = m_mapConnection.find(pConnection);

		//���ӵǾ� �ִ� ������ ���°��
		if (conn_it == m_mapConnection.end())
		{
			//LOG(LOG_INFO_NORMAL, "SYSTEM | ConnectionManager::RemoveConnection() | index[%d]�� m_mapConnection�� �����ϴ�.", pConnection->GetIndex());
			return;
		}

		m_mapConnection.erase(pConnection);
	}

	//�������� ProcessThread�� �ƴ� �ٸ� �����忡�� �߻���Ų 
	//�޽����� ���� ���ְ� ó���Ǿ� �Ѵٸ� �� �Լ��� ���.
	virtual bool OnSystemMsg([[maybe_unused]] void* pOwnerKey, [[maybe_unused]] UINT32 dwMsgType, [[maybe_unused]] LPARAM lParam)
	{
		return true;
	}


private:
	bool CreateConnection(CQNetLib::INITCONFIG& initConfig, UINT32 dwMaxConnection, CQNetLib::IocpServer* pServer)
	{
		CQNetLib::SpinLockGuard lock(m_Lock);

		m_pConnection = new CQNetLib::Session[dwMaxConnection];

		for (int i = 0; i < (int)dwMaxConnection; i++)
		{
			initConfig.nIndex = i;
			if (auto ret = m_pConnection[i].CreateConnection(initConfig); ret != CQNetLib::ERROR_CODE::none)
			{
				return false;
			}

			m_pConnection[i].SetDelegate(SA::delegate<void(CQNetLib::Session*)>::create<CQNetLib::IocpServer, &CQNetLib::IocpServer::OnClose>(pServer),
				SA::delegate<bool(CQNetLib::Session*, bool)>::create<CQNetLib::IocpServer, &CQNetLib::IocpServer::CloseConnection>(pServer));
		}

		return true;
	}


private:
	CQNetLib::SpinLock m_Lock;

	CQNetLib::Session* m_pConnection = nullptr;
		
	std::unordered_map<CQNetLib::Session*, UINT32>   m_mapConnection;

};