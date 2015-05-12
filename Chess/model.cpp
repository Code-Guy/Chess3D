#include "model.h"
#include "fileloader.h"

using namespace std;
using namespace ork;

bool Model::renderDepth = false;//是否写到深度

Model::Model(string modelName, bool isGlass, bool isAxes, bool isPiece)
	: isGlass(isGlass), isAxes(isAxes), isPiece(isPiece)
{
	alpha = 1;//默认为不透明

	FileLoader::LoadOrkMesh(GetModelPath(modelName), meshes, isPiece);
	CalcBoundAABB();//计算包围盒

	if (isGlass)//是否是玻璃材质
	{
		program = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("GlassVS.glsl")),
			(char*)FileLoader::LoadFile(GetShaderPath("GlassFS.glsl"))));
	}
	else 
	{
		program = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("PhongVS.glsl")),
			(char*)FileLoader::LoadFile(GetShaderPath("PhongFS.glsl"))));

		program->getUniform3f("LightPosition_worldspace1")->set(light1.pos);
		program->getUniform3f("LightColor1")->set(light1.color);
		program->getUniform1f("LightPower1")->set(light1.power);

		program->getUniform3f("LightPosition_worldspace2")->set(light2.pos);
		program->getUniform3f("LightColor2")->set(light2.color);
		program->getUniform1f("LightPower2")->set(light2.power);
	}

	renderDepthProgram = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("RenderDepthVS.glsl")),
		(char*)FileLoader::LoadFile(GetShaderPath("RenderDepthFS.glsl"))));

	isBlend = false;
	isShadowMap = false;//默认打开投影
}

Model::~Model()
{

}

void Model::SetDiffuseTexture(ptr<Texture2D> diffuseTexture)
{
	for (vector<OrkMesh>::iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		mIter->diffuseTexture = diffuseTexture;
	}
}

void Model::SetNormalTexture(ptr<Texture2D> normalTexture)
{
	for (vector<OrkMesh>::iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		mIter->normalTexture = normalTexture;
	}
}

void Model::SetSpecularTexture(ptr<Texture2D> specularTexture)
{
	for (vector<OrkMesh>::iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		mIter->specularTexture = specularTexture;
	}
}

void Model::SetTextureCube(ptr<TextureCube> textureCube)
{
	if (isGlass)
	{
		program->getUniformSampler("CubemapTex")->set(textureCube);
	}
}

void Model::SetMaterial(Material material)
{
	for (vector<OrkMesh>::iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		mIter->material = material;
	}
}

void Model::SetAlpha(float alpha)
{
	this->alpha = alpha;
}

void Model::SetBlend(bool flag)
{
	isBlend = flag;
}

void Model::SetShadowMap(bool flag)
{
	isShadowMap = flag;
}

void Model::GetOctrees(vector<OctreeNode *> &roots)
{
	roots.clear();
	for (vector<OrkMesh>::iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		roots.push_back(mIter->root);
	}
}

void Model::Render(ptr<FrameBuffer> fb, vec3f translate, vec3f rotate, vec3f scale)
{
	this->translate = translate;
	M = CalcModelMatrix(translate, rotate, scale);

	if (renderDepth)//如果要渲染到深度图
	{
		if (isShadowMap)
		{
			renderDepthProgram->getUniformMatrix4f("MVP")->setMatrix(camera->GetLightVP(light2) * M);
			for (vector<OrkMesh>::const_iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
			{
				fb->draw(renderDepthProgram, *(mIter->mesh));
			}
		}
	}
	else
	{
		program->getUniform1f("Alpha")->set(alpha);

		if (isGlass)
		{
			program->getUniformMatrix4f("VP")->setMatrix(camera->VP);
			program->getUniformMatrix4f("M")->setMatrix(M);
			program->getUniformMatrix4f("N")->setMatrix((camera->V * M).inverse().transpose());
			program->getUniform3f("CameraPos")->set(camera->pos);

			fb->setBlend(true, BlendEquation::ADD, BlendArgument::SRC_ALPHA, BlendArgument::ONE_MINUS_SRC_ALPHA);
			for (vector<OrkMesh>::const_iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
			{
				fb->draw(program, *(mIter->mesh));
			}
			fb->setBlend(false);
		}
		else
		{
			program->getUniformMatrix4f("DepthBiasMVP")->setMatrix(BiasMatrix * camera->GetLightVP(light2) * M);
			if (isAxes)
			{
				program->getUniformMatrix4f("MVP")->setMatrix(camera->O * camera->axesV);
				program->getUniformMatrix4f("V")->setMatrix(camera->axesV);
				program->getUniformMatrix4f("M")->setMatrix(mat4f::IDENTITY);
				program->getUniformMatrix4f("N")->setMatrix(camera->axesV.inverse().transpose());
			}
			else
			{
				program->getUniformMatrix4f("MVP")->setMatrix(camera->VP * M);
				program->getUniformMatrix4f("V")->setMatrix(camera->V);
				program->getUniformMatrix4f("M")->setMatrix(M);
				program->getUniformMatrix4f("N")->setMatrix((camera->V * M).inverse().transpose());
			}

			for (vector<OrkMesh>::const_iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
			{
				program->getUniform3f("MaterialAmbientColor")->set(mIter->material.ambientColor);
				program->getUniform3f("MaterialSpecularColor")->set(mIter->material.specularColor);
				program->getUniform3f("MaterialDiffuseColor")->set(mIter->material.diffuseColor);

				program->getUniform1b("UseDiffuseMap")->set(mIter->diffuseTexture != NULL);
				program->getUniform1b("UseNormalMap")->set(mIter->normalTexture != NULL);
				program->getUniform1b("UseSpecularMap")->set(mIter->specularTexture != NULL);
				program->getUniform1b("UseAmbientMap")->set(mIter->ambientTexture != NULL);
				program->getUniform1b("UseShadowMap")->set(isShadowMap);

				program->getUniformSampler("DiffuseTexture")->set(mIter->diffuseTexture);
				program->getUniformSampler("NormalTexture")->set(mIter->normalTexture);
				program->getUniformSampler("SpecularTexture")->set(mIter->specularTexture);
				program->getUniformSampler("AmbientTexture")->set(mIter->ambientTexture);
				program->getUniformSampler("ShadowTexture")->set(colorTexture);

				if (isBlend)
				{
					fb->setBlend(true, BlendEquation::ADD, BlendArgument::SRC_ALPHA, BlendArgument::ONE_MINUS_SRC_ALPHA);
				}
				fb->draw(program, *(mIter->mesh));
				if (isBlend)
				{
					fb->setBlend(false);
				}
			}
		}

		if (isPiece)
		{
			//RenderDebugFrame(OctreeMode);
		}
	}
}

void Model::CalcBoundAABB()//计算包围盒
{
	boundAABB.Inverse();
	for (vector<OrkMesh>::const_iterator mIter = meshes.begin(); mIter != meshes.end(); mIter++)
	{
		if (boundAABB.xMin > mIter->boundAABB.xMin)
		{
			boundAABB.xMin = mIter->boundAABB.xMin;
		}
		if (boundAABB.yMin > mIter->boundAABB.yMin)
		{
			boundAABB.yMin = mIter->boundAABB.yMin;
		}
		if (boundAABB.zMin > mIter->boundAABB.zMin)
		{
			boundAABB.zMin = mIter->boundAABB.zMin;
		}

		if (boundAABB.xMax < mIter->boundAABB.xMax)
		{
			boundAABB.xMax = mIter->boundAABB.xMax;
		}
		if (boundAABB.yMax < mIter->boundAABB.yMax)
		{
			boundAABB.yMax = mIter->boundAABB.yMax;
		}
		if (boundAABB.zMax < mIter->boundAABB.zMax)
		{
			boundAABB.zMax = mIter->boundAABB.zMax;
		}
	}
}

void Model::RenderDebugFrame(DebugFrameMode mode)//画debug用的框架
{
	if (mode == AABBMode)
	{
		OctreeManager::DrawAABBWrapper(boundAABB, translate);
	}
	else
	{
		//遍历所有obj mesh
		for (vector<OrkMesh>::iterator orkIter = meshes.begin(); orkIter != meshes.end(); orkIter++)
		{
			OctreeManager::DrawOctreeWrapper(orkIter->root, translate);
		}
	}
}

void Model::SetRenderDepth(bool flag)
{
	renderDepth = flag;
}