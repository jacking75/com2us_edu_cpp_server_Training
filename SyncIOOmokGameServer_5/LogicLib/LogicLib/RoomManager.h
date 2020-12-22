#pragma once

//TODO �����
// ���� �������� ���ּ���
//-> ��û�� �������� �߻��մϴ�...... 
#include <functional>
#include "Room.h"

namespace NServerNetLib
{
	class TcpNetwork;
}
namespace OmokServerLib
{

	class RoomManager
	{
	public:

		RoomManager() = default;
		~RoomManager() = default;

		//TODO �����
		// �Ƹ� �ٸ� ��� ���Ͽ��� TcpNetwork ���� ��� ������ include �ؼ� �� ���Ͽ��� include ���� �ʾƵ� ���� ������ ���� �ʽ��ϴ�.
		// �׷��� �̷� ��� �ٸ� ���Ͽ��� ��� ������ �����ϸ� ���⿡�� ���� ������ ���� �� �ֽ��ϴ�.
		// ���� ������ �ϰ� cpp���� TcpNetwork ��� ������ include �ϼ���
		//-> �ذ�
		void Init(const int maxRoomNum, NServerNetLib::TcpNetwork* pNetwork);

		std::optional <Room*> FindProperRoom();

		std::optional <Room*> FindRoom(const int roomIndex);

		std::function<void(const int, const short, const short, char*)> SendPacketFunc;

		void CheckRoomGameTime();

	private:

		std::vector<Room*> m_RoomList;
	};
}


