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

	virtual void Init() = 0;//��ʼ��
	virtual void Logic(float dt) = 0;
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt) = 0;

	Piece *GetPiece(ork::vec2i pos);
	int DeletePiece(ork::vec2i pos);
	int DeletePiece(Piece *p);
	void SetName(const char *name);
	string GetName();
	void SetOffsensive(int flag);//�趨���ֺ���
	int GetOffsensive();//�������ֺ���
	string GetOffsensiveStr();

	std::vector<Piece *> pieces;//���� note:���ӿ�����prevPos��Ψһ��ʶ
	Board *board;//����

private:
	std::string name;
	int isOnTheOffsensive;//�Ƿ�����
};

class Ally : public Player
{
public:
	Ally();
	~Ally();

	virtual void Init();//��ʼ��
	virtual void Logic(float dt);
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt);

	void RayCast(int x, int y, int mouseState);//mouseState 0��UP 1:DOWN
	Message GetAllyPieceMoveMsg();//�õ�Ally�������ƶ���Ϣ
	void RandomOffsensive();//����������ֺ���

	static Ally *GetInstance();

private:
	Ray ray;//�õ�Ͷ������
	int pickPieceIndex;//ʰȡ�������ӵ�����
	bool isPieceMove;//�Ƿ������ӱ��ƶ�

	static Ally *instance;
};

class Rival : public Player
{
public:
	Rival();
	~Rival();

	virtual void Init();//��ʼ��
	virtual void Logic(float dt);
	virtual void Render(ork::ptr<ork::FrameBuffer> fb, float dt);

	void DriveByMsg(Message msg);

	static Rival *GetInstance();

private:
	ork::vec2i AdjustEnemyPos(ork::vec2i pos);

	static Rival *instance;
};

#endif //_PLAYER_H