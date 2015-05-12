#ifndef _PACKET_H
#define  _PACKET_H

#include <string>
#include <vector>
#include "ork/math/vec2.h"

#define DEFAULT_BUFFER_SIZE 128 

const int MaxSendSize = 65535;//send����һ�δ������
const int PacketHeaderSize = 12;
const int SpecialFlag = 99999;

enum IDType {
	Open = 0, Move = 1, Invalid = -1
};

struct PacketHeader
{
	int startFlag;//���ı�־λ
	int length;//���ĳ���
	int ID;//���ı��
};

struct Message
{
	ork::vec2i prevPos;//��ǰλ��
	ork::vec2i targetPos;//Ŀ��λ��
	char hostName[DEFAULT_BUFFER_SIZE];//������
	bool isRivalOffsensive;//�Է��Ƿ�����
	bool isOpen;//�Ƿ�տ���
};

class Packet
{
public:
	Packet();
	~Packet();

	void Pack(Message msg);//װ��
	void Pack(int flag);//װ��

	void UnPack(std::string recvData, Message &msg, int &flag);//���

	PacketHeader header;

	char *headerBuffer;
	char *contentBuffer;

private:
	
};

std::string Trim(std::string s);
void Split(const std::string& str, std::vector<std::string>& ret, std::string sep);

#endif //_PACKET_H