#ifndef _PLAYER_H
#define  _PLAYER_H

#include "ray.h"
#include <vector>

const Material AllyMat(ork::vec3f(0.00, 0.00, 0.00), ork::vec3f(0.9, 0.9, 0.9), ork::vec3f(0.50, 0.50, 0.50));
const Material RivalMat(ork::vec3f(0.00, 0.00, 0.00), ork::vec3f(0.22, 0.17, 0.20), ork::vec3f(0.50, 0.50, 0.50));

class Message;
class Player
{
public:
	Player();
	~Player();

	virtual void Init() = 0;//初始化
	virtual void Logic(float dt) = 0;
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt) = 0;

	Piece *GetPiece(ork::vec2i pos);
	int DeletePiece(ork::vec2i pos);
	int DeletePiece(Piece *p);
	void SetName(const char *name);
	string GetName();
	void SetOffsensive(int flag);//设定先手后手
	int GetOffsensive();//返回先手后手
	string GetOffsensiveStr();

	std::vector<Piece *> pieces;//棋子 note:棋子可以用prevPos来唯一标识
	Board *board;//棋盘

private:
	std::string name;
	int isOnTheOffsensive;//是否先手
};

class Ally : public Player
{
public:
	Ally();
	~Ally();

	virtual void Init();//初始化
	virtual void Logic(float dt);
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt);

	void RayCast(int x, int y, int mouseState);//mouseState 0：UP 1:DOWN
	Message GetAllyPieceMoveMsg();//得到Ally的棋子移动信息
	void RandomOffsensive();//随机生成先手后手

	static Ally *GetInstance();

private:
	Ray ray;//得到投射射线
	int pickPieceIndex;//拾取到的棋子的索引
	bool isPieceMove;//是否有棋子被移动

	static Ally *instance;
};

class Rival : public Player
{
public:
	Rival();
	~Rival();

	virtual void Init();//初始化
	virtual void Logic(float dt);
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt);

	void DriveByMsg(Message msg);

	static Rival *GetInstance();

private:
	ork::vec2i AdjustEnemyPos(ork::vec2i pos);

	static Rival *instance;
};

#endif //_PLAYER_H