#pragma once

#include "PacketID.h"
#include "ErrorCode.h"
#include <string>

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
		short ErrorCode = (short)NServerNetLib::ERROR_CODE::NONE;
		void SetError(NServerNetLib::ERROR_CODE error) { ErrorCode = (short)error; }
	};

	//- �α��� ��û
	const int MAX_USER_ID_SIZE = 16;
	const int MAX_USER_PASSWORD_SIZE = 16;
	struct PktLogInReq
	{
		char szID[MAX_USER_ID_SIZE + 1] = { 0, };
		char szPW[MAX_USER_PASSWORD_SIZE + 1] = { 0, };
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
		short RoomIndex;
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

	struct PktPutStoneInfoNtf : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		int xPos;
		int yPos;
	};
	struct PktGameResultNtf : PktBase
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};

	//- �뿡 �ִ� �������� ���� ���� ���� ���� �뺸
	struct PktRoomEnterUserInfoNtf
	{
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
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
	};


	//- �� ä��
	const int MAX_ROOM_CHAT_MSG_SIZE = 256;
	struct PktRoomChatReq
	{
		wchar_t Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	struct PktRoomChatRes : PktBase
	{
	};

	struct PktRoomChatNtf
	{
		char UserID[MAX_USER_ID_SIZE + 1] = { 0, };
		wchar_t Msg[MAX_ROOM_CHAT_MSG_SIZE + 1] = { 0, };
	};

	const int DEV_ECHO_DATA_MAX_SIZE = 1024;

	struct PktDevEchoReq
	{
		short DataSize;
		char Datas[DEV_ECHO_DATA_MAX_SIZE];
	};

	struct PktDevEchoRes : PktBase
	{
		short DataSize;
		char Datas[DEV_ECHO_DATA_MAX_SIZE];
	};

#pragma pack(pop)



}