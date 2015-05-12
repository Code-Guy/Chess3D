#include "text2d.h"
#include "fileloader.h"

using namespace std;
using namespace ork;

ptr<Program> Text2D::program = NULL;
ptr< Mesh<V2_UV, unsigned int> > Text2D::mesh = NULL;
ptr<Texture2D> Text2D::texture = NULL;

void Text2D::Init(std::string texturePath)
{
	texture = FileLoader::LoadTexture(GetTexturePath(texturePath));
	program = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("Text2DVS.glsl")),
		(char*)FileLoader::LoadFile(GetShaderPath("Text2DFS.glsl"))));

	program->getUniformSampler("myTextureSampler")->set(texture);
	
	mesh = new Mesh<V2_UV, unsigned int>(TRIANGLES, GPU_STATIC);
	mesh->addAttributeType(0, 2, A32F, false);
	mesh->addAttributeType(1, 2, A32F, false);
}

void Text2D::Print(std::string text, int x, int y, int size, vec3f textColor, float spanRatio)
{
	x *= ratioX;
	y *= ratioY;
	size *= ratio;

	int len = text.length();

	mesh->clear();

	float span = size * spanRatio;

	for (int i=0; i<len; i++)
	{
		vec2f vertex_up_left    = vec2f(x+i*span     , y+size);
		vec2f vertex_up_right   = vec2f(x+i*span + size, y+size);
		vec2f vertex_down_right = vec2f(x+i*span + size, y     );
		vec2f vertex_down_left  = vec2f(x+i*span     , y     );

		char character = text[i];
		float uv_x = (character % 16) / 16.0f;
		float uv_y = (character / 16) / 16.0f;

		vec2f uv_up_left    = vec2f(uv_x           , uv_y);
		vec2f uv_up_right   = vec2f(uv_x+1.0f/16.0f, uv_y);
		vec2f uv_down_right = vec2f(uv_x+1.0f/16.0f, uv_y + 1.0f/16.0f);
		vec2f uv_down_left  = vec2f(uv_x           , uv_y + 1.0f/16.0f);

		mesh->addVertex(V2_UV(vertex_up_left.x, vertex_up_left.y, uv_up_left.x, uv_up_left.y));
		mesh->addVertex(V2_UV(vertex_down_left.x, vertex_down_left.y, uv_down_left.x, uv_down_left.y));
		mesh->addVertex(V2_UV(vertex_up_right.x, vertex_up_right.y, uv_up_right.x, uv_up_right.y));

		mesh->addVertex(V2_UV(vertex_down_right.x, vertex_down_right.y, uv_down_right.x, uv_down_right.y));
		mesh->addVertex(V2_UV(vertex_down_left.x, vertex_down_left.y, uv_down_left.x, uv_down_left.y));
		mesh->addVertex(V2_UV(vertex_up_right.x, vertex_up_right.y, uv_up_right.x, uv_up_right.y));
	}

	program->getUniform2f("ScreenSize")->set(ScreenSize);
	program->getUniform3f("TextColor")->set(textColor);

	screenFb->setDepthMask(false);
	screenFb->setBlend(true, BlendEquation::ADD, BlendArgument::SRC_ALPHA, BlendArgument::ONE_MINUS_SRC_ALPHA);
	screenFb->draw(program, *mesh);
	screenFb->setBlend(false);
	screenFb->setDepthMask(true);
}