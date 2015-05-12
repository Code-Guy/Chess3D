#include "packet.h"

using namespace std;
using namespace ork;

Packet::Packet()
{
	//��ͷ
	header.startFlag = SpecialFlag;//����ı�־λ
	header.length = 0;
	header.ID = Invalid;

	//��ͷbuffer
	headerBuffer = NULL;
	//���ľ�������buffer
	contentBuffer = NULL;
}

Packet::~Packet()
{
	delete headerBuffer;
	delete contentBuffer;
}

void Packet::Pack(Message msg)//װ��
{
	//���header��ID
	header.ID = Move;

	//�Ѱ��ľ�������д��buffer��
	contentBuffer = new char[sizeof(msg)];
	memcpy(contentBuffer, &msg, sizeof(msg));

	//���°��ĳ���
	header.length = sizeof(msg);

	//�Ѱ�ͷд��buffer��
	headerBuffer = new char[sizeof(header)];
	memcpy(headerBuffer, &header, sizeof(header));
}

void Packet::Pack(int flag)//װ��
{
	//���header��ID
	header.ID = Open;

	//�Ѱ��ľ�������д��buffer��
	contentBuffer = new char[1];
	contentBuffer[0] = flag == 0 ? '0' : '1';

	//���°��ĳ���
	header.length = 1;

	//�Ѱ�ͷд��buffer��
	headerBuffer = new char[sizeof(header)];
	memcpy(headerBuffer, &header, sizeof(header));

	printf("%d %d %d %d %d\n", string(headerBuffer).size(), strlen(headerBuffer), 
		sizeof(header), sizeof(PacketHeader), sizeof(headerBuffer));
}

void Packet::UnPack(string recvData, Message &msg, int &flag)//���
{
	if (header.ID == Open)
	{
		if (recvData == "0")
		{
			flag = 0;
		}
		else if (recvData == "1")
		{
			flag = 1;
		}
		else
		{
			flag = -1;
		}

		msg.prevPos = vec2i(-1, -1);
		msg.targetPos = vec2i(-1, -1);
	}
	else if (header.ID == Move)
	{
		memcpy(&msg, recvData.data(), sizeof(msg));
		flag = -1;
	}
}

string Trim(string s)
{
	if (s.empty())   
	{  
		return s;  
	}  

	s.erase(0,s.find_first_not_of(" "));  
	s.erase(s.find_last_not_of(" ") + 1);  
	return s;
}

void Split(const string& str, vector<string>& ret, string sep)
{
	if (str.empty())
	{
		return;
	}

	string tmp;
	string::size_type pos_begin = str.find_first_not_of(sep);
	string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret.push_back(Trim(tmp));
			tmp.clear();
		}
	}
}