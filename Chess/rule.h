#ifndef _RULE_H
#define  _RULE_H

#include "ork/math/vec2.h"
#include "piece.h"

enum Direction
{
	Horizontal, Vertical
};

class Board;//Board的前置声明
class Player;
class Ally;
class Rival;
class Rule
{
public:
	static void CalcCanGo(Piece *piece);//传入棋子的类型和位置，更新board的canGo和piecesState矩阵
	static bool IsCalcCanGo(Piece *piece, ork::vec2i pos);//传入棋子的类型和位置，返回棋子是否能到达目标位置，不更新board的canGo和piecesState矩阵

	static void UpdateBoard(Piece *piece);//更新board状态

	static bool IsPosValid(int x, int y);
	static bool IsPosValid(ork::vec2i pos);
	static void PrintPos(ork::vec2i pos);
	static int Random(int n);//得到0-n-1的随机数

	static Board *board;
	static Ally *ally;
	static Rival *rival;

private:
	//计算(我方/敌方)棋子能够走的格子 返回能走到的格子
	static vector<ork::vec2i> KingCanGo(Piece *king, bool isUpdateCanGo = true);
	static vector<ork::vec2i> QueenCanGo(Piece *queen, bool isUpdateCanGo = true);
	static vector<ork::vec2i> RookCanGo(Piece *rook, bool isUpdateCanGo = true);
	static vector<ork::vec2i> BishopCanGo(Piece *bishop, bool isUpdateCanGo = true);
	static vector<ork::vec2i> KnightCanGo(Piece *knight, bool isUpdateCanGo = true);
	static vector<ork::vec2i> PawnCanGo(Piece *pawn, bool isUpdateCanGo = true);

	//判断(我方/敌方)棋子能否走到指定的格子
	static bool IsKingCanGo(Piece *king, ork::vec2i pos);
	static bool IsQueenCanGo(Piece *queen, ork::vec2i pos);
	static bool IsRookCanGo(Piece *rook, ork::vec2i pos);
	static bool IsBishopCanGo(Piece *bishop, ork::vec2i pos);
	static bool IsKnightCanGo(Piece *knight, ork::vec2i pos);
	static bool IsPawnCanGo(Piece *pawn, ork::vec2i pos);

	static void GetRooks(Piece *rook1, Piece *rook2, PlayerType friendType);//获取我方的两个rook (我方/敌方)
	static vector<ork::vec2i> GetPosesBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction);//获取两颗棋子之间的格子(纵向/横向)
	static bool IsPieceBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction);//两颗棋子之间是否有其他棋子阻隔(纵向/横向)
	static bool IsChecked(Piece *king, ork::vec2i offset = ork::vec2i::ZERO);//是否被将军(我方/敌方) 偏移量
	static ork::vec2i Castling(Piece *king, Piece *rook);//王车易位
	static bool IsPassPawn(ork::vec2i pos);//该位置是不是一个过路兵
	static bool IsPassEPawn(ork::vec2i pos);//该位置是不是一个过路兵

	static bool ContainPos(const vector<ork::vec2i> &poses, ork::vec2i pos);//可到达格子中是否存在某一特定格子

	static Player *GetFriend(Piece *piece);
	static Player *GetEnemy(Piece *piece);

	static void RandomPawnPromotion(Piece *pawn, PlayerType type);//随机兵的升变
};

#endif //_RULE_H