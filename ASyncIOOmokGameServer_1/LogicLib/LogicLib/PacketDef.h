#pragma once

#include "PacketID.h"
#include <string>
#include "ErrorCode.h"

namespace OmokServerLib
{
#pragma pack(push, 1)
	struct PktHeader
	{
		UINT16 TotalSize;
		UINT16 Id;
		UINT8 Type;
	};

	const UINT32 PACKET_HEADER_SIZE = sizeof(PktHeader);

	struct PktBase : public PktHeader
	{
		short ErrorCode = (short)OmokServerLib::ERROR_CODE::NONE;
		void SetError(OmokServerLib::ERROR_CODE error) { ErrorCode = (short)error; }
	};

	//- �α��� ��û
	const int MAX_USER_ID_SIZE = 16;
	const int MAX_USER_PASSWORD_SIZE = 16;

	struct PktLogInReq : public PktHeader
	{
		char szID[MAX_USER_ID_SIZE] = { 0, };
		char szPW[MAX_USER_PASSWORD_SIZE] = { 0, };
	};

	struct PktLogInRes : PktBase 
	{

	};


	struct PktMatchReq : public PktHeader
	{

	};

	struct PktMatchRes : PktBase
	{

	};


	//- �뿡 ���� ��û
	const int MAX_ROOM_TITLE_SIZE = 16;
	struct PktRoomEnterReq : public PktHeader
	{
		INT16 RoomIndex;
	};

	struct PktRoomEnterRes : PktBase
	{

	};


	struct PktPutStoneReq : public PktHeader
	{
		int x;
		int y;
	};


	struct PktGameReadyRes : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	struct PktTimeOutTurnChange : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	struct PktPutStoneRes : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	struct PktGameResultNtf : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	//- �뿡 �ִ� �������� ���� ���� ���� ���� �뺸
	struct PktRoomEnterUserInfoNtf : PktBase
	{
		INT64 UserUniqueId;
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};


	//- �� ������ ��û
	struct PktRoomLeaveReq : public PktHeader 
	{
	
	};

	struct PktRoomLeaveRes : PktBase
	{

	};

	//- �뿡�� ������ ���� �뺸(�κ� �ִ� ��������)
	struct PktRoomLeaveUserInfoNtf : PktBase
	{
		INT64 UserUniqueId;
	};


	//- �� ä��
	const int MAX_ROOM_CHAT_MSG_SIZE = 256;
	struct PktRoomChatReq : public PktHeader
	{
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	struct PktRoomChatRes : PktBase
	{
	};

	struct PktRoomChatNtf : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};


#pragma pack(pop)

}