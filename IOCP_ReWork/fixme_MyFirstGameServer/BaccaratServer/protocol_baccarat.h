#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/*
 ��� ��Ŷ���� ��Ŷ�� ù �κп��� int(4 Byte)�� ��Ŷ�� �� ũ�⸦ �����ϰ�
 char(1 Byte)�� ��Ŷ�� ������ �����Ѵ�.

  ��) �α��� ��û
		struct {
					int		size
					char	type
					char	idLen
					string  id
					char	pwdLen
					string	pwd
					short	version
				}

	1: char		- 1 Byte
	2: short	- 2 Byte

 Ŭ���̾�Ʈ�� ��û�� ���� ���������� ��� �뺸 �ÿ��� ��û�� ���� ���� �� ������
 �ΰ����� ������ ��Ŷ�� �ٿ� ������.
*/

enum
{
	// ������ �α��� ó�� -------------------------------------
	
	REQUEST_LOGIN = 0,				//	C -> S	�α��� ��û
	// idLen(1), id(text),  pwdLen(1), pwd(text), version(2) 
	
	ANSWER_LOGIN,					//	S -> C �α��� �ᰡ �뺸  
	// result(1), [ nicLen(1), NicName(text), sex(1), DBIndex(4)
	//		       level(1), win(4), lose(4), disconnect(4), ���� ��(4), ���� ��(4), 
	//			����� �����������(1), ����������(1), �ƹ�Ÿ���̾�(32),�ƹ�ŸŸ��(1) ]	
	// : �����۵��� TRUE�̸� ��� ����, �ƹ�Ÿ ������ short ������ 16�迭���� �ƹ�Ÿ Ÿ���̴�.	

	// ---------------------------------------------------------


	
	
	// ä���� ����Ʈ ��û   -------------------
	
	REQUEST_CHANNEL_LIST,
	//  
	
	ANSWER_CHANNEL_LIST,
	// LowNum(1),MidNum(1), HighNum(1), MaxUserNum(2), [CurUserNum(shorts)], ��(INT64) 
	// usernum ��Ʈ���� 2����Ʈ�� �о���� �� ä���� ���� �ο��� �˼� �ִ�.
	
	// --------------------------------------

	
	

	// ���� �˻� (ģ�� ã�� ) -------------------------------

	REQUEST_FIND_FRIEND,
	// ã�� ģ�� ���̵� ����(1), ã�� ģ�� ���̵� ���ڿ�(text) 

	ANSWER_FIND_FRIEND,
	// result(1), idLen(1), id(text), [ channelNum(-1), roomNum(-1) ]
	// roomNum�� ��� Byte�� char�� ���̿� ���� 0 ������ ���� ��� ���� ������ ���Ѵ�.
	
	// ------------------------------------------------



	// �α� �ƿ�  ---------------------------
	
	REQUEST_LOGOUT,
	// DBIndex(4)

	ANSWER_LOGOUT,

	REQUEST_LOGOUT_DB,		// S -> S �α� �ƿ����� DB ó��

	
	// --------------------------------------



	// ������ ��ȣ ( ������ ������ ��� ������ �뺸 ) ------

	REQUEST_LIFECOUNT,
	 

	// -------------------------------------------------------



	// ä�� ���� ------------------------------

	REQUEST_JOINCHANNEL,
	// channelNum(1)

	ANSWER_JOINCHANNEL,
	// result(1),[ channelNum(1), maxroomNum(1) ]

	// -----------------------------------------

	
	// ä�ο� �ִ� ��, ���� ����Ʈ�� �����ش�.

	ANSWER_NOTIFY_ROOMLIST,		// �� ����Ʈ 
	// roomNum(1), roomSerial(1), ownerLen(1), owner(text), titleLen(1), title(text),
	// roomInuserNum(1), type(1), status(1)

	
	ANSWER_NOTIFY_USERLIST,		// ä�� ���� ����Ʈ
	// usernum(1), idLen(1),id(text), position(1), nicLen(1), 
	// NicName(text), sex(1), DBIndex(4), level(1), ���ȣ(1), �濡������(1)

	// ---------------------------------------------
	

	
	// �κ񿡼��� ä�� ---------------------------------------
	
	REQUEST_CHATINLOBBY,
	// ���� ���:DBIndex(4), �޼��� ����(1), �޼���(text)

	ANSWER_CHATINLOBBY,
	// ���� ���:DBIndex(4), �޼��� ����(1), �޼���(text)

	// ---------------------------------------------------



	// �ӼӸ� (�κ񿡼��� ä�ý��� �ӼӸ��� ���Ѵ�.) -----------------------------

	REQUEST_WHISPER,
	// ToDBIndex(4), FromDBIndex(4), msgLen(1), msg(text)

	ANSWER_WHISPER,
	// ToDBIndex(4), FromDBIndex(4), msgLen(1), msg(text)

	ANSWER_WHISPER_FAIL,
	// ����(B)�� �κ� ���� �ӼӸ��� ���� ��� ��û��(A)���� �˷��ش�.

	// ----------------------------------------------------------------------------



	// �ʴ� �� 1:1 ��ȭ �ź� ---------------------------
	
	REQUEST_ALLINVITE_REJECT,
	// onoff(1), �ź��� ���� - 0, ���� ���� 1

	ANSWER_ALLINVITE_REJECT,
	// onoff(1)

	REQUEST_ALLCHAT_REJECT,
	// onoff(1)

	ANSWER_ALLCHAT_REJECT,
	// onoff(1)
	
	// -----------------------------------------------



	// 1:1 ��ȭ ------------------------------

	REQUEST_1ON1CHAT_DEMANDA,	
	// ����(B):DBIndex(4), �ڸ�Ʈ����(1), �ڸ�Ʈ(text)

	ANSWER_1ON1CHAT_DEMANDB,	
	// ��û��(A):DBIndex(4),ä�ι�ȣ(1), �ڸ�Ʈ����(1), �ڸ�Ʈ(text) 

	REQUEST_1ON1CHAT_RESULTB,
	// ���(1), ��û��(A):DBIndex(4), ä�ι�ȣ(1)

	ANSWER_1ON1CHAT_RESULTA, 
	// ���(1), ����(B):DBIndex(4)
	
	REQUEST_1ON1CHAT,
	// ����(B):DBIndex(4),  �޼�������(1), �޼���(text)

	ANSWER_1ON1CHAT,											// ���游 �޴´�
	// ��û��(A):DBIndex(4), �޼�������(1), �޼���(text)

	REQUEST_1ON1CHAT_CLOSE,
	// ����(B):DBIndex(4)

	ANSWER_1ON1CHAT_CLOSE,
	// ��û��(A): DBIndex(4)

	// ------------------------------------------


	
	// ä�� ����  -------------------------------------
	
	REQUEST_CHANGE_CHANNEL,
	
	
	ANSWER_CHANGE_CHANNEL,
	 
	
	// ------------------------------------------------------



	// �н����� ��û (�κ� �ִ� ����� ����� ���� ���忡�� �뿡 ������ ��û��) -------

	REQUEST_ROOMPASSWORDA,
	// roomSerial(1)

	ANSWER_ROOMPASSWORDB,
	// ��û�� : ��û��DBIndex(4) - ������ ���忡�� ������.

	REQUEST_ROOMPASSWORD_RESULTB,
	// result(1), ��û��DBIndex(4) - ������ ������.

	ANSWER_ROOMPASSWORD_RESULTA,
	// result(1), [ roomSerial(1), pwdLen(1), pwd(text) ]

	// -----------------------------------------------------------------------



	// ���� �����    --------------------------------------------------
	
	REQUEST_CREATEROOM,
	// type(1), titleLen(1), title(text), passwordLen(1), password(text)

	ANSWER_CREATEROOM,
	// result(1), roomSerial(1), level(1), BaseBettingMoney(4)
	
	// -------------------------------------------------------------------



	// �뿡 ���� -----------------------------------------------
	
	REQUEST_JOINROOM,
	// roomSerial(1), pwdLen(1), pwd(text)

	ANSWER_JOINROOM,
	// result(1), roomSerial(1), level(1), BaseBettingMoney(4), 
	// ��û�ڸ� ������ ��� ������(1),[ DBIndex(4),����(1), ��(INT64), �ƹ�Ÿ���̾�(32),�ƹ�ŸŸ��(1) ]
	/// : ��û�ڸ� ������ ��� ������ ���� �� �ƹ�Ÿ ������ ���ΰ��� ���� �ʴ´�.

	// ������ ���� �����ϸ� ���� �ٸ� �������� �뺸�Ѵ�.
	ANSWER_NOTIFY_USERJOINROOM,
	// DBIndex(4), ����(1), ��(INT64), �ƹ�Ÿ���̾�(32),�ƹ�ŸŸ��(1)

	// ----------------------------------------------------------


	


	
	// �뿡�� �κ�� ���� --------------------------

	REQUEST_EXITROOM,  
	// ���� ������ ���� (1) : Ŭ���̾�Ʈ�� ������ '0'�� ��� �ȴ�.

	ANSWER_EXITROOM,
	// ���� ������ ���� (1)
	// 0 : �Ϲ�����, 1 : ���� ������


	// ���� ���� �Ǿ��� �� �޴� �޼���
	ANSWER_NOTIFY_ROOMDELETE,
	// �� ��ȣ(1)
	

	// ������ ���� ������ ���� �ٸ� �������� �뺸�Ѵ�.
	ANSWER_NOTIFY_USEREXITROOM,
	// DBIndex(4), ����(1)

	// -------------------------------------------------


	// ������ ������ ������ ������ �˸�.
	ANSWER_NOTIFY_RESERVATE_EXITROOM,
	// DBindex(4), ����(1) : 0 - ������ ���� ���, 1 - ������ ���� 




	// �� �̸� ����
	REQUEST_CHANGE_ROOMTITLE,
	// ���ȣ(1), �� �̸� ���ڼ�(1), ���̸�(text)
	// ���ȣ�� ������ 0 �̴�.

	ANSWER_NOTIFY_CHANGEROOMTITLE,
	// �� ��ȣ(1), �� �̸� ���ڼ�(1), �� �̸�(text)



	// �� ����/�� ���� ����, �н����� ����
	REQUEST_CHANGE_ROOMTYPE,
	// �� Ÿ��(1), �н����� ���ڼ�(1), �н�����(text)
	// �� Ÿ�� : 0(����), 1(�� ����) - �� ���� �϶��� �н� ���� ����.      

	ANSWER_NOTIFY_CHANGEROOMTYPE,
	// �� ��ȣ(1), �� Ÿ��(1)


	

	// �뿡���� ä�� ---------------------------------------
	
	REQUEST_CHATINROOM,
	// ���� ���:DBIndex(4), �޼��� ����(1), �޼���(text)

	ANSWER_CHATINROOM,
	// ���� ���:DBIndex(4), �޼��� ����(1), �޼���(text)

	// ---------------------------------------------------



	// �濡 ������ ���� �� ä�ο� �ִ� ������ �޴� �޼���(������ ���� ���� ��)
	ANSWER_NOTIFY_ROOMSETINFO,
	// ������ ���� ��� : �� ��ȣ(1), ������(1), DBIndex(4) - DBIndex�� ���ο� ������ ��.
	// �Ϲ� ������ ������ ���� �� : ���ȣ(1), ������(1), 0(4)

	
	
	// ���� ���¸� �뺸
	ANSWER_NOTIFYROOMSTATE,
	// �� ��ȣ(1), ���� ����(1) - GAME_OFF(���), GAME_ING(���� ��)



	// �뿡���� �ʴ� -----------------------------------------

	REQUEST_INVITEA,
	// ��û�� : DBIndex(4) 

	ANSWER_INVITEB,
	// DB�ε���(4), channelNum(1), roomSerial(1),
	// �н��������(1), [ �н�����(text) ]

	REQUEST_INVITE_RESULTB,
	// result(1), DB�ε���(4), channelNum(1), roomSerial(1)

	ANSWER_INVITE_RESULTA,
	// result(1)
	
	// -----------------------------------------------------------



	// ä�ο� �ִ� ������ �뿡�� ä��, ä�ο��� ������ �̵����� �� ä���� Ÿ ������ �޴� �޼���
	ANSWER_NOTIFY_USERSETINFO, 
	// DBIndex(4), ��ġ(1), [ ���ȣ(1) ]
	

	// ä�ο��� ������ ���� ��� ä���� Ÿ �����鿡�Ե� �뺸
	ANSWER_NOTIFY_USERDELETE,
	// DBIndex(4)
	
	
	// ������ ������ ������ �����ϱ⸦ ��û.
	REQUEST_RELOADUSERITEM,


	// ������ �������� ���ο� ������ ������ �����ش�.
	ANSWER_RELOADUSERITEM,
	// �� ������ ���� ������(1), ���� ������(1) - �������� ��� ���� ���θ� �˷��ش�.TRUE, FALSE


	// ������ ������ ������ �� ��� ������ ������ �������� ���� ��Ŷ
	ANSWER_RESERVATE_EXITROOM,
};


//   <<<< ���� ��Ŷ ���� >>>>
enum {

	// ���� ���� �뺸 
	ANSWER_GAMESTART = 101,
	

	
	// ������ ���� �ݾ� �� ���� ��ġ�� �˷��ش�.
	REQUEST_BEGINGAME,
	// ���ôܰ�(1) : 1 - X1, 2 - X2, 3 - X3, 4 - X4, 5 - X5
	// ��ġ(1)     - ������ ���� ��ġ.   0 - ��ũ, 1 - �÷��̾�, 2 - Ÿ��

	ANSWER_BEGINGAME,
	// �����(1), [ DBIndex(4), ���ñݾ�(4), ��ġ(1) ] - ��� ������ ������ ��� �������� �뺸�Ѵ�.
	// ���� �ð����� ������ ���� ������ ������ ���Ƿ� ���ؼ� �뺸 �Ѵ�.

	
	// ��Ŀ�� �÷��̾��� ī�带 ������.
	ANSWER_STARTCARDS,
	// ��Ŀ(2), ��Ŀ�� ����(1), �÷��̾�(2), �÷��̾��� ����(1).    ��Ŀ, �÷��̾� ���� ī�� 2�徿 �޴´�.




	// 3��° ī�带 ���� �� �ִٸ� ������.
	ANSWER_SENDTHREECARD,
	// �÷��̾�(1), ��ũ(1).    ī�� ��ȣ 52��� �����Ѵ�.


	
	// ���� ��� �뺸
	ANSWER_RESULT,
	// �̱���(1), ��ũ ����(1), �÷��̾�����(1), �����(1),[ DBIndex(4),�ݾ�(4) ], 
	// �ݾ��� �������� �ްų� ������ �ݾ��� ���Ѵ�.



	// ���� ���� �뺸
	ANSWER_GAMEOFF,



	// ������ ��Ŷ ���� ( �������� ��Ŷ ó���� ���� �̿� ).���ӿ����� ��� ����
	FINAL_PACKETDEFINE

};




//  Server To client - reuslt(1)  -----------------------------------------------

// �α��ν��� ��� �� �޼���
enum
{
	LOGIN_SUCCESS,			//	S ->	�α��� ����
	LOGFAIL_NOID,			//	S ->	�α��� ���� : ID �̵��
	LOGFAIL_DUPLEID,		//	S ->	�α��� ���� : ID �ߺ�
	LOGFAIL_STOPID,			//	S ->	�α��� ���� : ���� ID
	LOGFAIL_PASSWORD,		//	S ->	�α��� ���� : �н����� Ʋ��
	LOGFAIL_NOMONEY,		//	S ->	�α��� ���� : ������ ����
	LOGFAIL_NOVERSION,      //	S ->	�α��� ���� : Ŭ���̾�Ʈ ������ Ʋ��
	LOGFAIL_NOLEVEL,        //  S ->    �α��� ���� : ������ ������ Ʋ��.
	LOGFAIL_DBERROR			//  S ->	DB ����. ����Ÿ���̽��� ���� ����
};


// �α� �ƿ� ��� ǥ��
enum
{
	LOGOUT_NORMAL,			// �Ϲ����� Ŭ���̾�Ʈ�� ��û�� ���� �α� �ƿ�.
	LOGOUT_NOTMONEY,		// ���� ���� ������ ��ó���� �α� �ƿ�.
};


// ���� �˻� ��û ��� �� �޼���
enum
{
	FIND_SUCCESS,			// �˻� ����
	FIND_FAIL				// �˻� ����
};


// ä�� ���� ��û ��� �� �޼���
enum
{
	JOINCHANNEL_SUCCESS,		// ä�� ���� ����
	JOINCHANNEL_FAIL_FULL,		// ä�ο� ������ ������.
	JOINCHANNEL_FAIL_NOTLEVEL	// ������ ��� ���� ���� ä����
};


// �н����� ��û�� ���� ��� �� �޼���
enum
{
	ROOMPASSWORD_SUCCESS,				// ���� ���� ��û ����
	ROOMPASSWORD_FAIL_NOINJECT,			// ���� ���� �ź�
	ROOMPASSWORD_FAIL_NOROOM,			// ���� ����
	ROOMPASSWORD_FAIL_FULLUSER			// ���� �ִ� �ο��� �ʰ�
};


// 1:1 ��ȭ ��û�� ���� ��� �� �޼���
enum
{
	ONECHAT_SUCCESS,					// 1:1 ��ȭ �¶�
	ONECHAT_FAIL_ALLREJECT,				// ��� 1:1 ��ȭ�� �ź��ϰ� �ִ�.
	ONECHAT_FAIL_REJECT,				// 1:1 ��ȭ �ź�
	ONECHAT_FAIL_NOTUSER,				// �κ� ������ ����.
};


// �� ���� ��û ��� �� �޼���
enum
{

	ROOM_SUCCESS_JOINROOM,		// �뿡 ���� ����
	ROOM_FAIL_MAXUSER,			// �뿡 �ִ��ο� �ʰ�
	ROOM_FAIL_PWD,				// ����� ���� ��ȣ Ʋ��
	ROOM_FAIL_EXIST,			// �������� �ʴ� ��
};


// �� ����� ��û ��� �� �޼���
enum
{
	CREATEROOM_SUCCESS,			// �� ���� ���� 
	CREATEROOM_FAIL_MAXROOM,	// �� ���� ���� - ä���� �ִ� �� ���� ���� ����
};


// �뿡���� ���� �ʴ� ��û ��� �� �޼���
enum
{
	INVITE_SUCCESS,
	INVITE_FAIL_ALLREJECT,		// ������ ��� �ʴ븦 �ź��ϰ� ����
	INVITE_FAIL_REJECT,			// ������ �ʴ뿡 ���� �ź�
	INVITE_FAIL_NOTUSER,		// ������ �κ� ����.
};


// ���� Ÿ��( ����, ����� )
enum
{
	PUBLICTYPE, 
	PRIVATETYPE,
};


// ���� ������ ����
enum
{
	EXITROOM_NORMAL = 0,
	EXITROOM_NOTMONEY
};


// ��� ������ ���� ����
enum
{
	GAME_CLOSE,				// �������� ���� ������ �� �Ѵ�.
	GAME_OFF,
	GAME_READY,
	GAME_WAIT,				//  �̻��� ���� �뿡 ���� �ź�.
	GAME_ING,				// ���� ���´� GAME_OFF -> GAME_READY -> GAME_WAIT -> GAME_ING  ������ ��ȭ�Ѵ�.
	GAME_SCORECALCULATE,    // �濡�� ���� ��� ���� ����� ����.
	GAME_AGENCY,			// Ŭ���̾�Ʈ�� ���� ���� �� ���� ������ ��츦 ���Ѵ�.
	GAME_NOTMONEY			// ���� ����.
};


// ������ ��ġ
enum
{
	WH_CHANNELSELECT = 0,
	WH_LOBBY,
	WH_ROOM,
	WH_NONE					// Ŭ���̾�Ʈ���� ���� �������� ���
};


#endif

// ���� ������ : 2003�� 4�� 8��