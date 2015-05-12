#include "geometry.h"

using namespace std;
using namespace ork;

vector<vec3f> VBO::vertices;
vector<vec2f> VBO::uvs;
vector<vec3f> VBO::normals;

AABB::AABB(float xMin, float xMax,
	float yMin, float yMax,
	float zMin, float zMax)
	: xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax)
{
	
}

AABB::~AABB()
{

}

void AABB::Inverse()
{
	xMin = yMin = zMin = MAXNUM;
	xMax = yMax = zMax = -MAXNUM;
}

void AABB::TranslateSelf(vec3f offset)
{
	xMin += offset.x;
	xMax += offset.x;

	yMin += offset.y;
	yMax += offset.y;

	zMin += offset.z;
	zMax += offset.z;
}

AABB AABB::Translate(vec3f offset) const
{
	return AABB(xMin + offset.x, xMax + offset.x,
		yMin + offset.y, yMax + offset.y,
		zMin + offset.z, zMax + offset.z);
}

bool AABB::IsVertexIn(const vec3f &v) const
{
	return v.x > this->xMin && v.x < this->xMax &&
		v.y > this->yMin && v.y < this->yMax &&
		v.z > this->zMin && v.z < this->zMax;
}

bool IsFloatEqual(float a, float b)
{
	return abs(a - b) < EPSILON;
}