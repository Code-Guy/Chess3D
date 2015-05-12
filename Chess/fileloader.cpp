#include "fileloader.h"
#include <cstdio>

using namespace std;
using namespace ork;

//加载文件，返回文件内容（char *）和文件大小
unsigned char* FileLoader::LoadFile(const string filePath, int &size)
{
	FILE *file = fopen(filePath.c_str(), "rb");

	if (file == NULL)//打开文件错误，例如文件路径不存在，直接返回NULL
	{
		return NULL;
	}

	//先获取文件大小
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	//回到文件头
	fseek(file, 0, SEEK_SET);

	char *data = new char[size + 1];
	fread(data, size, 1, file);
	data[size] = 0;//最后一个字符为0？\0

	fclose(file);

	return (unsigned char *)data;
}

//加载文件，返回文件内容（char *）
unsigned char* FileLoader::LoadFile(const string filePath)
{
	FILE *file = fopen(filePath.c_str(), "rb");

	if (file == NULL)//打开文件错误，例如文件路径不存在，直接返回NULL
	{
		return NULL;
	}

	//先获取文件大小
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	//回到文件头
	fseek(file, 0, SEEK_SET);

	char *data = new char[size + 1];
	fread(data, size, 1, file);
	data[size] = 0;//最后一个字符为0？\0

	fclose(file);

	return (unsigned char *)data;
}

//加载program
ptr<Program> FileLoader::LoadProgram(const string filePath)
{
	return new Program(new Module(330, (char *)LoadFile(filePath)));
}

//加载天空盒
ptr<TextureCube> FileLoader::LoadTextureCube(const string filePath, Texture::Parameters para)
{
	ptr<Buffer> buffers[6];
	Buffer::Parameters texpar[6] = { Buffer::Parameters() };
	int size, w, h, channels;
	string curpath = filePath + "/posx.jpg";
	unsigned char* chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	unsigned char* tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[0] = new CPUBuffer(tempdata);

	curpath = filePath + "/negx.jpg";
	chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[1] = new CPUBuffer(tempdata);

	curpath = filePath + "/posy.jpg";
	chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[2] = new CPUBuffer(tempdata);

	curpath = filePath + "/negy.jpg";
	chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[3] = new CPUBuffer(tempdata);

	curpath = filePath + "/posz.jpg";
	chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[4] = new CPUBuffer(tempdata);

	curpath = filePath + "/negz.jpg";
	chardata = LoadFile(curpath.c_str(), size);
	if (chardata == NULL)
	{
		return NULL;
	}
	tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);
	buffers[5] = new CPUBuffer(tempdata);

	if (channels == 3)
		return new TextureCube(w, h, RGB8, RGB, PixelType::UNSIGNED_BYTE, para, texpar, buffers);
	return new TextureCube(w, h, RGBA8, RGBA, PixelType::UNSIGNED_BYTE, para, texpar, buffers);
}

//加载贴图
ptr<Texture2D> FileLoader::LoadTexture(const string filePath, Texture::Parameters para)
{
	int w, h, channels, size;
	unsigned char* chardata = LoadFile(filePath, size);
	if (chardata == NULL)
	{
		return NULL;
	}
	unsigned char* tempdata = stbi_load_from_memory(chardata, size, &w, &h, &channels, 0);

	if (channels == 3)
		return new Texture2D(w, h, RGB8, RGB, UNSIGNED_BYTE, para, Buffer::Parameters(), CPUBuffer(tempdata));
	return new Texture2D(w, h, RGBA8, RGBA, UNSIGNED_BYTE, para, Buffer::Parameters(), CPUBuffer(tempdata));
}

//加载obj mesh文件
bool FileLoader::LoadObjMesh(const string filePath, vector<ObjMesh> &meshes, string &mtlName)
{
	printf("Loading mesh %s\n", filePath);

	meshes.clear();
	ObjMesh objMesh;//一个obj mesh

	bool isNextMesh = false;

	int vOffset = VBO::vertices.size();
	int uvOffset = VBO::uvs.size();
	int nOffset = VBO::normals.size();

	FILE *file = fopen(filePath.c_str(), "r");
	if (file == NULL)
	{
		printf("Could not open this obj file!\n");
		getchar();
		return false;
	}

	while (true)
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "g") == 0)
		{
			fscanf(file, "%s\n", objMesh.ID);
			isNextMesh = true;
		}
		else if (strcmp(lineHeader, "mtllib") == 0)
		{
			char mtlNameStr[MAX_ID_LEN];//mtl文件名
			fscanf(file, "%s\n", mtlNameStr);
			mtlName = mtlNameStr;
		}
		else if (strcmp(lineHeader, "usemtl") == 0)
		{
			fscanf(file, "%s\n", objMesh.MID);
		}
		else if (strcmp(lineHeader, "v") == 0)
		{
			if (isNextMesh)
			{
				meshes.push_back(objMesh);
				objMesh.Clear();
				isNextMesh = false;
			}
			vec3f v;
			fscanf(file, "%f %f %f\n", &v.x, &v.y, &v.z);
			VBO::vertices.push_back(v);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			vec2f uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			VBO::uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			vec3f n;
			fscanf(file, "%f %f %f\n", &n.x, &n.y, &n.z);
			VBO::normals.push_back(n);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			//简单起见，这里的面只是三角面^_^
			Face face;
			int vertexIndex[3], uvIndex[3], normalIndex[3];

			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			face.refers.push_back(Refer(vertexIndex[0] - 1 + vOffset, uvIndex[0] - 1 + uvOffset, normalIndex[0] - 1 + nOffset));
			face.refers.push_back(Refer(vertexIndex[1] - 1 + vOffset, uvIndex[1] - 1 + uvOffset, normalIndex[1] - 1 + nOffset));
			face.refers.push_back(Refer(vertexIndex[2] - 1 + vOffset, uvIndex[2] - 1 + uvOffset, normalIndex[2] - 1 + nOffset));

			objMesh.faces.push_back(face);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	fclose(file);

	meshes.push_back(objMesh);

	return true;
}

//加载ork mesh文件
bool FileLoader::LoadOrkMesh(const string filePath, vector<OrkMesh> &meshes, bool isCreateOctree)
{
	ObjMesh objMesh;//一个obj mesh
	vector<ObjMesh> objMeshes;//所有obj mesh
	string mtlName = "";

	LoadObjMesh(filePath, objMeshes, mtlName);

	if (mtlName != "")
		LoadMaterialTex(mtlName, objMeshes);//载入材质和贴图到objMeshes

	FillMeshes(objMeshes, meshes, isCreateOctree);//用自定义的objmesh来填充ork mesh

	VBO::Clear();//清空vbo

	return true;
}

//载入材质和贴图
bool FileLoader::LoadMaterialTex(const string mtlName, vector<ObjMesh> &objMeshes)
{
	printf("  %s\n", GetModelPath(mtlName));

	FILE *file = fopen(GetModelPath(mtlName).c_str(), "r");
	if (file == NULL)
	{
		getchar();
		return false;
	}

	vector<ObjMesh>::iterator mIter;//一个mesh迭代器

	while (true)
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "newmtl") == 0)
		{
			char MID[MAX_ID_LEN];
			fscanf(file, "%s\n", MID);

			mIter = ObjMesh::GetObjMeshWithMID(objMeshes, MID);
		}
		else if (strcmp(lineHeader, "Kd") == 0 && mIter != objMeshes.end())
		{
			fscanf(file, "%f %f %f\n", &(mIter->material.diffuseColor.x), &(mIter->material.diffuseColor.y), &(mIter->material.diffuseColor.z));
		}
		else if (strcmp(lineHeader, "Ka") == 0 && mIter != objMeshes.end())
		{
			fscanf(file, "%f %f %f\n", &(mIter->material.ambientColor.x), &(mIter->material.ambientColor.y), &(mIter->material.ambientColor.z));
		}
		else if (strcmp(lineHeader, "Ks") == 0 && mIter != objMeshes.end())
		{
			fscanf(file, "%f %f %f\n", &(mIter->material.specularColor.x), &(mIter->material.specularColor.y), &(mIter->material.specularColor.z));
		}
		else if (strcmp(lineHeader, "map_Kd") == 0 && mIter != objMeshes.end())
		{
			char texName[MAX_ID_LEN];
			fscanf(file, "%s\n", texName);
			mIter->diffuseTexName = string(texName);
		}
		else if (strcmp(lineHeader, "map_Ka") == 0 && mIter != objMeshes.end())
		{
			char texName[MAX_ID_LEN];
			fscanf(file, "%s\n", texName);
			mIter->ambientTexName = string(texName);
		}
		else if (strcmp(lineHeader, "map_Ks") == 0 && mIter != objMeshes.end())
		{
			char texName[MAX_ID_LEN];
			fscanf(file, "%s\n", texName);
			mIter->specularTexName = string(texName);
		}
		else if (strcmp(lineHeader, "bump") == 0 && mIter != objMeshes.end())
		{
			char texName[MAX_ID_LEN];
			fscanf(file, "%s\n", texName);
			mIter->normalTexName = string(texName);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	return true;
}

//用自定义的objmesh来填充ork mesh
void FileLoader::FillMeshes(vector<ObjMesh> &objMeshes, vector<OrkMesh> &meshes, bool isCreateOctree)
{
	//遍历所有obj mesh
	for (vector<ObjMesh>::iterator oIter = objMeshes.begin(); oIter != objMeshes.end(); oIter++)
	{
		AABB aabb;
		aabb = oIter->CalcAABB();//计算包围盒

		OctreeNode *root = NULL;

		if (isCreateOctree)
		{
			root = new OctreeNode();
			oIter->CreateOctree(root);//计算八叉树
		}

		ptr< Mesh<V_UV_N_T_B, unsigned int> > mesh = new Mesh<V_UV_N_T_B, unsigned int>(TRIANGLES, GPU_STATIC);
		mesh->addAttributeType(0, 3, A32F, false);
		mesh->addAttributeType(1, 2, A32F, false);
		mesh->addAttributeType(2, 3, A32F, false);
		mesh->addAttributeType(3, 2, A32F, false);
		mesh->addAttributeType(4, 3, A32F, false);

		//遍历obj mesh的所有face
		for (vector<Face>::const_iterator fIter = oIter->faces.begin(); fIter != oIter->faces.end(); fIter++)
		{
			vec3f T, B;
			CalcTB(T, B, *fIter);//计算Tangent和Bitangent向量
			for (vector<Refer>::const_iterator rIter = fIter->refers.begin(); rIter != fIter->refers.end(); rIter++)
			{
				mesh->addVertex(V_UV_N_T_B(VBO::vertices[rIter->v].x, VBO::vertices[rIter->v].y, VBO::vertices[rIter->v].z,
					VBO::uvs[rIter->uv].x, VBO::uvs[rIter->uv].y,
					VBO::normals[rIter->n].x, VBO::normals[rIter->n].y, VBO::normals[rIter->n].z,
					T.x, T.y, T.z, B.x, B.y, B.z));

			}
		}

		if (oIter->diffuseTexName != "")
		{
			printf("    Diffuse Texture: %s\n", oIter->diffuseTexName.c_str());
		}
		if (oIter->normalTexName != "")
		{
			printf("    Normal Texture: %s\n", oIter->normalTexName.c_str());
		}
		if (oIter->specularTexName != "")
		{
			printf("    Specular Texture: %s\n", oIter->specularTexName.c_str());
		}
		if (oIter->ambientTexName != "")
		{
			printf("    Ambient Texture: %s\n", oIter->ambientTexName.c_str());
		}

		meshes.push_back(OrkMesh(mesh, oIter->material, aabb, root, 
			LoadTexture(GetTexturePath(oIter->diffuseTexName)),
			LoadTexture(GetTexturePath(oIter->normalTexName)),
			LoadTexture(GetTexturePath(oIter->specularTexName))));
	}
}

//获取资源路径
string GetModelPath(string fileName)
{
	return BaseModelPath + fileName;
}

string GetTexturePath(string fileName)
{
	return BaseTexturePath + fileName;
}

string GetShaderPath(string fileName)
{
	return BaseShaderPath + fileName;
}