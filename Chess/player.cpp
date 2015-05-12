#include "player.h"
#include "rule.h"
#include "packet.h"

using namespace std;
using namespace ork;

Player::Player()
{
	name = "Unknown";
	isOnTheOffsensive = -1;//���ֺ���δ��
}

Player::~Player()
{
	
}

Piece *Player::GetPiece(vec2i pos)
{
	for (size_t i=0; i<pieces.size(); i++)
	{
		if (pieces[i]->prevPos == pos)
		{
			return pieces[i];
		}
	}
	return NULL;
}

int Player::DeletePiece(vec2i pos)
{
	int ret = 0;
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); )
	{
		if ((*pIter)->prevPos == pos)//��������Ҫ��ɾ��
		{
			if ((*pIter)->pieceType == EKing)//ɱ���з�����ʤ��
			{
				ret = 1;
			}
			if ((*pIter)->pieceType == King)//ɱ���ҷ�����ʧ��
			{
				ret = -1;
			}
			delete (*pIter);
			pIter = pieces.erase(pIter);
		}
		else
		{
			pIter++;
		}
	}

	return ret;
}

int Player::DeletePiece(Piece *p)
{
	int ret = 0;
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); )
	{
		if ((*pIter)->prevPos == p->prevPos)//��������Ҫ��ɾ��
		{
			if ((*pIter)->pieceType == EKing)//ɱ���з�����ʤ��
			{
				ret = 1;
			}
			if ((*pIter)->pieceType == King)//ɱ���ҷ�����ʧ��
			{
				ret = -1;
			}
			delete (*pIter);
			pIter = pieces.erase(pIter);
		}
		else
		{
			pIter++;
		}
	}

	return ret;
}

void Player::SetName(const char *name)
{
	this->name = string(name);
}

string Player::GetName()
{
	return name;
}

void Player::SetOffsensive(int flag)//�趨���ֺ���
{
	isOnTheOffsensive = flag;
}

int Player::GetOffsensive()//�������ֺ���
{
	return isOnTheOffsensive;
}

string Player::GetOffsensiveStr()
{
    if (isOnTheOffsensive == 0)
	{
		return "Defensive";
	}
	else if (isOnTheOffsensive == 1)
	{
		return "Offsensive";
	}
	else
	{
		return "Undefined";
	}
}

Ally *Ally::instance = NULL;

Ally::Ally()
{
	pickPieceIndex = -1;
	isPieceMove = false;
}

Ally::~Ally()
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		delete (*pIter);
	}

	pieces.clear();
}

void Ally::Init()//��ʼ��
{
	//��ʼ������
	board = Board::GetInstance();

	//��ʼ������
	pieces.push_back(new Piece(board->GetPieceModel(Rook), Rook, vec2i(0, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Knight), Knight, vec2i(1, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Bishop), Bishop, vec2i(2, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(King), King, vec2i(3, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Queen), Queen, vec2i(4, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Bishop), Bishop, vec2i(5, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Knight), Knight, vec2i(6, 7), vec3f(0, 180, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(Rook), Rook, vec2i(7, 7), vec3f(0, 180, 0)));

	for (int i = 0; i < 8; i++)
	{
		pieces.push_back(new Piece(board->GetPieceModel(Pawn), Pawn, vec2i(i, 6), vec3f(0, 180, 0)));
	}
}

void Ally::Logic(float dt)
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		(*pIter)->Logic(dt);
	}

	board->Logic(dt);
}

void Ally::Render(ptr<FrameBuffer> fb, float dt)
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		(*pIter)->SetMaterial(AllyMat);
		(*pIter)->Render(fb, dt);
	}

	//ray.Render(dt);
	board->Render(fb, dt);
}

void Ally::RayCast(int x, int y, int mouseState)//mouseState 0��UP 1:DOWN
{
	//�õ�Ͷ������
	ray = CalcPickRay(x, y);
	if(mouseState == 1)//DOWN ��������̵��ཻ����
	{
		board->pickPos = ray.RayCastWithBoard(board);
		//���û�㵽���и��ӣ��ͺ��������ཻ����
		if (!board->PickCanGo())
		{
			board->pickPos = INVALID_POS;//����û�㵽���и���

			//��ʼ��board�Ŀ�������
			board->ResetCanGo();
		}
		else //����㵽���ǿ��и��ӣ��Ͳ����������ӵ��ཻ������ 
		{
			isPieceMove = true;
			//����֮ǰ��ѡ�����ӵĶ���
			pieces[pickPieceIndex]->SetTargetPos(board->pickPos);
		}
	}
	else if (mouseState == 0)//UP ����������ཻ����
	{
		//�����������ӣ������һ�����Ӵ��ڲ��Ŷ���״̬�������ཻ����
		for (size_t i=0; i<pieces.size(); i++)
		{
			if (pieces[i]->IsPlayAnimation())
			{
				//��ʼ��board�Ŀ�������
				board->ResetCanGo();
				return;
			}
		}

		pickPieceIndex = -1;//invalid piece

		//��ȡ�����������ӵ�ѡ��
		for (size_t i=0; i<pieces.size(); i++)
		{
			pieces[i]->UnPick();
		}

		for (size_t i = 0; i < pieces.size(); i++)
		{
			if (ray.RayCastWithPiece(pieces[i], OctreeMode))
			{
				pickPieceIndex = i;
				pieces[i]->Pick();
				break;
			}
		}
	}
}

Message Ally::GetAllyPieceMoveMsg()//�õ�Ally�������ƶ���Ϣ
{
	Message msg;
	msg.prevPos = INVALID_POS;
	msg.targetPos = INVALID_POS;
	msg.isOpen = false;

	if (isPieceMove)
	{
		//�����������ӣ������һ�����Ӵ��ڲ��Ŷ���״̬���������ƶ���Ϣ
		for (size_t i=0; i<pieces.size(); i++)
		{
			if (pieces[i]->IsPlayAnimation())
			{
				msg.prevPos = pieces[i]->prevPos;
				msg.targetPos = pieces[i]->targetPos;

				isPieceMove = false;
				return msg;
			}
		}
	}
	
	return msg;
}

void Ally::RandomOffsensive()//����������ֺ���
{
	SetOffsensive(Rule::Random(2));
}

Ally *Ally::GetInstance()
{
	if (instance == NULL)
	{
		instance = new Ally();
	}

	return instance;
}

Rival *Rival::instance = NULL;

Rival::Rival()
{
}

Rival::~Rival()
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		delete (*pIter);
	}

	pieces.clear();
}

void Rival::Init()//��ʼ��
{
	//��ʼ������
	board = Board::GetInstance();

	//��ʼ������
	pieces.push_back(new Piece(board->GetPieceModel(ERook), ERook, vec2i(0, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EKnight), EKnight, vec2i(1, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EBishop), EBishop, vec2i(2, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EKing), EKing, vec2i(3, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EQueen), EQueen, vec2i(4, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EBishop), EBishop, vec2i(5, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(EKnight), EKnight, vec2i(6, 0)));
	pieces.push_back(new Piece(board->GetPieceModel(ERook), ERook, vec2i(7, 0)));

	for (int i = 0; i < 8; i++)
	{
		pieces.push_back(new Piece(board->GetPieceModel(EPawn), EPawn, vec2i(i, 1)));
	}
}

void Rival::Logic(float dt)
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		(*pIter)->Logic(dt);
	}
}

void Rival::Render(ptr<FrameBuffer> fb, float dt)
{
	for (vector<Piece *>::iterator pIter = pieces.begin(); pIter != pieces.end(); pIter++)
	{
		(*pIter)->SetMaterial(RivalMat);
		(*pIter)->Render(fb, dt);
	}
}

void Rival::DriveByMsg(Message msg)
{
	vec2i prevPos = AdjustEnemyPos(msg.prevPos);
	vec2i targetPos = AdjustEnemyPos(msg.targetPos);

	if (Rule::IsPosValid(prevPos) && Rule::IsPosValid(targetPos))
	{
		Piece *p = GetPiece(prevPos);//��ȡ��λ���ϵ�����
		p->SetTargetPos(targetPos);
	}
}

vec2i Rival::AdjustEnemyPos(vec2i pos)
{
	return vec2i(pos.x, ROW_NUM - pos.y - 1);
}

Rival *Rival::GetInstance()
{
	if (instance == NULL)
	{
		instance = new Rival();
	}

	return instance;
}