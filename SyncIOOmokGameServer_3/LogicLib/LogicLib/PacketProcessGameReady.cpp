#include "PacketProcess.h"
#include "PacketDef.h"

namespace ChatServerLib
{
	ERROR_CODE PacketProcess::GameReady(PacketInfo packetInfo)
	{
		NCommon::PktGameReadyRes resPkt;

		auto userInfo = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = userInfo.first;
		auto pUser = userInfo.second;

		if (errorCode != ERROR_CODE::NONE)
		{
			resPkt.SetError(errorCode);
			m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
			return errorCode;
		}
		auto checkReadyResult = m_pRefUserMgr->CheckReady(pUser);

		if (checkReadyResult != ERROR_CODE::NONE)
		{
			resPkt.SetError(checkReadyResult);
			m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
			return errorCode;
		}

		pUser->SetReady();

		auto pRoom = m_pRefRoomMgr->FindRoom(pUser->GetRoomIndex());
		auto pOpponentUser = packetInfo.SessionIndex == pRoom.value()->m_UserList[0]->GetSessioIndex() ? pRoom.value()->m_UserList[1] : pRoom.value()->m_UserList[0];
		auto roomUserIndex = packetInfo.SessionIndex == pRoom.value()->m_UserList[0]->GetSessioIndex() ? 1 : 0;

		if (pOpponentUser->IsCurDomainInReady() == true)
		{
			pOpponentUser->SetGame();
			pUser->SetGame();

			//TODO :  ������ ���� ����
			pRoom.value()->m_OmokGame->m_BlackStoneUserIndex = std::abs(1 - roomUserIndex);
			pRoom.value()->m_OmokGame->m_WhiteStoneUserIndex = roomUserIndex;
			pRoom.value()->m_OmokGame->m_TurnIndex = packetInfo.SessionIndex;
			pRoom.value()->m_OmokGame->init();
			pRoom.value()->m_OmokGame->initType();

			strncpy_s(resPkt.UserID, (NCommon::MAX_USER_ID_SIZE + 1),pUser->GetID().c_str(), NCommon::MAX_USER_ID_SIZE);
			m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
			pRoom.value()->NotifyGameStart(packetInfo.SessionIndex, pUser->GetID().c_str());
		}
		else
		{
			resPkt.SetError(ERROR_CODE::NOT_READY_EXIST);
			m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
		}
	

		return ERROR_CODE::NONE;
	}
}
