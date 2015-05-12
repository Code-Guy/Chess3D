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

void Rule::CalcCanGo(Piece *piece)//�������ӵ����ͺ�λ�ã�����board��canGo��piecesState����
{
	board->ResetCanGo();//���ÿ��о���
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

bool Rule::IsCalcCanGo(Piece *piece, vec2i pos)//�������ӵ����ͺ�λ�ã����������Ƿ��ܵ���Ŀ��λ�ã�������board��canGo��piecesState����
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

void Rule::UpdateBoard(Piece *piece)//����board״̬ ֻ���ally���� ����Ҫ���ally��rival˫��
{
	if (piece->pieceType != Pawn && piece->pieceType != EPawn)//���ڹ�·������ʱ��������
	{
		//�ȰѸ�������Ϊ���˶���
		piece->isMove = true;
	}

	//1 �Ѹ�����ԭ����λ���ÿ�
	board->SetPiecesState(piece->originPrevPos, None);

	//2 Ҫ��������������λ��
	//2.1 ������λ
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
	//2.2 �Թ�·��
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
	//2.3 �����ǰλ��ԭ���ез������ӣ�����ɾ��
	else if (DeterminePlayerType(board->GetPiecesState(piece->prevPos)) == piece->enemyType)
	{
		Player *enemy = GetEnemy(piece);
		MainWindow::GameState = enemy->DeletePiece(piece->prevPos);
	}

	//3 Ҫ���Ǳ�������͹�·��
	//3.1 �������� ��·��
	if (piece->pieceType == Pawn)//�Ǳ������Ѿ��ﵽ�Է�����
	{
		if (piece->prevPos.y == 0)
		{
			//���ڵ�������������� ��Ϊ����������ѡ��T_T
			RandomPawnPromotion(piece, AllyType);//�����������
		}
		else if (!piece->isMove && abs(piece->prevPos.y - piece->originPrevPos.y) == 2)//û�������ҵ�һ����������
		{
			piece->isMove = true;
			piece->isFirstTwoStepMove = true;
		}
		else
		{
			piece->isFirstTwoStepMove = false;
		}
	}
	else if (piece->pieceType == EPawn)//�Ǳ������Ѿ��ﵽ�Է�����
	{
		if (piece->prevPos.y == 0)
		{
			//���ڵ�������������� ��Ϊ����������ѡ��T_T
			RandomPawnPromotion(piece, RivalType);//�����������
		}
		else if (!piece->isMove && abs(piece->prevPos.y - piece->originPrevPos.y) == -2)//û�������ҵ�һ����������
		{
			piece->isMove = true;
			piece->isFirstTwoStepMove = true;
		}
		else
		{
			piece->isFirstTwoStepMove = false;
		}
	}

	//3.2 ����ǰλ���ó��Լ�������
	board->SetPiecesState(piece->prevPos, piece->pieceType);
}

vector<vec2i> Rule::KingCanGo(Piece *king, bool isUpdateCanGo)
{
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��

	vec2i kingPos = king->prevPos;
	//�����Ź���
	for (int i = kingPos.x - 1; i <= kingPos.x + 1; i++)
	{
		for (int j = kingPos.y - 1; j <= kingPos.y + 1; j++)
		{
			//3������ 1.���������غ� 2.λ�ò��������̸� 3.Ŀ��λ�ò����Լ�������
			if (!(i == kingPos.x && j == kingPos.y) && IsPosValid(i, j) && DeterminePlayerType(board->GetPiecesState(i, j)) != king->friendType)
			{
				canGoPoses.push_back(vec2i(i, j));
			}
		}
	}
	if (!king->isMove)//���King�Ѿ��߹� //���Kingû���߹�������������λ
	{
		Piece *rook1 = NULL;
		Piece *rook2 = NULL;
		GetRooks(rook1, rook2, king->friendType);//�õ��ҷ�������rook
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
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��
	vec2i queenPos = queen->prevPos;//��ȡqueen��λ��
	//���� ��
	for (int i=queenPos.x - 1; i>=0; i--)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(i, queenPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, queenPos.y));
	}
	//���� ��
	for (int i=queenPos.x + 1; i<ROW_NUM; i++)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, queenPos.y)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(i, queenPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, queenPos.y));
	}
	//���� �ϣ��ҷ��ӽǣ�
	for (int j=queenPos.y - 1; j>=0; j--)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(queenPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(queenPos.x, j));
	}
	//���� �£��ҷ��ӽǣ�
	for (int j=queenPos.y + 1; j<ROW_NUM; j++)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(queenPos.x, j)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(queenPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(queenPos.x, j));
	}
	//б�����ϣ��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos - vec2i(i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//б�����£��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	//б�����ϣ��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(i, -i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//б�����£��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = queenPos + vec2i(-i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == queen->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
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
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��
	vec2i rookPos = rook->prevPos;//��ȡqueen��λ��

	//���� ��
	for (int i=rookPos.x - 1; i>=0; i--)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(i, rookPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, rookPos.y));
	}
	//���� ��
	for (int i=rookPos.x + 1; i<ROW_NUM; i++)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(i, rookPos.y)) == rook->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(i, rookPos.y));
			break;
		}
		canGoPoses.push_back(vec2i(i, rookPos.y));
	}
	//���� �ϣ��ҷ��ӽǣ�
	for (int j=rookPos.y - 1; j>=0; j--)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(vec2i(rookPos.x, j));
			break;
		}
		canGoPoses.push_back(vec2i(rookPos.x, j));
	}
	//���� �£��ҷ��ӽǣ�
	for (int j=rookPos.y + 1; j<ROW_NUM; j++)
	{
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(rookPos.x, j)) == rook->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
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
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��
	vec2i bishopPos = bishop->prevPos;

	//б���ϣ��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos - vec2i(i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//б���£��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}
	//б���ϣ��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(i, -i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
			canGoPoses.push_back(candidatePos);
			break;
		}
		canGoPoses.push_back(candidatePos);
	}

	//б���£��ҷ��ӽǣ�
	for(int i=1; ; i++)
	{
		vec2i candidatePos = bishopPos + vec2i(-i, i);
		if (!IsPosValid(candidatePos))//�������̣�����ѭ��
		{
			break;
		}
		//����ø����ҷ���ֱ������ѭ��
		if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->friendType)
		{
			break;
		}
		else if (DeterminePlayerType(board->GetPiecesState(candidatePos)) == bishop->enemyType)
		{
			//����ø��ǵз�����ͨ���������õз�������ѭ��
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
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��
	vec2i knightPos = knight->prevPos;

	//knight���ߵ���������ƫ������
	vec2i posOffsets[8] = {vec2i(-2, -1), vec2i(-1, -2), vec2i(1, -2), vec2i(2, -1),
					  vec2i(-2, 1), vec2i(-1, 2), vec2i(1, 2), vec2i(2, 1)};

	for (int i=0; i<8; i++)
	{
		vec2i pos = knightPos + posOffsets[i];
		if (IsPosValid(pos))//û�г�������
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
	vector<vec2i> canGoPoses;//�ܹ��ﵽ��λ��
	vector<vec2i> rawCanGoPoses;//����ܹ��ﵽ��λ�ã�����Ҫ����ɸѡ

	vec2i pawnPos = pawn->prevPos;

	bool isAlly = pawn->friendType == AllyType;
	//�ȿ���ֱ��
	if (!pawn->isMove)//pawnû��������������
	{
		if (isAlly)//������ҷ�
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
		if (isAlly)//������ҷ�
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, -1));
		}
		else
		{
			rawCanGoPoses.push_back(pawnPos + vec2i(0, 1));
		}
	}
	
	//�ٿ���б��
	//����б�Ե�λ��
	vec2i eatPos1;
	vec2i eatPos2;
	if (isAlly)//������ҷ�
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

//�ж�(�ҷ�/�з�)�����ܷ��ߵ�ָ���ĸ���
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

void Rule::GetRooks(Piece *rook1, Piece *rook2, PlayerType friendType)//��ȡ�ҷ�������rook
{
	int count = 0;//ͳ��rook�ĸ���
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

vector<vec2i> Rule::GetPosesBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction)//��ȡ��������֮��ĸ���(����/����)
{
	vector<vec2i> poses;
	poses.clear();

	//��ȡ�������ӵ�λ��
	vec2i pos1 = p1->prevPos;
	vec2i pos2 = p2->prevPos;

	if (direction == Horizontal)
	{
		if (pos1.y != pos2.y)//������y��һ�����������
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
		if (pos1.x != pos2.x)//������x��һ�����������
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

bool Rule::IsPieceBetweenTwoPieces(Piece *p1, Piece *p2, Direction direction)//��������֮���Ƿ��������������(����/����)
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

bool Rule::IsChecked(Piece *king, vec2i offset)//�Ƿ񱻽���(�ҷ�/�з�) ƫ����
{
	if (king->pieceType == King)
	{
		//�����Է�������
		for (size_t i=0; i<rival->pieces.size(); i++)
		{
			if(IsCalcCanGo(rival->pieces[i], king->prevPos + offset))
				return true;
		}
	}
	else if (king->pieceType == EKing)
	{
		//�����Է�������
		for (size_t i=0; i<ally->pieces.size(); i++)
		{
			if(IsCalcCanGo(ally->pieces[i], king->prevPos + offset))
				return true;
		}
	}
	return false;
}

vec2i Rule::Castling(Piece *king, Piece *rook)//������λ
{
	if (rook != NULL && !rook->isMove)//���rook1���ڲ���û����
	{
		if (!IsPieceBetweenTwoPieces(king, rook, Horizontal))//���ˮƽ���������ͳ�֮��û������
		{
			vector<vec2i> poses;//���ܵ����λ�ã�ƫ������
			poses.push_back(vec2i::ZERO);//note!��Ҫ��������ǰ��λ��
			if (rook->prevPos.x < king->prevPos.x)//����rook for ally ��rook for rival
			{
				poses.push_back(vec2i(-1, 0));
				poses.push_back(vec2i(-2, 0));
			}
			else if(rook->prevPos.x > king->prevPos.x)//����rook for ally ��rook for rival
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

bool Rule::IsPassPawn(vec2i pos)//��λ���ǲ���һ����·��
{
	Piece *p = rival->GetPiece(pos);
	return p != NULL && p->pieceType == EPawn && p->isFirstTwoStepMove;
}

bool Rule::IsPassEPawn(ork::vec2i pos)//��λ���ǲ���һ����·��
{
	Piece *p = ally->GetPiece(pos);
	return p != NULL && p->pieceType == Pawn && p->isFirstTwoStepMove;
}

bool Rule::ContainPos(const vector<vec2i> &poses, vec2i pos)//�ɵ���������Ƿ����ĳһ�ض�����
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

int Rule::Random(int n)//�õ�0-n-1�������
{
	return rand() % n;
}

void Rule::RandomPawnPromotion(Piece *pawn, PlayerType type)//�����������
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