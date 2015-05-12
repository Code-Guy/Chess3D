#include "rule.h"
#include "board.h"
#include "player.h"
#include "mainwindow.h"

#include <cstdlib>

using namespace std;
using namespace ork;

Board *Rule::board;
Ally *Rule::ally;
Rival *Rule::rival;

void Rule::CalcCanGo(Piece *piece)//传入棋子的类型和位置，更新board的canGo和piecesState矩阵
{
	board->ResetCanGo();//重置可行矩阵
	switch (piece->pieceType)
	{
	case King:
	case EKing:
		KingCanGo(piece);
		break;
	case Queen:
	case EQueen:
		QueenCanGo(piece);
		break;
	case Rook:
	case ERook:
		RookCanGo(piece);
		break;
	case Bishop:
	case EBishop:
		BishopCanGo(piece);
		break;
	case Knight:
	case EKnight:
		KnightCanGo(piece);
		break;
	case Pawn:
	case EPawn:
		PawnCanGo(piece);
		break;
	}
}

bool Rule::IsCalcCanGo(Piece *piece, vec2i pos)//传入棋子的类型和位置，返回棋子是否能到达目标位置，不更新board的canGo和piecesState矩阵
{
	switch (piece->pieceType)
	{
	case King:
	case EKing:
		return IsKingCanGo(piece, pos);
	case Queen:
	case EQueen:
		return IsQueenCanGo(piece, pos);
	case Rook:
	case ERook:
		return IsRookCanGo(piece, pos);
	case Bishop:
	case EBishop:
		return IsBishopCanGo(piece, pos);
	case Knight:
	case EKnight:
		return IsKnightCanGo(piece, pos);
	case Pawn:
	case EPawn:
		return IsPawnCanGo(piece, pos);
	}
	return false;
}

void Rule::UpdateBoard(Piece *piece)//更新board状态 只针对ally方的 现在要针对ally和rival双方
{
	if (piece->pieceType != Pawn && piece->pieceType != EPawn)//兵在过路兵检测的时候再设置
	{
		//先把该棋子设为已运动的
		piece->isMove = true;
	}

	//1 把该棋子原来的位置置空
	board->SetPiecesState(piece->originPrevPos, None);

	//2 要考虑王（王车易位）
	//2.1 王车易位
	if (piece->pieceType == King || piece->pieceType == EKing)
	{
		board->SetPiecesState(piece->originPrevPos, None);

		int vertNum = piece->pieceType == King ? 7 : 0;
		PieceType rookType = piece->pieceType == King ? Rook : ERook;

		if (piece->prevPos.x - piece->originPrevPos.x == 2)
		{
			Piece *rook = ally->GetPiece(vec2i(7, vertNum));
			if (rook != NULL)
			{
				board->SetPiecesState(vec2i(7, vertNum), None);
				board->SetPiecesState(vec2i(piece->originPrevPos.x + 1, vertNum), rookType);
			}
		}
		else if (piece->prevPos.x - piece->originPrevPos.x == -2)
		{
			Piece *rook = ally->GetPiece(vec2i(0, vertNum));
			if (rook != NULL)
			{
				board->SetPiecesState(vec2i(0, vertNum), None);
				board->SetPiecesState(vec2i(piece->originPrevPos.x - 1, vertNum), rookType);
			}
		}
	}
	//2.2 吃过路兵
	else if (piece->pieceType == Pawn)
	{
		vec2i passPawnPos = vec2i(piece->prevPos.x, 3);
		if (piece->prevPos.y == 2 && IsPassPawn(passPawnPos))
		{
			rival->DeletePiece(passPawnPos);
			board->SetPiecesState(passPawnPos, None);
		}
		else if (DeterminePlayerType(board->GetPiecesState(piece->prevPos)) == piece->enemyType)
		{
			Player *enemy = GetEnemy(piece);
			enemy->DeletePiece(piece->prevPos);
		}
	}
	else if (piece->pieceType == EPawn)
	{
		vec2i passPawnPos = vec2i(piece->prevPos.x, 4);
		if (piece->prevPos.y == 5 && IsPassEPawn(passPawnPos))
		{
			ally->DeletePiece(passPawnPos);
			board->SetPiecesState(passPawnPos, None);
		}
		else if (DeterminePlayerType(board->GetPiecesState(piece->prevPos)) == piece->enemyType)
		{
			Player *enemy = GetEnemy(piece);
			enemy->DeletePiece(piece->prevPos);
		}
	}
	//2.3 如果当前位置原来有敌方的棋子，将其删除
	else if (DeterminePlayerType(board->GetPiecesState(piece->prevPos)) == piece->enemyType)
	{
		Player *enemy = GetEnemy(piece);
		MainWindow::GameState = enemy->DeletePiece(piece->prevPos);
	}

	//3 要考虑兵的升变和过路兵
	//3.1 兵的升变 过路兵
	if (piece->pieceType == Pawn)//是兵并且已经达到对方底线
	{
		if (piece->prevPos.y == 0)
		{
			//现在的做法是随机升变 因为不好做交互选择T_T
			RandomPawnPromotion(piece, AllyType);//随机兵的升变
		}
		else if (!piece->isMove && abs(piece->prevPos.y - piece->originPrevPos.y) == 2)//没动过并且第一次跑了两格
		{
			piece->isMove = true;
			piece->isFirstTwoStepMove = true;
		}
		else
		{
			piece->isFirstTwoStepMove = false;
		}
	}
	else if (piece->pieceType == EPawn)//是兵并且已经达到对方底线
	{
		if (piece->prevPos.y == 0)
		{
			//现在的做法是随机升变 因为不好做交互选择T_T
			RandomPawnPromotion(piece, RivalType);//随机兵的升变
		}
		else if (!piece->isMove && abs(piece->prevPos.y - piece->originPrevPos.y) == -2)//没动过并且第一次跑了两格
		{
			piece->isMove = true;
			piece->isFirstTwoStepMove = true;
		}
		else
		{
			piece->isFirstTwoStepMove = false;
		}
	}

	//3.2 将当前位置置成自己的类型
	board->SetPiecesState(piece->prevPos, piece->pieceType);
}

vector<vec2i> Rule::KingCanGo(Piece *king, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置

	vec2i kingPos = king->prevPos;
	//遍历九宫格
	for (int i = kingPos.x - 1; i <= kingPos.x + 1; i++)
	{
		for (int j = kingPos.y - 1; j <= kingPos.y + 1; j++)
		{
			//3个条件 1.不和自身重合 2.位置不超出棋盘格 3.目标位置不是自己的棋子
			if (!(i == kingPos.x && j == kingPos.y) && IsPosValid(i, j) && DeterminePlayerType(board->GetPiecesState(i, j)) != king->friendType)
			{
				canGoPoses.push_back(vec2i(i, j));
			}
		}
	}
	if (!king->isMove)//如果King已经走过 //如果King没有走过，考虑王车易位
	{
		Piece *rook1 = NULL;
		Piece *rook2 = NULL;
		GetRooks(rook1, rook2, king->friendType);//得到我方的两个rook
		vec2i canGoPos1 = Castling(king, rook1);
		vec2i canGoPos2 = Castling(king, rook2);

		if (canGoPos1 != INVALID_POS)
		{
			canGoPoses.push_back(canGoPos1);
		}
		if (canGoPos2 != INVALID_POS)
		{
			canGoPoses.push_back(canGoPos2);
		}
	}

	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

vector<vec2i> Rule::QueenCanGo(Piece *queen, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置
	vec2i queenPos = queen->prevPos;//获取queen的位置
	//横向 左
	for (int i=queenPos.x - 1; i>=0; i--)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(i, queenPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, queenPos.y));
	}
	//横向 右
	for (int i=queenPos.x + 1; i<ROW_NUM; i++)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(i, queenPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, queenPos.y));
	}
	//竖向 上（我方视角）
	for (int j=queenPos.y - 1; j>=0; j--)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(queenPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(queenPos.x, j));
	}
	//竖向 下（我方视角）
	for (int j=queenPos.y + 1; j<ROW_NUM; j++)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(queenPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(queenPos.x, j));
	}
	//斜向左上（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos - vec2i(i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//斜向左下（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	//斜向右上（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(i, -i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//斜向左下（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(-i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

vector<vec2i> Rule::RookCanGo(Piece *rook, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置
	vec2i rookPos = rook->prevPos;//获取queen的位置

	//横向 左
	for (int i=rookPos.x - 1; i>=0; i--)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(i, rookPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, rookPos.y));
	}
	//横向 右
	for (int i=rookPos.x + 1; i<ROW_NUM; i++)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(i, rookPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, rookPos.y));
	}
	//竖向 上（我方视角）
	for (int j=rookPos.y - 1; j>=0; j--)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(rookPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(rookPos.x, j));
	}
	//竖向 下（我方视角）
	for (int j=rookPos.y + 1; j<ROW_NUM; j++)
	{
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(vec2i(rookPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(rookPos.x, j));
	}

	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

vector<vec2i> Rule::BishopCanGo(Piece *bishop, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置
	vec2i bishopPos = bishop->prevPos;

	//斜向上（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos - vec2i(i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//斜向下（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	//斜向上（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(i, -i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//斜向下（我方视角）
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(-i, i);
		if (!IsPosValid(candidatePos))//出了棋盘，跳出循环
		{
			break;
		}
		//如果该格是我方，直接跳出循环
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//如果该格是敌方，可通行区域加入该敌方，跳出循环
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

vector<vec2i> Rule::KnightCanGo(Piece *knight, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置
	vec2i knightPos = knight->prevPos;

	//knight能走的所有区域（偏移量）
	vec2i posOffsets[8] = {vec2i(-2, -1), vec2i(-1, -2), vec2i(1, -2), vec2i(2, -1),
					  vec2i(-2, 1), vec2i(-1, 2), vec2i(1, 2), vec2i(2, 1)};

	for (int i=0; i<8; i++)
	{
		vec2i pos = knightPos + posOffsets[i];
		if (IsPosValid(pos))//没有超过棋盘
		{
			if (knight->friendType != DeterminePlayerType(board->GetPiecesState(pos)))
			{
				canGoPoses.push_back(pos);
			}
		}
	}

	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

vector<vec2i> Rule::PawnCanGo(Piece *pawn, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//能够达到的位置
	vector<vec2i> rawCanGoPoses;//大概能够达到的位置，后面要进行筛选

	vec2i pawnPos = pawn->prevPos;

	bool isAlly = pawn->friendType == AllyType;
	//先考虑直走
	if (!pawn->isMove)//pawn没动过，可走两格
	{
		if (isAlly)//如果是我方
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, -1));
			rawCanGoPoses.push_back(pawnPos + vec2i(0, -2));
		}
		else
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, 1));
			rawCanGoPoses.push_back(pawnPos + vec2i(0, 2));
		}
	}
	else
	{
		if (isAlly)//如果是我方
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, -1));
		}
		else
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, 1));
		}
	}
	
	//再考虑斜吃
	//可以斜吃的位置
	vec2i eatPos1;
	vec2i eatPos2;
	if (isAlly)//如果是我方
	{
		eatPos1 = pawnPos + vec2i(-1, -1);
		eatPos2 = pawnPos + vec2i(1, -1);
	}
	else
	{
		eatPos1 = pawnPos + vec2i(-1, 1);
		eatPos2 = pawnPos + vec2i(1, 1);
	}
	if (IsPosValid(eatPos1))
	{
		if (pawn->enemyType == DeterminePlayerType(board->GetPiecesState(eatPos1)))
		{
			canGoPoses.push_back(eatPos1);
		}
	}
	if (IsPosValid(eatPos2) && pawn->enemyType == DeterminePlayerType(board->GetPiecesState(eatPos2)))
	{
		canGoPoses.push_back(eatPos2);
	}

	for (size_t i=0; i<rawCanGoPoses.size(); i++)
	{
		if (board->GetPiecesState(rawCanGoPoses[i]) == None)
		{
			canGoPoses.push_back(rawCanGoPoses[i]);
		}
	}

	if (rawCanGoPoses.size() == 2 && board->GetPiecesState(rawCanGoPoses.front()) != None)
	{
		canGoPoses.pop_back();
	}

	if (isUpdateCanGo)
	{
		for(size_t i=0; i<canGoPoses.size(); i++)
		{
			board->SetCanGo(canGoPoses[i], true);
		}
	}
	return canGoPoses;
}

//判断(我方/敌方)棋子能否走到指定的格子
bool Rule::IsKingCanGo(Piece *king, vec2i pos)
{
	return ContainPos(KingCanGo(king, false), pos);
}

bool Rule::IsQueenCanGo(Piece *queen, vec2i pos)
{
	return ContainPos(QueenCanGo(queen, false), pos);
}

bool Rule::IsRookCanGo(Piece *rook, vec2i pos)
{
	return ContainPos(RookCanGo(rook, false), pos);
}

bool Rule::IsBishopCanGo(Piece *bishop, vec2i pos)
{
	return ContainPos(BishopCanGo(bishop, false), pos);
}

bool Rule::IsKnightCanGo(Piece *knight, vec2i pos)
{
	return ContainPos(KnightCanGo(knight, false), pos);
}

bool Rule::IsPawnCanGo(Piece *pawn, vec2i pos)
{
	return ContainPos(PawnCanGo(pawn, false), pos);
}

bool Rule::IsPosValid(int x, int y)
{
	return x >= 0 && x < ROW_NUM && y >= 0 && y < ROW_NUM;
}

bool Rule::IsPosValid(vec2i pos)
{
	return pos.x >= 0 && pos.x < ROW_NUM && pos.y >= 0 && pos.y < ROW_NUM;
}

void Rule::PrintPos(vec2i pos)
{
	printf("(%d, %d)\n", pos.x, pos.y);
}

void Rule::GetRooks(Piece *rook1, Piece *rook2, PlayerType friendType)//获取我方的两个rook
{
	int count = 0;//统计rook的个数
	for (size_t i = 0; i < ally->pieces.size(); i++)
	{
		if (ally->pieces[i]->pieceType == Rook)
		{
			if (count == 0)
			{
				rook1 = ally->pieces[i];
				count++;
			}
			else if (count == 1)
			{
				rook2 = ally->pieces[i];
				count++;
			}
		}
	}
}

vector<vec2i> Rule::GetPosesBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction)//获取两颗棋子之间的格子(纵向/横向)
{
	vector<vec2i> poses;
	poses.clear();

	//获取两颗棋子的位置
	vec2i pos1 = p1->prevPos;
	vec2i pos2 = p2->prevPos;

	if (direction == Horizontal)
	{
		if (pos1.y != pos2.y)//横向但是y不一样，输入错误
		{
			return poses;
		}

		int v = pos1.y;
		int lowerH = pos1.x < pos2.x ? pos1.x : pos2.x;
		int upperH = pos1.x > pos2.x ? pos1.x : pos2.x;

		for (int i = lowerH + 1; i < upperH; i++)
		{
			poses.push_back(vec2i(i, v));
		}
	}
	else if (direction == Vertical)
	{
		if (pos1.x != pos2.x)//竖向但是x不一样，输入错误
		{
			return poses;
		}
		int h = pos1.x;
		int lowerV = pos1.y < pos2.y ? pos1.y : pos2.y;
		int upperV = pos1.y > pos2.y ? pos1.y : pos2.y;

		for (int i = lowerV + 1; i < upperV; i++)
		{
			poses.push_back(vec2i(h, i));
		}
	}

	return poses;
}

bool Rule::IsPieceBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction)//两颗棋子之间是否有其他棋子阻隔(纵向/横向)
{
	vector<vec2i> poses = GetPosesBetweenTwoPieces(p1, p2, direction);

	for (size_t i = 0; i < poses.size(); i++)
	{
		if (board->GetPiecesState(poses[i]) != None)
		{
			return true;
		}
	}
	return false;
}

bool Rule::IsChecked(Piece *king, vec2i offset)//是否被将军(我方/敌方) 偏移量
{
	if (king->pieceType == King)
	{
		//遍历对方的棋子
		for (size_t i=0; i<rival->pieces.size(); i++)
		{
			if(IsCalcCanGo(rival->pieces[i], king->prevPos + offset))
				return true;
		}
	}
	else if (king->pieceType == EKing)
	{
		//遍历对方的棋子
		for (size_t i=0; i<ally->pieces.size(); i++)
		{
			if(IsCalcCanGo(ally->pieces[i], king->prevPos + offset))
				return true;
		}
	}
	return false;
}

vec2i Rule::Castling(Piece *king, Piece *rook)//王车易位
{
	if (rook != NULL && !rook->isMove)//如果rook1存在并且没动过
	{
		if (!IsPieceBetweenTwoPieces(king, rook, Horizontal))//如果水平方向上王和车之间没有棋子
		{
			vector<vec2i> poses;//王能到达的位置（偏移量）
			poses.push_back(vec2i::ZERO);//note!需要加上王当前的位置
			if (rook->prevPos.x < king->prevPos.x)//是左rook for ally 右rook for rival
			{
				poses.push_back(vec2i(-1, 0));
				poses.push_back(vec2i(-2, 0));
			}
			else if(rook->prevPos.x > king->prevPos.x)//是右rook for ally 左rook for rival
			{
				poses.push_back(vec2i(1, 0));
				poses.push_back(vec2i(2, 0));
			}
			for (size_t i = 0; i < poses.size(); i++)
			{
				if (IsChecked(king, poses[i]))
				{
					return INVALID_POS;
				}
			}

			vec2i canGoPos = king->prevPos + poses.back();
			return canGoPos;
		}
	}
	return INVALID_POS;
}

bool Rule::IsPassPawn(vec2i pos)//该位置是不是一个过路兵
{
	Piece *p = rival->GetPiece(pos);
	return p != NULL && p->pieceType == EPawn && p->isFirstTwoStepMove;
}

bool Rule::IsPassEPawn(ork::vec2i pos)//该位置是不是一个过路兵
{
	Piece *p = ally->GetPiece(pos);
	return p != NULL && p->pieceType == Pawn && p->isFirstTwoStepMove;
}

bool Rule::ContainPos(const vector<vec2i> &poses, vec2i pos)//可到达格子中是否存在某一特定格子
{
	for (size_t i = 0; i < poses.size(); i++)
	{
		if (pos.x == poses[i].x && pos.y == poses[i].y)
		{
			return true;
		}
	}
	return false;
}

Player *Rule::GetFriend(Piece *piece)
{
	if (piece->friendType == AllyType)
	{
		return ally;
	}
	else 
	{
		return rival;
	}
}

Player *Rule::GetEnemy(Piece *piece)
{
	if (piece->friendType == AllyType)
	{
		return rival;
	}
	else 
	{
		return ally;
	}
}

int Rule::Random(int n)//得到0-n-1的随机数
{
	return rand() % n;
}

void Rule::RandomPawnPromotion(Piece *pawn, PlayerType type)//随机兵的升变
{
	PieceType queen = (type == AllyType) ? Queen : EQueen;
	PieceType rook = (type == AllyType) ? Rook : ERook;
	PieceType knight = (type == AllyType) ? Knight : EKnight;
	PieceType bishop = (type == AllyType) ? Bishop : EBishop;

	int random = Random(4);
	if (random == 0)
	{
		pawn->ReBorn(board->GetPieceModel(queen), queen);
	}
	else if (random == 1)
	{
		pawn->ReBorn(board->GetPieceModel(rook), rook);
	}
	else if (random == 2)
	{
		pawn->ReBorn(board->GetPieceModel(knight), knight);
	}
	else 
	{
		pawn->ReBorn(board->GetPieceModel(bishop), bishop);
	}
}