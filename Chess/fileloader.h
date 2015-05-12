#ifndef _FILELOADER_H
#define  _FILELOADER_H

#include <string>
#include "mesh.h"

#include "libraries/stbi/stb_image.h"

const std::string BaseModelPath = "media/models/";
const std::string BaseTexturePath = "media/textures/";
const std::string BaseShaderPath = "media/shaders/";

const ork::Texture::Parameters DefaultTexturePara = ork::Texture::Parameters().min(ork::TextureFilter::LINEAR).mag(ork::TextureFilter::LINEAR).wrapS(ork::TextureWrap::CLAMP_TO_EDGE).wrapT(ork::TextureWrap::CLAMP_TO_EDGE);

class FileLoader
{
public:
	//加载文件，返回文件内容（char *）和文件大小
	static unsigned char* LoadFile(const std::string filePath, int &size);
	//加载文件，返回文件内容（char *）
	static unsigned char* LoadFile(const std::string filePath);

	//加载program
	static ork::ptr<ork::Program> LoadProgram(const std::string filePath);
	//加载天空盒
	static ork::ptr<ork::TextureCube> LoadTextureCube(const std::string filePath, ork::Texture::Parameters para = DefaultTexturePara);
	//加载贴图
	static ork::ptr<ork::Texture2D> LoadTexture(const std::string filePath, ork::Texture::Parameters para = DefaultTexturePara);
	//加载obj mesh文件
	static bool LoadObjMesh(const std::string filePath, std::vector<ObjMesh> &meshes, std::string &mtlName);
	//加载ork mesh文件
	static bool LoadOrkMesh(const std::string filePath, std::vector<OrkMesh> &meshes, bool isCreateOctree);

	//获取资源路径
	friend std::string GetModelPath(std::string fileName);
	friend std::string GetTexturePath(std::string fileName);
	friend std::string GetShaderPath(std::string fileName);

private:
	//载入材质和贴图
	static bool LoadMaterialTex(const std::string mtlName, std::vector<ObjMesh> &objMeshes);
	//用自定义的objmesh来填充ork mesh
	static void FillMeshes(std::vector<ObjMesh> &objMeshes, std::vector<OrkMesh> &meshes, bool isCreateOctree);
};

#endif //_FILELOADER_H