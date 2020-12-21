#include "PacketProcess.h"
#include "PacketDef.h"


namespace OmokServerLib
{
	ERROR_CODE PacketProcess::MatchUser(PacketInfo packetInfo)
	{
		OmokServerLib::PktMatchRes resPkt;

		//TODO �����
		// �ٸ� ��Ŷ ó�� �Լ����� ����� �ڵ尡 �ֽ��ϴ�. �ߺ��� �������ּ���
		//-> 25�ٿ��� ���� ã���� �ٸ� �Լ���� �޸� FindProperRoom �Լ��� ���� ã�� �ٸ� �Լ���� �޶� ���⿡ ������ �ִ� �� �����ϴ�.
		auto userInfo = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);

		auto errorCode = userInfo.first;
		auto pUser = userInfo.second;

		if (errorCode != ERROR_CODE::NONE) 
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::MATCH_USER_RES, errorCode);
			return errorCode;
		}

		auto pRoom = m_pRefRoomMgr->FindProperRoom();

		if (pRoom.has_value() == false) 
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::ROOM_ENTER_RES, ERROR_CODE::MATCHING_FAIL);
			return errorCode;
		}
		//

		pRoom.value()->EnterUser(pUser);
		pUser->EnterRoom(pRoom.value()->GetIndex());
		pRoom.value()->NotifyEnterUserInfo(packetInfo.SessionIndex, pUser->GetID().c_str());

		SendPacketFunc(packetInfo.SessionIndex, (short)OmokServerLib::PACKET_ID::MATCH_USER_RES, sizeof(resPkt), (char*)&resPkt);
		
		return ERROR_CODE::NONE;

	}
}