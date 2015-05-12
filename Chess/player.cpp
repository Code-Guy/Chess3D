#include "player.h"
#include "rule.h"
#include "packet.h"

using namespace std;
using namespace ork;

Player::Player()
{
	name = "Unknown";
	isOnTheOffsensive = -1;//先手后手未定
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
		if ((*pIter)->prevPos == pos)//该棋子需要被删除
		{
			if ((*pIter)->pieceType == EKing)//杀死敌方王，胜利
			{
				ret = 1;
			}
			if ((*pIter)->pieceType == King)//杀死我方王，失败
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
		if ((*pIter)->prevPos == p->prevPos)//该棋子需要被删除
		{
			if ((*pIter)->pieceType == EKing)//杀死敌方王，胜利
			{
				ret = 1;
			}
			if ((*pIter)->pieceType == King)//杀死我方王，失败
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

void Player::SetOffsensive(int flag)//设定先手后手
{
	isOnTheOffsensive = flag;
}

int Player::GetOffsensive()//返回先手后手
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

void Ally::Init()//初始化
{
	//初始化棋盘
	board = Board::GetInstance();

	//初始化棋子
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

void Ally::RayCast(int x, int y, int mouseState)//mouseState 0：UP 1:DOWN
{
	//得到投射射线
	ray = CalcPickRay(x, y);
	if(mouseState == 1)//DOWN 处理和棋盘的相交测试
	{
		board->pickPos = ray.RayCastWithBoard(board);
		//如果没点到可行格子，就和棋子做相交测试
		if (!board->PickCanGo())
		{
			board->pickPos = INVALID_POS;//还是没点到可行格子

			//初始化board的可行区域
			board->ResetCanGo();
		}
		else //如果点到的是可行格子，就不再做和棋子的相交测试了 
		{
			isPieceMove = true;
			//播放之前被选中棋子的动画
			pieces[pickPieceIndex]->SetTargetPos(board->pickPos);
		}
	}
	else if (mouseState == 0)//UP 处理和棋子相交测试
	{
		//遍历所有棋子，如果有一个棋子处于播放动画状态，不做相交测试
		for (size_t i=0; i<pieces.size(); i++)
		{
			if (pieces[i]->IsPlayAnimation())
			{
				//初始化board的可行区域
				board->ResetCanGo();
				return;
			}
		}

		pickPieceIndex = -1;//invalid piece

		//先取消对所有棋子的选中
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

Message Ally::GetAllyPieceMoveMsg()//得到Ally的棋子移动信息
{
	Message msg;
	msg.prevPos = INVALID_POS;
	msg.targetPos = INVALID_POS;
	msg.isOpen = false;

	if (isPieceMove)
	{
		//遍历所有棋子，如果有一个棋子处于播放动画状态，返回其移动信息
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

void Ally::RandomOffsensive()//随机生成先手后手
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

void Rival::Init()//初始化
{
	//初始化棋盘
	board = Board::GetInstance();

	//初始化棋子
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
		Piece *p = GetPiece(prevPos);//获取该位置上的棋子
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