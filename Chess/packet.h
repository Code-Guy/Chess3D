#ifndef _PACKET_H
#define  _PACKET_H

#include <string>
#include <vector>
#include "ork/math/vec2.h"

#define DEFAULT_BUFFER_SIZE 128 

const int MaxSendSize = 65535;//send函数一次传输的量
const int PacketHeaderSize = 12;
const int SpecialFlag = 99999;

enum IDType {
	Open = 0, Move = 1, Invalid = -1
};

struct PacketHeader
{
	int startFlag;//包的标志位
	int length;//包的长度
	int ID;//包的编号
};

struct Message
{
	ork::vec2i prevPos;//当前位置
	ork::vec2i targetPos;//目标位置
	char hostName[DEFAULT_BUFFER_SIZE];//主机名
	bool isRivalOffsensive;//对方是否先手
	bool isOpen;//是否刚开局
};

class Packet
{
public:
	Packet();
	~Packet();

	void Pack(Message msg);//装包
	void Pack(int flag);//装包

	void UnPack(std::string recvData, Message &msg, int &flag);//拆包

	PacketHeader header;

	char *headerBuffer;
	char *contentBuffer;

private:
	
};

std::string Trim(std::string s);
void Split(const std::string& str, std::vector<std::string>& ret, std::string sep);

#endif //_PACKET_H