#include "ray.h"
#include "fileloader.h"
#include "rule.h"
#include <GL/glut.h>

using namespace ork;

Ray::Ray(const vec3f &p, const vec3f &dir)
   :p(p), dir(dir.normalize())
{
	lineProgram = new Program(new Module(330, (char*)FileLoader::LoadFile(GetShaderPath("LineVS.glsl")),
		(char*)FileLoader::LoadFile(GetShaderPath("LineFS.glsl"))));

	lineProgram->getUniform3f("LineColor")->set(RAY_COLOR);

	mesh = new Mesh<vec3f, unsigned int>(LINES, GPU_STATIC);
	mesh->addAttributeType(0, 3, A32F, false);
}

Ray::~Ray()
{
}

vec2i Ray::RayCastWithBoard(const Board *board)//和棋盘包围盒相交，返回相交到那一个格子
{
	AABB aabb = board->aabb;

	vec2f pY = XZPlaneJoint(aabb.yMax);//获得射线与棋盘包围盒上平面的交点
	if (IsJointOnSide(pY, aabb.xMin, aabb.xMax, aabb.zMin, aabb.zMax, SIDE_OFFSET_RATIO, SIDE_OFFSET_RATIO))
	{
		vec2i pickPos;
		pickPos.x = (pY.x - aabb.xMin - board->sideLen) / board->gridLen;
		pickPos.y = (pY.y - aabb.zMin - board->sideLen) / board->gridLen;

		if (Rule::IsPosValid(pickPos))
		{
			return pickPos;
		}
		return INVALID_POS;
	}
	else
	{
		return INVALID_POS;
	}
}

bool Ray::RayCastWithPiece(const Piece *piece, DebugFrameMode mode)//和棋子包围盒相交，返回是否相交
{
	if (mode == AABBMode)
	{
		return IntersectWithAABB(piece->aabb, piece->translate);
	}
	else
	{
		for (size_t i=0; i<piece->roots.size(); i++)
		{
			if (IntersectWithOctree(piece->roots[i], piece->translate))
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

//光线与XY平面 XZ平面 YZ平面
vec2f Ray::XYPlaneJoint(float z) const
{
	float t = (z - this->p.z) / this->dir.z;
	
	if (t < 0)
		return vec2f::ZERO;

	return vec2f(this->p.x + t * this->dir.x, this->p.y + t * this->dir.y);
}

vec2f Ray::XZPlaneJoint(float y) const
{
	float t = (y - this->p.y) / this->dir.y;

	if (t < 0)
		return vec2f::ZERO;

	return vec2f(this->p.x + t * this->dir.x, this->p.z + t * this->dir.z);
}

vec2f Ray::YZPlaneJoint(float x) const
{
	float t = (x - this->p.x) / this->dir.x;

	if (t < 0)
		return vec2f::ZERO;

	return vec2f(this->p.y + t * this->dir.y, this->p.z + t * this->dir.z);
}

bool Ray::IsJointOnSide(vec2f joint, float min1, float max1, float min2, float max2, 
	float sideOffsetRatio1, float sideOffsetRatio2) const//判断交点是否在面上
{
	float sideOffset1 = (max1 - min1) * sideOffsetRatio1;
	float sideOffset2 = (max2 - min2) * sideOffsetRatio2;

	return joint.x > min1 + sideOffsetRatio1 && joint.x < max1 - sideOffsetRatio1 && 
		joint.y > min2 + sideOffsetRatio2 && joint.y < max2 - sideOffsetRatio2;
}

Ray CalcPickRay(int x, int y)//计算射线
{
	GLint viewPort[4];
	GLdouble mvMatrix[16];
	GLdouble projMatrix[16];

	GLdouble wX, wY, wZ;

	//视口 左上角x y坐标 宽度 高度
	vec4i viewPortVec = screenFb->getViewport();
	viewPort[0] = viewPortVec.x;
	viewPort[1] = viewPortVec.y;
	viewPort[2] = viewPortVec.z;
	viewPort[3] = viewPortVec.w;

	//模型视图矩阵 投影矩阵
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mvMatrix[4 * i + j] = camera->V[j][i];
			projMatrix[4 * i + j] = camera->P[j][i];
		}
	}

	int rectifyY = viewPort[3] - y - 1;//-1??

	//映射
	gluUnProject(x, rectifyY, 1.0, mvMatrix, projMatrix, viewPort, &wX, &wY, &wZ);

	vec3f p = camera->pos;
	vec3f w = vec3f(wX, wY, wZ);

	vec3f dir = (w - p).normalize();

	return Ray(p, dir);
}

void Ray::Render(float dt)
{
	mesh->clear();
	mesh->addVertex(p);
	mesh->addVertex(p + dir * RENDER_LEN);

	lineProgram->getUniformMatrix4f("MVP")->setMatrix(camera->VP);

	screenFb->draw(lineProgram, *mesh);
}

bool Ray::IntersectWithAABB(AABB aabb, vec3f translate) const
{
	aabb.TranslateSelf(translate);

	vec2f pZ1 = XYPlaneJoint(aabb.zMin);
	vec2f pZ2 = XYPlaneJoint(aabb.zMax);

	vec2f pY1 = XZPlaneJoint(aabb.yMin);
	vec2f pY2 = XZPlaneJoint(aabb.yMax);

	vec2f pX1 = YZPlaneJoint(aabb.xMin);
	vec2f pX2 = YZPlaneJoint(aabb.xMax);

	if ((pZ1 != vec2f::ZERO && IsJointOnSide(pZ1, aabb.xMin, aabb.xMax, aabb.yMin, aabb.yMax)) ||
		(pZ2 != vec2f::ZERO && IsJointOnSide(pZ2, aabb.xMin, aabb.xMax, aabb.yMin, aabb.yMax)))
		return true;
	if ((pY1 != vec2f::ZERO && IsJointOnSide(pY1, aabb.xMin, aabb.xMax, aabb.zMin, aabb.zMax)) ||
		(pY2 != vec2f::ZERO && IsJointOnSide(pY2, aabb.xMin, aabb.xMax, aabb.zMin, aabb.zMax)))
		return true;
	if ((pX1 != vec2f::ZERO && IsJointOnSide(pX1, aabb.yMin, aabb.yMax, aabb.zMin, aabb.zMax)) ||
		(pX2 != vec2f::ZERO && IsJointOnSide(pX2, aabb.yMin, aabb.yMax, aabb.zMin, aabb.zMax)))
		return true;

	return false;
}

bool Ray::IntersectWithOctree(OctreeNode *root, vec3f translate) const
{
	if (root != NULL)
	{
		if (root->isLeaf && !root->IsEmpty())
		{
			if (IntersectWithAABB(root->aabb, translate))
			{
				return true;
			}
		}

		for (int i=0; i<8; i++)
		{
			if(IntersectWithOctree(root->children[i], translate))
				return true;
		}
		return false;
	}
	return false;
}