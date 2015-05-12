#ifndef _OVERLAY2D_H
#define  _OVERLAY2D_H

#include "geometry.h"

class Overlay2D
{
public:
	Overlay2D(std::string texturePath, ork::vec2f sizeRatio = ork::vec2f(1.0, 1.0f), float alpha = 1.0f);
	~Overlay2D();

	void Draw(int x, int y);
	ork::vec2i GetSize();

private:
	float alpha;
	ork::vec2f sizeRatio;
	ork::vec2i size;

	ork::ptr<ork::Program> program;
	ork::ptr< ork::Mesh<V2_UV, unsigned int> > mesh;
	ork::ptr<ork::Texture2D> texture;
};
#endif //_OVERLAY2D_H