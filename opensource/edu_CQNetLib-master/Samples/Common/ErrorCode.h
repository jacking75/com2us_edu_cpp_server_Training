#pragma once

//���� �ڵ� ����
enum class ErrorCode : short
{
	//������ ���� �ڵ�
	ERR_NONE						= 0x00000000,			//�Ϲ����� ��� ����
	
	ERR_NONE_CREATENICKNAME			= 0x00000002,			//�г��� ���� ����
	ERR_NONE_CREATECHARACTER		= 0x00000003,			//ĳ���� ���� ����
	ERR_NONE_CONNECTGAMESERVER		= 0x00000004,			//���Ӽ��� ���� ���
	ERR_NONE_CREATEACCOUNT			= 0x00000005,			//�������� ����
	ERR_NONE_SAVEINVENTORYINFO		= 0x00000006,			//�κ��丮 ���� ����
	ERR_NONE_SERVICESTART			= 0x00000007,			//���� ���� ����
	ERR_NONE_SERVICESTOP			= 0x00000008,			//���� ���� ����
	ERR_NONE_CONNECTSTART			= 0x00000009,			//���� ���� ����
	ERR_EXIST						= 0x00001000,			//������ �ִ��� üũ�ϴ� ����

	//�α� ���� ���� �ڵ�
	ERR_LOGIN_SAMEID				= 0x00001001,			//���� ���̵� �����մϴ�.
	ERR_LOGIN_SERVER				= 0x00001002,			//�α� ���� ����
	ERR_LOGIN_ID					= 0x00001003,			//ID�� �������� �ʽ��ϴ�.
	ERR_LOGIN_PW					= 0x00001004,			//PW�� Ʋ���ϴ�.
	ERR_LOGIN_SAMENICKNAME     		= 0x00001005,			//���� �г����� ���� �մϴ�.
	ERR_LOGIN_NOEXISTCHACRACTER		= 0x00001006,			//ĳ���Ͱ� ����. ó�� ���� ����� ��
	ERR_LOGIN_NOEXISTNICKNAME		= 0x00001007,			//�г����� ����
	ERR_LOGIN_COMMUNITYSERVER		= 0x00001008,			//Ŀ�´�Ƽ ������ �α��� �� ��������.


	//���ӷ� ���� ���� �ڵ�
	ERR_GAMEROOM_FULLROOM			= 0x00001100,			//���ӷ��� ���̻� �����Ҽ� ����.
	ERR_GAMEROOM_NOTEXIST			= 0x00001101,			//���ӷ��� ã���� �����ϴ�.
	ERR_GAMEROOM_FULLUSER			= 0x00001102,			//����ڰ� �������� �濡 ���� �����ϴ�.
	ERR_GAMEROOM_NOTREADY			= 0x00001103,			//�غ���� ���� ����ڰ� �ֽ��ϴ�
	ERR_GAMEROOM_ALREADYSTART		= 0x00001104,			//�̹� �÷������� ���̴�.
	ERR_GAMEROOM_NOMUSIC			= 0x00001105,			//���� �����̴�.
	ERR_GAMEROOM_FULLAUDIENCE   	= 0x00001106,			//������ ������ ���� á���ϴ�.
	ERR_GAMEROOM_FULLPLAYER			= 0x00001107,			//�÷��̾� ������ ���� á���ϴ�.
	ERR_GAMEROOM_NOTEAM				= 0x00001108,			//���� �߸��Ǿ���.(���ο��� ���� �ʴ´�.)
	ERR_GAMEROOM_SELECTEDINSTRUMENT	= 0x00001109,			//�̹� ���õ� �Ǳ� �Դϴ�.
	ERR_GAMEROOM_NOSELECTMUSIC		= 0x00001110,			//���õ� ������ ���� ���� �������� ���Ѵ�.
	ERR_GAMEROOM_NOSELECTINSTRUMENT = 0x00001111,			//�Ǳ⸦ �������� �ʾƼ� �������� ���Ѵ�.
	ERR_GAMEROOM_WRONGSLOTNO		= 0x00001112,			//�߸��� ���Թ�ȣ
	ERR_GAMEROOM_WRONGROOMPW		= 0x00001113,			//�߸��� �н�����
	ERR_GAMEROOM_NOQUICKSTART		= 0x00001114,			//�� ���� ��� ����ŸƮ ����
	ERR_GAMEROOM_FAILEDCREATEROOM	= 0x00001115,			//���ӷ� ����⸦ �����Ͽ���.
	
	//�κ��丮 ���� ���� �ڵ�
	ERR_INVENTORY_NOCHARACTER		= 0x00001201,			//�ش� ĳ���ʹ� �������� �ʴ´�.
	ERR_INVENTORY_NOITEM			= 0x00001202,			//���� �������� �����Ϸ��� �ߴ�.
	ERR_SQL_FAILED					= 0x00009001,			//��� ����

	
	//Ŀ�´�Ƽ ���� ���� �ڵ�
	ERR_COMMUNITY_SERVER			= 0x00003001,			//Ŀ�´�Ƽ ���� ����
	ERR_COMMUNITY_NOTCONNECTDB		= 0x00003002,			//����DB������ ������� �ʾҽ��ϴ�.
	ERR_COMMUNITY_NOTCONNECTGS		= 0x00003003,			//���Ӽ����� ������� �ʾҽ��ϴ�.
	
	ERR_COMMUNITY_NOUSER			= 0x00003101,			//����ڸ� ã���� �����ϴ�.
	ERR_COMMUNITY_NOFRIEND			= 0x00003102,			//ģ���� ã���� �����ϴ�.
	ERR_COMMUNITY_NOTEXISTID		= 0x00003103,			//�������� �ʴ� ID�Դϴ�.
	ERR_COMMUNITY_ALREADYFRIEND		= 0x00003104,			//�̹� ��ϵ� ģ���Դϴ�.
	ERR_COMMUNITY_NOCONFIRMID		= 0x00003105,			//ID�� ������ �� ���ڸ� �����մϴ�.

	ERR_COMMUNITY_NOTSENDMSG		= 0x00003201,			//�޽����� ������ ���߽��ϴ�.

	//���� ���� �ڵ�
	ERR_SERVICE_START				= 0x00004001,			//���� ������ ���߽��ϴ�.
	ERR_SERVICE_STOP				= 0x00004002,			//���� ������ ���߽��ϴ�.
	ERR_PROCESS_START				= 0x00004003,			//���μ��� ������ ���߽��ϴ�.
	ERR_CONNECT_START				= 0x00004004,			//���� ������ ���߽��ϴ�.
	

	ERR_COMMUNITY_NOTCONNECTCS		= 0x00002003,		//Ŀ�´�Ƽ������ ������� �ʾҽ��ϴ�.
	
	ERR_BUFFER_FAILED				= 0x00005001,			//���� ����
};

