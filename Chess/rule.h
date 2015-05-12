#ifndef _RULE_H
#define  _RULE_H

#include "ork/math/vec2.h"
#include "piece.h"

enum Direction
{
	Horizontal, Vertical
};

class Board;//Board��ǰ������
class Player;
class Ally;
class Rival;
class Rule
{
public:
	static void CalcCanGo(Piece *piece);//�������ӵ����ͺ�λ�ã�����board��canGo��piecesState����
	static bool IsCalcCanGo(Piece *piece, ork::vec2i pos);//�������ӵ����ͺ�λ�ã����������Ƿ��ܵ���Ŀ��λ�ã�������board��canGo��piecesState����

	static void UpdateBoard(Piece *piece);//����board״̬

	static bool IsPosValid(int x, int y);
	static bool IsPosValid(ork::vec2i pos);
	static void PrintPos(ork::vec2i pos);
	static int Random(int n);//�õ�0-n-1�������

	static Board *board;
	static Ally *ally;
	static Rival *rival;

private:
	//����(�ҷ�/�з�)�����ܹ��ߵĸ��� �������ߵ��ĸ���
	static vector<ork::vec2i> KingCanGo(Piece *king, bool isUpdateCanGo = true);
	static vector<ork::vec2i> QueenCanGo(Piece *queen, bool isUpdateCanGo = true);
	static vector<ork::vec2i> RookCanGo(Piece *rook, bool isUpdateCanGo = true);
	static vector<ork::vec2i> BishopCanGo(Piece *bishop, bool isUpdateCanGo = true);
	static vector<ork::vec2i> KnightCanGo(Piece *knight, bool isUpdateCanGo = true);
	static vector<ork::vec2i> PawnCanGo(Piece *pawn, bool isUpdateCanGo = true);

	//�ж�(�ҷ�/�з�)�����ܷ��ߵ�ָ���ĸ���
	static bool IsKingCanGo(Piece *king, ork::vec2i pos);
	static bool IsQueenCanGo(Piece *queen, ork::vec2i pos);
	static bool IsRookCanGo(Piece *rook, ork::vec2i pos);
	static bool IsBishopCanGo(Piece *bishop, ork::vec2i pos);
	static bool IsKnightCanGo(Piece *knight, ork::vec2i pos);
	static bool IsPawnCanGo(Piece *pawn, ork::vec2i pos);

	static void GetRooks(Piece *rook1, Piece *rook2, PlayerType friendType);//��ȡ�ҷ�������rook (�ҷ�/�з�)
	static vector<ork::vec2i> GetPosesBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction);//��ȡ��������֮��ĸ���(����/����)
	static bool IsPieceBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction);//��������֮���Ƿ��������������(����/����)
	static bool IsChecked(Piece *king, ork::vec2i offset = ork::vec2i::ZERO);//�Ƿ񱻽���(�ҷ�/�з�) ƫ����
	static ork::vec2i Castling(Piece *king, Piece *rook);//������λ
	static bool IsPassPawn(ork::vec2i pos);//��λ���ǲ���һ����·��
	static bool IsPassEPawn(ork::vec2i pos);//��λ���ǲ���һ����·��

	static bool ContainPos(const vector<ork::vec2i> &poses, ork::vec2i pos);//�ɵ���������Ƿ����ĳһ�ض�����

	static Player *GetFriend(Piece *piece);
	static Player *GetEnemy(Piece *piece);

	static void RandomPawnPromotion(Piece *pawn, PlayerType type);//�����������
};

#endif //_RULE_H