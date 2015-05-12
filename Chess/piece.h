#ifndef _PIECE_H
#define  _PIECE_H

#include "model.h"

#define PICK_MATERIAL_ENHANCE ork::vec3f(0, 0.7, 0.7);
#define ANIMATION_SPEED 2	//���������ٶ�Ϊÿ��2��

enum PlayerType
{
	AllyType, RivalType, NoneType
};

enum PieceType
{
	King, Queen, Rook, Bishop, Knight, Pawn,
	EKing, EQueen, ERook, EBishop, EKnight, EPawn, None
};

class Piece
{
public:
	Piece(Model *model, PieceType pieceType, ork::vec2i originPos, ork::vec3f rotate = ork::vec3f::ZERO);
	~Piece();

	void Logic(float dt);
	void Render(ork::ptr<ork::FrameBuffer> fb, float dt);
	
	void SetMaterial(Material material);
	void SetTargetPos(ork::vec2i targetPos);
	void ReBorn(Model *model, PieceType pieceType);

	bool IsPlayAnimation();

	void Pick();
	void UnPick();

	PieceType pieceType;//��������
	PlayerType friendType;//�ҷ�
	PlayerType enemyType;//�з�

	ork::vec2i originPos;//ԭʼλ�ã�x 0-7, z 0-7��
	ork::vec2i originPrevPos;//ǰһ����ǰλ��
	ork::vec2i prevPos;//��ǰ�������ϵ�λ�ã�x 0-7, z 0-7��
	ork::vec2i targetPos;//�������ϵ�Ŀ��λ�ã�x 0-7, z 0-7��

	ork::vec2f curPos;//��ǰ֡��������λ�ã�x float z float��

	ork::vec3f translate;
	ork::vec3f rotate;

	AABB aabb;
	vector<OctreeNode *> roots;

	Material originMateial;//ԭʼ�Ĳ���

	bool isMove;//�Ƿ��Ѿ��ƶ���
	bool isFirstTwoStepMove;//�Ƿ��һ�����������ֱ������ just for pawn

	friend PlayerType DeterminePlayerType(PieceType pieceType);//ȷ�������Ӫ
	friend void PrintPlayerType(PlayerType playerType);
	friend void PrintPieceType(PieceType pieceType);

private:
	Model *model;
	bool isPick;//�Ƿ�ѡ��
	bool isPlayAnimation;//�Ƿ��ڲ��Ŷ�����from prevPos to targetPos��
	bool needCalcCanGo;//�Ƿ��Ѿ���������һ��������

	ork::vec2f moveDir;//�����ƶ����� normalized
	float timer;//������
	float amount;//���ƶ�ʱ��
};

#endif //_PIECE_H