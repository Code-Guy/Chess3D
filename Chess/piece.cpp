#include "piece.h"
#include "board.h"
#include "rule.h"

using namespace ork;

Piece::Piece(Model *model, PieceType pieceType, vec2i originPos, vec3f rotate) : model(model), pieceType(pieceType), originPos(originPos), rotate(rotate)
{
	model->GetOctrees(roots);//�õ����а˲���
	friendType = DeterminePlayerType(pieceType);
	if (friendType == AllyType)
	{
		enemyType = RivalType;
	}
	else
	{
		enemyType = AllyType;
	}

	aabb = model->boundAABB;
	translate = vec3f::ZERO;

	prevPos = originPos;
	targetPos = originPos;
	originPrevPos = originPos;
	curPos = vec2f(originPos.x, originPos.y);

	isPick = false;
	isMove = false;
	isFirstTwoStepMove = false;
	isPlayAnimation = false;
	needCalcCanGo = false;

	moveDir = vec2f::ZERO;//�����ƶ����� normalized
	timer = 0;//������
	amount = 0;//���ƶ�ʱ��
}

Piece::~Piece()
{
	
}

void Piece::Logic(float dt)
{
	if (needCalcCanGo)//�����ѡ�еĻ�
	{
		Rule::CalcCanGo(this);
		needCalcCanGo = false;
	}
	if (isPlayAnimation)//������ڲ��Ŷ���
	{
		timer += dt;
		if (timer > amount)//����Լ��Ƶ���Ŀ��λ��
		{
			originPrevPos = prevPos;
			prevPos = targetPos;
			curPos = vec2f(targetPos.x, targetPos.y);
			isPlayAnimation = false;
			isPick = false;
			Rule::UpdateBoard(this);
		}
		else
		{
			//prevPos�ĳ�curPos������Ч������������
			curPos += moveDir * ANIMATION_SPEED * dt;//����ÿ֡�ƶ�������
		}
	}
}

void Piece::Render(ptr<FrameBuffer> fb, float dt)
{
	vec2f rawPos = Board::GetInstance()->BoardPosToRawPos(curPos);

	if (isPick)
	{
		Material pickMaterial;
		pickMaterial.ambientColor = originMateial.ambientColor;
		pickMaterial.diffuseColor = PICK_MATERIAL_ENHANCE;
		pickMaterial.specularColor = originMateial.specularColor;

		model->SetMaterial(pickMaterial);
	}

	translate = vec3f(rawPos.x, 0, rawPos.y);

	model->Render(fb, translate, rotate);
	model->SetMaterial(originMateial);
}

void Piece::SetMaterial(Material material)
{
	originMateial = material;
	model->SetMaterial(originMateial);
}

void Piece::SetTargetPos(vec2i targetPos)
{
	this->targetPos = targetPos;
	isPlayAnimation = true;

	vec2f direction = vec2f(targetPos.x, targetPos.y) - vec2f(prevPos.x, prevPos.y);//�ܵ��˶�����

	moveDir = direction.normalize();
	timer = 0;//������
	amount = direction.length() / ANIMATION_SPEED;//���ƶ�ʱ��
}

void Piece::ReBorn(Model *model, PieceType pieceType)
{
	this->model = model;
	this->pieceType = pieceType;

	roots.clear();
	model->GetOctrees(roots);//�õ����а˲���
	friendType = DeterminePlayerType(pieceType);
	if (friendType == AllyType)
	{
		enemyType = RivalType;
	}
	else
	{
		enemyType = AllyType;
	}

	aabb = model->boundAABB;
	translate = vec3f::ZERO;

	isPick = false;
	isMove = false;
	isFirstTwoStepMove = false;
	isPlayAnimation = false;
	needCalcCanGo = false;
}

bool Piece::IsPlayAnimation()
{
	return isPlayAnimation;
}

void Piece::Pick()
{
	isPick = true;
	needCalcCanGo = true;
}

void Piece::UnPick()
{
	isPick = false;
}

PlayerType DeterminePlayerType(PieceType pieceType)//ȷ�������Ӫ
{
	switch (pieceType)
	{
	case King:
	case Queen:
	case Rook:
	case Bishop:
	case Knight:
	case Pawn:
		return AllyType;
	case EKing:
	case EQueen:
	case ERook:
	case EBishop:
	case EKnight:
	case EPawn:
		return RivalType;
	}
	return NoneType; 
}

void PrintPlayerType(PlayerType playerType)
{
	switch(playerType)
	{
	case AllyType:
		printf("Ally\n");
		break;
	case RivalType:
		printf("Rival\n");
		break;
	case NoneType:
		printf("None\n");
		break;
	}
}

void PrintPieceType(PieceType pieceType)
{
	switch (pieceType)
	{
	case King:
		printf("King\n");
		break;
	case Queen:
		printf("Queen\n");
		break;
	case Rook:
		printf("Rook\n");
		break;
	case Bishop:
		printf("Bishop\n");
		break;
	case Knight:
		printf("Knight\n");
		break;
	case Pawn:
		printf("Pawn\n");
		break;
	case EKing:
		printf("EKing\n");
		break;
	case EQueen:
		printf("EQueen\n");
		break;
	case ERook:
		printf("ERook\n");
	case EBishop:
		printf("EBishop\n");
		break;
	case EKnight:
		printf("EKnight\n");
		break;
	case EPawn:
		printf("EPawn\n");
		break;
	case None:
		printf("None\n");
		break;
	}
}