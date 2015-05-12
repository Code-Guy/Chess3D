#include "overlay2d.h"
#include "fileloader.h"

using namespace std;
using namespace ork;

Overlay2D::Overlay2D(std::string texturePath, ork::vec2f sizeRatio, float alpha) 
	: alpha(alpha), sizeRatio(sizeRatio)
{
	texture = FileLoader::LoadTexture(GetTexturePath(texturePath));
	program = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("Overlay2DVS.glsl")),
		(char*)FileLoader::LoadFile(GetShaderPath("Overlay2DFS.glsl"))));

	program->getUniformSampler("myTextureSampler")->set(texture);
	program->getUniform1f("Alpha")->set(alpha);

	mesh = new Mesh<V2_UV, unsigned int>(TRIANGLES, GPU_STATIC);
	mesh->addAttributeType(0, 2, A32F, false);
	mesh->addAttributeType(1, 2, A32F, false);

	size = vec2i(texture->getWidth() * sizeRatio.x, texture->getHeight() * sizeRatio.y);
}

Overlay2D::~Overlay2D()
{

}

void Overlay2D::Draw(int x, int y)
{
	x *= ratioX;
	y *= ratioY;

	y = ScreenSize.y - y;//½ÃÕýÒ»ÏÂy

	float sizeX = size.x * ratioXY.x;
	float sizeY = size.y * ratioXY.y;

	mesh->clear();

	mesh->addVertex(V2_UV(x, y, 0, 0));
	mesh->addVertex(V2_UV(x, y + sizeY, 0, 1));
	mesh->addVertex(V2_UV(x + sizeX, y, 1, 0));

	mesh->addVertex(V2_UV(x + sizeX, y + sizeY, 1, 1));
	mesh->addVertex(V2_UV(x, y + sizeY, 0, 1));
	mesh->addVertex(V2_UV(x + sizeX, y, 1, 0));

	program->getUniform2f("ScreenSize")->set(ScreenSize);

	screenFb->setDepthMask(false);
	screenFb->setBlend(true, BlendEquation::ADD, BlendArgument::SRC_ALPHA, BlendArgument::ONE_MINUS_SRC_ALPHA);
	screenFb->draw(program, *mesh);
	screenFb->setBlend(false);
	screenFb->setDepthMask(true);
}

vec2i Overlay2D::GetSize()
{
	return size;
}