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

		if (m_pRefConUserMgr->CheckUserLogin(packetInfo.SessionIndex) == true)
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::LOGIN_IN_RES, ERROR_CODE::ALREADY_LOGIN_STATE);
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