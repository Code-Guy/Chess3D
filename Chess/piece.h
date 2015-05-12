#ifndef _PIECE_H
#define  _PIECE_H

#include "model.h"

#define PICK_MATERIAL_ENHANCE ork::vec3f(0, 0.7, 0.7);
#define ANIMATION_SPEED 2	//动画播放速度为每秒2格

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

	PieceType pieceType;//棋子类型
	PlayerType friendType;//我方
	PlayerType enemyType;//敌方

	ork::vec2i originPos;//原始位置（x 0-7, z 0-7）
	ork::vec2i originPrevPos;//前一个当前位置
	ork::vec2i prevPos;//当前在棋盘上的位置（x 0-7, z 0-7）
	ork::vec2i targetPos;//在棋盘上的目标位置（x 0-7, z 0-7）

	ork::vec2f curPos;//当前帧棋盘所在位置（x float z float）

	ork::vec3f translate;
	ork::vec3f rotate;

	AABB aabb;
	vector<OctreeNode *> roots;

	Material originMateial;//原始的材质

	bool isMove;//是否已经移动过
	bool isFirstTwoStepMove;//是否第一次行棋而且是直进两个 just for pawn

	friend PlayerType DeterminePlayerType(PieceType pieceType);//确定玩家阵营
	friend void PrintPlayerType(PlayerType playerType);
	friend void PrintPieceType(PieceType pieceType);

private:
	Model *model;
	bool isPick;//是否被选中
	bool isPlayAnimation;//是否在播放动画（from prevPos to targetPos）
	bool needCalcCanGo;//是否已经计算完下一步行棋了

	ork::vec2f moveDir;//棋子移动方向 normalized
	float timer;//计数器
	float amount;//总移动时间
};

#endif //_PIECE_H