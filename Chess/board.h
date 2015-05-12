#ifndef _BOARD_H
#define  _BOARD_H

#include "model.h"
#include "piece.h"

#define ROW_NUM 8
#define PIECE_NUM 6
#define INVALID_POS ork::vec2i(-1, -1)
#define SIDE_OFFSET_RATIO 3.0 / 51.0

const Material StepUnpickMat(ork::vec3f(0.00, 0.1, 0.00), ork::vec3f(0.00, 0.8, 0.00), ork::vec3f(0.50, 0.50, 0.50));
const Material StepPickedMat(ork::vec3f(0.1, 0.1, 0.00), ork::vec3f(0.8, 0.8, 0.00), ork::vec3f(0.50, 0.50, 0.50));
const float StepAlpha = 0.5;

class Board
{
public:
	Board(Model *model);
	~Board();

	void Logic(float dt);
	void Render(ork::ptr<ork::FrameBuffer> fb, float dt);

	PieceType GetPiecesState(ork::vec2i pos);
	PieceType GetPiecesState(int i, int j);
	void SetPiecesState(ork::vec2i pos, PieceType type);
	void SetPiecesState(int i, int j, PieceType type);

	bool GetCanGo(ork::vec2i pos);
	bool GetCanGo(int i, int j);
	void SetCanGo(ork::vec2i pos, bool flag);
	void SetCanGo(int i, int j, bool flag);

	Model *GetPieceModel(PieceType pieceType);

	bool PickCanGo();
	void ResetCanGo();//重置可行矩阵

	ork::vec2i pickPos;//射线选中的棋盘格

	float sideLen;//棋盘边沿的宽度
	float boardLen;//棋盘中心的宽度
	float wholeLen;//整个棋盘的宽度
	float gridLen;//一个棋盘格的宽度

	ork::vec2f BoardPosToRawPos(ork::vec2f pos);//计算真正的位置
	ork::vec2f BoardPosToRawPos(ork::vec2i pos);//计算真正的位置

	AABB aabb;

	static Board *GetInstance();

private:
	bool canGo[ROW_NUM][ROW_NUM];//下一步可以行走的格子矩阵
	PieceType piecesState[ROW_NUM][ROW_NUM];//整体棋子的摆放局势

	Model * pieceModels[PIECE_NUM];//棋子模型
	Model *model;
	Model *text;//字
	Model *step;//可行走的格子标志物
	Model *scene;//场景
	Model *window;//窗户玻璃
	Model *cup;//玻璃杯
	Model *chair;//椅子

	vector<ork::vec2i> stepPoses;//step的位置

	static Board *instance;

	void InitPiecesModels();//初始化棋子模型
};

#endif //_BOARD_H