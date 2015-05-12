#ifndef _TEXT2D_H
#define  _TEXT2D_H

#include "geometry.h"

class Text2D
{
public:
	static void Init(std::string texturePath);
	static void Print(std::string text, int x, int y, int size, ork::vec3f textColor = ork::vec3f(0.0f, 0.0f, 0.0f), float spanRatio = 0.3f);

private:
	static ork::ptr<ork::Program> program;
	static ork::ptr< ork::Mesh<V2_UV, unsigned int> > mesh;
	static ork::ptr<ork::Texture2D> texture;
};

#endif //_TEXT2D_H