#ifndef _MODEL_H
#define  _MODEL_H

#include "mesh.h"
#include "global.h"

enum DebugFrameMode {AABBMode, OctreeMode};

class Model
{
public:
	Model(std::string modelName, bool isGlass = false, bool isAxes = false, bool isPiece = false);
	~Model();

	void SetDiffuseTexture(ork::ptr<ork::Texture2D> diffuseTexture);
	void SetNormalTexture(ork::ptr<ork::Texture2D> normalTexture);
	void SetSpecularTexture(ork::ptr<ork::Texture2D> specularTexture);

	void SetTextureCube(ork::ptr<ork::TextureCube> textureCube);
	void SetMaterial(Material material);
	void SetAlpha(float alpha);
	void SetBlend(bool flag);
	void SetShadowMap(bool flag);

	void GetOctrees(std::vector<OctreeNode *> &roots);

	void Render(ork::ptr<ork::FrameBuffer> fb,
		ork::vec3f translate = ork::vec3f::ZERO, ork::vec3f rotate = ork::vec3f::ZERO, ork::vec3f scale = ork::vec3f(1.0, 1.0, 1.0));

	AABB boundAABB;

	ork::mat4f M;
	ork::vec3f translate;

	static void SetRenderDepth(bool flag);

private:
	ork::ptr<ork::Program> program;
	ork::ptr<ork::Program> renderDepthProgram;

	std::vector<OrkMesh> meshes;

	ork::vec3f rotate;
	ork::vec3f scale;

	bool isGlass;//是否为玻璃材质
	bool isAxes;//是否为坐标轴
	bool isPiece;//是否是棋子

	bool isBlend;//是否要混合
	bool isShadowMap;//是否投影

	float alpha;//透明度

	static bool renderDepth;//是否写到深度

	void CalcBoundAABB();//计算包围盒
	void RenderDebugFrame(DebugFrameMode mode);//画debug用的框架
};

#endif //_MODEL_H