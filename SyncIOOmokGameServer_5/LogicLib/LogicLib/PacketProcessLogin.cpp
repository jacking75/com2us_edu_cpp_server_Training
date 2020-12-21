#include <tuple>
#include <iostream>
#include "User.h"
#include "UserManager.h"
#include "PacketProcess.h"
#include "PacketDef.h"

namespace OmokServerLib
{
	
	ERROR_CODE PacketProcess::Login(PacketInfo packetInfo)
	{
		//TODO �����
		// �̹� �α��� �� �����ε� �� �α��� ��û�� ���� ��� �� ����ϰ� �ֳ���?
		// -> �ذ�
		auto reqPkt = (OmokServerLib::PktLogInReq*)packetInfo.pRefData;
		OmokServerLib::PktLogInRes resPkt;

		if (m_pRefConUserMgr->CheckUserLogin(packetInfo.SessionIndex) == true)
		{
			resPkt.SetError(ERROR_CODE::ALREADY_LOGIN_STATE);
			SendPacketFunc(packetInfo.SessionIndex, (short)OmokServerLib::PACKET_ID::LOGIN_IN_RES, sizeof(resPkt), (char*)&resPkt);
			return ERROR_CODE::ALREADY_LOGIN_STATE;
		}

		CommandRequest redisRequestInfo;
		redisRequestInfo.sessionIndex = packetInfo.SessionIndex;
		redisRequestInfo.redisTaskID = (int)RedisTaskID::confirmLogin;
		redisRequestInfo.commandBody = packetInfo.pRefData;

		m_pRefRedisMgr->InsertRedisRequestQueue(redisRequestInfo);

		return ERROR_CODE::NONE;
	}
	
}