#pragma once

#include "PacketID.h"
#include <string>
#include "ErrorCode.h"

namespace NCommon
{
#pragma pack(push, 1)
	struct PktHeader
	{
		short TotalSize;
		short Id;
		unsigned char Reserve;
	};

	struct PktBase
	{
		short ErrorCode = (short)ChatServerLib::ERROR_CODE::NONE;
		void SetError(ChatServerLib::ERROR_CODE error) { ErrorCode = (short)error; }
	};

	//- �α��� ��û
	const int MAX_USER_ID_SIZE = 16;
	const int MAX_USER_PASSWORD_SIZE = 16;

	struct PktLogInReq
	{
		char szID[MAX_USER_ID_SIZE] = { 0, };
		char szPW[MAX_USER_PASSWORD_SIZE] = { 0, };
	};

	struct PktLogInRes : PktBase
	{

	};


	struct PktMatchReq
	{
		
	};

	struct PktMatchRes : PktBase
	{
		
	};


	//- �뿡 ���� ��û
	const int MAX_ROOM_TITLE_SIZE = 16;
	struct PktRoomEnterReq
	{
		INT16 RoomIndex;
	};

	struct PktRoomEnterRes : PktBase
	{

	};


	struct PktPutStoneReq
	{
		int x;
		int y;
	};


	struct PktGameReadyRes : PktBase
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
	struct PktRoomEnterUserInfoNtf
	{
		INT64 UserUniqueId;
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};


	//- �� ������ ��û
	struct PktRoomLeaveReq {};

	struct PktRoomLeaveRes : PktBase
	{
	
	};

	//- �뿡�� ������ ���� �뺸(�κ� �ִ� ��������)
	struct PktRoomLeaveUserInfoNtf
	{
		INT64 UserUniqueId;
	};


	//- �� ä��
	const int MAX_ROOM_CHAT_MSG_SIZE = 256;
	struct PktRoomChatReq
	{
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	struct PktRoomChatRes : PktBase
	{
	};

	struct PktRoomChatNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		char Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};


#pragma pack(pop)



}