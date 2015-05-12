#include "board.h"
#include "fileloader.h"

using namespace ork;

Board *Board::instance = NULL;

Board::Board(Model *model) : model(model)
{
	aabb = model->boundAABB;

	sideLen = (aabb.xMax - aabb.xMin) * SIDE_OFFSET_RATIO;//棋盘边缘大小
	boardLen = (aabb.xMax - aabb.xMin) * (1 - 2 * SIDE_OFFSET_RATIO);//8*8棋盘格总大小 
	wholeLen = aabb.xMax - aabb.xMin;//整个棋盘的宽度
	gridLen = boardLen / ROW_NUM;//一个棋盘格的宽度

	//初始化所有棋盘格都不能通行
	for (int i = 0; i < ROW_NUM; i++)
	{
		for (int j = 0; j < ROW_NUM; j++)
		{
			canGo[i][j] = false;
		}
	}

	//初始化整体棋子的摆放局势
	piecesState[0][0] = ERook;  piecesState[1][0] = EKnight; piecesState[2][0] = EBishop; piecesState[3][0] = EQueen;
	piecesState[4][0] = EKing;  piecesState[5][0] = EBishop; piecesState[6][0] = EKnight; piecesState[7][0] = ERook;
	piecesState[0][1] = EPawn;  piecesState[1][1] = EPawn; piecesState[2][1] = EPawn; piecesState[3][1] = EPawn; 
	piecesState[4][1] = EPawn;  piecesState[5][1] = EPawn; piecesState[6][1] = EPawn; piecesState[7][1] = EPawn;
	piecesState[0][2] = None;  piecesState[1][2] = None; piecesState[2][2] = None; piecesState[3][2] = None;
	piecesState[4][2] = None;  piecesState[5][2] = None; piecesState[6][2] = None; piecesState[7][2] = None;
	piecesState[0][3] = None;  piecesState[1][3] = None; piecesState[2][3] = None; piecesState[3][3] = None;
	piecesState[4][3] = None;  piecesState[5][3] = None; piecesState[6][3] = None; piecesState[7][3] = None;
	piecesState[0][4] = None;  piecesState[1][4] = None; piecesState[2][4] = None; piecesState[3][4] = None;
	piecesState[4][4] = None;  piecesState[5][4] = None; piecesState[6][4] = None; piecesState[7][4] = None;
	piecesState[0][5] = None;  piecesState[1][5] = None; piecesState[2][5] = None; piecesState[3][5] = None;
	piecesState[4][5] = None;  piecesState[5][5] = None; piecesState[6][5] = None; piecesState[7][5] = None;
	piecesState[0][6] = Pawn;  piecesState[1][6] = Pawn; piecesState[2][6] = Pawn; piecesState[3][6] = Pawn;
	piecesState[4][6] = Pawn;  piecesState[5][6] = Pawn; piecesState[6][6] = Pawn; piecesState[7][6] = Pawn;
	piecesState[0][7] = Rook;  piecesState[1][7] = Knight; piecesState[2][7] = Bishop; piecesState[3][7] = King;
	piecesState[4][7] = Queen;  piecesState[5][7] = Bishop; piecesState[6][7] = Knight; piecesState[7][7] = Rook;
	
	InitPiecesModels();//初始化棋子模型

	text = new Model("text.obj");
	scene = new Model("scene.obj");
	chair = new Model("chair.obj");

	window = new Model("window.obj", true);
	cup = new Model("cup.obj", true);
	window->SetTextureCube(glassTex);
	window->SetAlpha(0.7);
	cup->SetTextureCube(skyboxTex);
	cup->SetAlpha(0.7);

 	step = new Model("step.obj");
 	step->SetMaterial(StepUnpickMat);
	step->SetAlpha(0.7);
	step->SetBlend(true);
}

Board::~Board()
{
 	delete model;
 	delete text;
 	delete scene;
	delete chair;
}

void Board::Logic(float dt)
{
	stepPoses.clear();
	for (int i = 0; i < ROW_NUM; i++)
	{
		for (int j = 0; j < ROW_NUM; j++)
		{
			if(canGo[i][j])
				stepPoses.push_back(vec2i(i, j));
		}
	}
}

void Board::Render(ptr<FrameBuffer> fb, float dt)
{
	model->Render(fb);
	text->Render(fb);
	scene->Render(fb);
	chair->Render(fb);

	window->Render(fb);
	cup->Render(fb, vec3f(1.2, 0, 0.3));

	//画step
	for(size_t i=0; i<stepPoses.size(); i++)
	{
		if (stepPoses[i] == pickPos)
		{
			step->SetMaterial(StepPickedMat);
		}
		vec2f rawStepPos = BoardPosToRawPos(stepPoses[i]);
		step->Render(fb, vec3f(rawStepPos.x, 0, rawStepPos.y));
		step->SetMaterial(StepUnpickMat);
	}
}

PieceType Board::GetPiecesState(vec2i pos)
{
	return piecesState[pos.x][pos.y];
}

PieceType Board::GetPiecesState(int i, int j)
{
	return piecesState[i][j];
}

void Board::SetPiecesState(vec2i pos, PieceType type)
{
	piecesState[pos.x][pos.y] = type;
}

void Board::SetPiecesState(int i, int j, PieceType type)
{
	piecesState[i][j] = type;
}

bool Board::GetCanGo(vec2i pos)
{
	return canGo[pos.x][pos.y];
}

bool Board::GetCanGo(int i, int j)
{
	return canGo[i][j];
}

void Board::SetCanGo(vec2i pos, bool flag)
{
	canGo[pos.x][pos.y] = flag;
}

void Board::SetCanGo(int i, int j, bool flag)
{
	canGo[i][j] = flag;
}

Model *Board::GetPieceModel(PieceType pieceType)
{
	switch (pieceType)
	{
	case King:
	case EKing:
		return pieceModels[3];
	case Queen:
	case EQueen:
		return pieceModels[4];
	case Rook:
	case ERook:
		return pieceModels[0];
	case Bishop:
	case EBishop:
		return pieceModels[2];
	case Knight:
	case EKnight:
		return pieceModels[1];
	case Pawn:
	case EPawn:
		return pieceModels[5];
	}
	return NULL;
}

bool Board::PickCanGo()
{
	if (pickPos == INVALID_POS)
	{
		return false;
	}
	return canGo[pickPos.x][pickPos.y];
}

vec2f Board::BoardPosToRawPos(vec2f pos)//计算真正的位置
{
	return vec2f((pos.x + 0.5) * gridLen - 0.5 * boardLen, (pos.y + 0.5) * gridLen - 0.5 * boardLen);
}

vec2f Board::BoardPosToRawPos(vec2i pos)//计算真正的位置
{
	return vec2f((pos.x + 0.5) * gridLen - 0.5 * boardLen, (pos.y + 0.5) * gridLen - 0.5 * boardLen);
}

Board *Board::GetInstance()
{
	if (instance == NULL)
	{
		instance = new Board(new Model("Board.obj"));
	}

	return instance;
}

void Board::InitPiecesModels()//初始化棋子模型
{
	pieceModels[0] = new Model("rook.obj", false, false, true);
	pieceModels[1] = new Model("knight.obj", false, false, true);
	pieceModels[2] = new Model("bishop.obj", false, false, true);
	pieceModels[3] = new Model("king.obj", false, false, true);
	pieceModels[4] = new Model("queen.obj", false, false, true);
	pieceModels[5] = new Model("pawn.obj", false, false, true);
}

void Board::ResetCanGo()//重置可行矩阵
{
	//初始化所有棋盘格都不能通行
	for (int i = 0; i < ROW_NUM; i++)
	{
		for (int j = 0; j < ROW_NUM; j++)
		{
			canGo[i][j] = false;
		}
	}
}