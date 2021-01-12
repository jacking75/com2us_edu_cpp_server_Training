﻿#include <iostream>
#include "PacketProcess.h"
#include "User.h"
#include "Room.h"
#include "PacketDef.h"

namespace OmokServerLib
{	
	void PacketProcess::Init(NServerNetLib::TcpNetwork* pNetwork, UserManager* pUserMgr, RoomManager* pRoomMgr, RedisManager* pRedisMgr, NServerNetLib::Logger* pLogger, ConnectedUserManager* m_pConUserMgr)
	{		
		m_pRefUserMgr = pUserMgr;
		m_pRefRoomMgr = pRoomMgr;
		m_pRefRedisMgr = pRedisMgr;
		m_pRefLogger = pLogger;
		m_pRefConUserMgr = m_pConUserMgr;

		using netLibPacketId = NServerNetLib::PACKET_ID;
		using commonPacketId = OmokServerLib::PACKET_ID;

		for (int i = 0; i < (int)commonPacketId::MAX; ++i)
		{
			PacketFuncArray[i] = nullptr;
		}

		PacketFuncArray[(int)netLibPacketId::NTF_SYS_CONNECT_SESSION] = &PacketProcess::NtfSysConnectSession;
		PacketFuncArray[(int)netLibPacketId::NTF_SYS_CLOSE_SESSION] = &PacketProcess::NtfSysCloseSession;
		PacketFuncArray[(int)commonPacketId::ROOM_ENTER_REQ] = &PacketProcess::RoomEnter;
		PacketFuncArray[(int)commonPacketId::LOGIN_IN_REQ] = &PacketProcess::Login;
		PacketFuncArray[(int)commonPacketId::ROOM_LEAVE_REQ] = &PacketProcess::RoomLeave;
		PacketFuncArray[(int)commonPacketId::ROOM_CHAT_REQ] = &PacketProcess::RoomChat;
		PacketFuncArray[(int)commonPacketId::MATCH_USER_REQ] = &PacketProcess::MatchUser;
		PacketFuncArray[(int)commonPacketId::PUT_STONE_REQ] = &PacketProcess::GamePut;
		PacketFuncArray[(int)commonPacketId::GAME_START_REQ] = &PacketProcess::GameReady;

	}
	
	void PacketProcess::Process(PacketInfo packetInfo)
	{
		auto packetId = packetInfo.PacketId;

		if (packetId < 0 || packetId > (int)OmokServerLib::PACKET_ID::MAX)
		{
			return;
		}

		if (PacketFuncArray[packetId] == nullptr)
		{
			return;
		}

		(this->*PacketFuncArray[packetId])(packetInfo);

	}

	void PacketProcess::StateCheck()
	{
		m_pRefConUserMgr->LoginCheck();
		m_pRefRoomMgr->CheckRoomGameTime();
	}

	ERROR_CODE PacketProcess::NtfSysConnectSession(PacketInfo packetInfo)
	{
		m_pRefConUserMgr->SetConnectSession(packetInfo.SessionIndex);
		return ERROR_CODE::NONE;
	}
	
	ERROR_CODE PacketProcess::NtfSysCloseSession(PacketInfo packetInfo)
	{
		auto pUser = std::get<1>(m_pRefUserMgr->GetUser(packetInfo.SessionIndex));

		if (pUser) 
		{
			auto pRoom = m_pRefRoomMgr->FindRoom(pUser->GetRoomIndex());
			if (pRoom.has_value() == true)
			{
				pRoom.value()->LeaveUser(pUser->GetIndex(), pUser->GetSessioIndex(), pUser->GetID().c_str());
			}
			m_pRefUserMgr->RemoveUser(packetInfo.SessionIndex);
		}

		m_pRefConUserMgr->SetDisConnectSession(packetInfo.SessionIndex);
		m_pRefLogger->info("NtfSysCloseSession | Close Session [{}]", packetInfo.SessionIndex);

		return ERROR_CODE::NONE;
	}

	void PacketProcess::SendPacketSetError(int sessionIndex, OmokServerLib::PACKET_ID packetID, ERROR_CODE errorCode)
	{
		if (packetID == OmokServerLib::PACKET_ID::PUT_STONE_RES)
		{
			OmokServerLib::PktPutStoneRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::PUT_STONE_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktPutStoneRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;
		}
		else if (packetID == OmokServerLib::PACKET_ID::GAME_START_RES)
		{
			OmokServerLib::PktGameReadyRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::GAME_START_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktGameReadyRes);
			resPkt.SetError(errorCode);
			
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;
		}
		else if (packetID == OmokServerLib::PACKET_ID::MATCH_USER_RES)
		{
			OmokServerLib::PktMatchRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::MATCH_USER_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktMatchRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;

		}
		else if (packetID == OmokServerLib::PACKET_ID::ROOM_ENTER_RES)
		{
			OmokServerLib::PktRoomEnterRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::ROOM_ENTER_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktRoomEnterRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;
		}
		else if (packetID == OmokServerLib::PACKET_ID::ROOM_LEAVE_RES)
		{
			OmokServerLib::PktRoomLeaveRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::ROOM_LEAVE_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktRoomLeaveRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);

			return;
		}
		else if (packetID == OmokServerLib::PACKET_ID::ROOM_CHAT_RES)
		{
			OmokServerLib::PktRoomChatRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::ROOM_CHAT_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktRoomChatRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;
		}
		else if (packetID == OmokServerLib::PACKET_ID::LOGIN_IN_RES)
		{
			OmokServerLib::PktLogInRes resPkt;
			resPkt.Id = (short)OmokServerLib::PACKET_ID::LOGIN_IN_RES;
			resPkt.TotalSize = sizeof(OmokServerLib::PktLogInRes);
			resPkt.SetError(errorCode);
			SendPacketFunc(sessionIndex, sizeof(resPkt), (char*)&resPkt);
			return;
		}

	}

	std::optional <std::pair<User*,Room*>> PacketProcess::FindUserAndRoom(int sessionIndex, OmokServerLib::PACKET_ID packetID, ERROR_CODE roomErrorCode)
	{
		bool isTrue = true;
		auto userInfo = m_pRefUserMgr->GetUser(sessionIndex);

		auto errorCode = userInfo.first;
		auto pUser = userInfo.second;

		if (errorCode != ERROR_CODE::NONE)
		{
			SendPacketSetError(sessionIndex, packetID, errorCode);
			return std::nullopt;
		}

		auto pRoom = m_pRefRoomMgr->FindRoom(pUser->GetRoomIndex());

		if (pRoom.has_value() == false)
		{
			SendPacketSetError(sessionIndex, packetID, roomErrorCode);
			return std::nullopt;
		}

		return std::make_pair(pUser, pRoom.value());
	}

}