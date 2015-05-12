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
	void ResetCanGo();//���ÿ��о���

	ork::vec2i pickPos;//����ѡ�е����̸�

	float sideLen;//���̱��صĿ��
	float boardLen;//�������ĵĿ��
	float wholeLen;//�������̵Ŀ��
	float gridLen;//һ�����̸�Ŀ��

	ork::vec2f BoardPosToRawPos(ork::vec2f pos);//����������λ��
	ork::vec2f BoardPosToRawPos(ork::vec2i pos);//����������λ��

	AABB aabb;

	static Board *GetInstance();

private:
	bool canGo[ROW_NUM][ROW_NUM];//��һ���������ߵĸ��Ӿ���
	PieceType piecesState[ROW_NUM][ROW_NUM];//�������ӵİڷž���

	Model * pieceModels[PIECE_NUM];//����ģ��
	Model *model;
	Model *text;//��
	Model *step;//�����ߵĸ��ӱ�־��
	Model *scene;//����
	Model *window;//��������
	Model *cup;//������
	Model *chair;//����

	vector<ork::vec2i> stepPoses;//step��λ��

	static Board *instance;

	void InitPiecesModels();//��ʼ������ģ��
};

#endif //_BOARD_H