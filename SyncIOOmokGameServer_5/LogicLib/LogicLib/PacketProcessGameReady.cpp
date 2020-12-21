#include "PacketProcess.h"
#include "PacketDef.h"

//TODO �����
// ��Ŷ �ڵ鷯 �Լ��� �ִ� �ڵ���� Room���� �ű� �� ������ �������� �Ű��ּ���
// ��Ŷ �ڵ� �ڵ尡 ���� �ִ� �� ���� ���� �ʽ��ϴ�.
//-> �Ʒ��� UserSetGame �Լ��� Room ���� �ű�� Room�� RoomManager�� �������־���ϴµ� �̰������� �����߻�.. �ٽð��..
namespace OmokServerLib
{
	ERROR_CODE PacketProcess::GameReady(PacketInfo packetInfo)
	{
		OmokServerLib::PktGameReadyRes resPkt;

		//TODO �����
		// �� ������ ���� �濡 �ִ� �������� Ȯ���غ��� ���� �ʳ���?
		// �� �ܰ迡�� room ��ü�� ��� ���� �����ϴ�.
		//-> �ذ�
		auto findResult = FindUserAndRoom(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::GAME_START_RES, ERROR_CODE::USER_STATE_NOT_ROOM);

		if (findResult.has_value() == false)
		{
			return ERROR_CODE::USER_ROOM_FIND_ERROR;
		}

		auto pUser = findResult.value().first;
		auto pRoom = findResult.value().second;

		if (pUser->IsCurDomainInRoom() == false)
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::GAME_START_RES, ERROR_CODE::USER_STATE_NOT_ROOM);
			return ERROR_CODE::USER_STATE_NOT_ROOM;
		}

		auto checkReadyResult = m_pRefUserMgr->CheckReady(pUser);

		if (checkReadyResult != ERROR_CODE::NONE)
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::GAME_START_RES, checkReadyResult);
			return checkReadyResult;
		}

		pUser->SetReady();

		auto setUserStateResult = UserSetGame(pUser, packetInfo.SessionIndex);

		if (setUserStateResult == ERROR_CODE::NONE)
		{
			strncpy_s(resPkt.UserID, (OmokServerLib::MAX_USER_ID_SIZE + 1), pUser->GetID().c_str(), OmokServerLib::MAX_USER_ID_SIZE);
			SendPacketFunc(packetInfo.SessionIndex, (short)OmokServerLib::PACKET_ID::GAME_START_RES, sizeof(resPkt), (char*)&resPkt);
		}
		else
		{
			SendPacketSetError(packetInfo.SessionIndex, OmokServerLib::PACKET_ID::GAME_START_RES, ERROR_CODE::NOT_READY_EXIST);
		}

		return ERROR_CODE::NONE;
	}

	ERROR_CODE PacketProcess::UserSetGame(User* pUser, int sessionIndex)
	{
		auto pRoom = m_pRefRoomMgr->FindRoom(pUser->GetRoomIndex());
		auto pOpponentUser = sessionIndex == pRoom.value()->m_UserList[0]->GetSessioIndex() ? pRoom.value()->m_UserList[1] : pRoom.value()->m_UserList[0];
		auto roomUserIndex = sessionIndex == pRoom.value()->m_UserList[0]->GetSessioIndex() ? 1 : 0;

		if (pOpponentUser->IsCurDomainInReady() == true)
		{
			pOpponentUser->SetGame();
			pUser->SetGame();
			pRoom.value()->SetRoomStateGame();

			//������ ���� ����
			pRoom.value()->m_OmokGame->m_BlackStoneUserIndex = std::abs(1 - roomUserIndex);
			pRoom.value()->m_OmokGame->m_WhiteStoneUserIndex = roomUserIndex;
			pRoom.value()->m_OmokGame->m_TurnIndex = sessionIndex;
			pRoom.value()->m_OmokGame->init();
			pRoom.value()->NotifyGameStart(sessionIndex, pUser->GetID().c_str());

			pRoom.value()->m_OmokGame->SetUserTurnTime();

			return ERROR_CODE::NONE;

		}
		else
		{
			return ERROR_CODE::NOT_READY_EXIST;
		}

		return ERROR_CODE::UNASSIGNED_ERROR;
	}
}
