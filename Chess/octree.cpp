#include "octree.h"
#include "fileloader.h"

using namespace std;
using namespace ork;

ptr<Program> OctreeManager::lineProgram = NULL;
ptr< Mesh<vec3f, unsigned int> > OctreeManager::aabbMesh = NULL;

void OctreeManager::Init()//初始化OctreeManager
{
	lineProgram = new Program(new Module(400, (char*)FileLoader::LoadFile(GetShaderPath("LineVS.glsl")),
		(char*)FileLoader::LoadFile(GetShaderPath("LineFS.glsl"))));

	lineProgram->getUniform3f("LineColor")->set(OctreeLineColor);

	aabbMesh = new Mesh<vec3f, unsigned int>(LINES, GPU_STATIC);
	aabbMesh->addAttributeType(0, 3, A32F, false);
}

void OctreeManager::CreateOctree(OctreeNode *node, int depth)//构建模型的八叉树 face-based
{
	if (node->boundFaces.size() <= MIN_FACE_NUM)
	{
		node->isLeaf = true;
		return;
	}

	int nextDepth = depth + 1;//递归深度加1
	vector<AABB> splitedAABBs(8);
	SplitAABB(node->aabb, splitedAABBs);//八分空间

	for (int i = 0; i < 8; i++)
	{
		vector<Face> boundFaces;
		GetBoundFaces(boundFaces, node->boundFaces, splitedAABBs[i]);
		node->children[i] = new OctreeNode(node, splitedAABBs[i], i, nextDepth);
		node->children[i]->boundFaces = boundFaces;

		if (depth >= MAX_DEPTH || boundFaces.size() <= MIN_FACE_NUM)
		{
			node->children[i]->isLeaf = true;
			continue;
		}

		CreateOctree(node->children[i], nextDepth);
	}
	node->boundFaces.clear();//清空父节点的Faces
}

void OctreeManager::DrawOctreeWrapper(OctreeNode *root, vec3f translate)
{
	aabbMesh->clear();

	DrawOctree(root, translate);

	lineProgram->getUniformMatrix4f("MVP")->setMatrix(camera->VP);
	screenFb->draw(lineProgram, *aabbMesh);
}

void OctreeManager::DrawOctree(OctreeNode *root, vec3f translate)
{
	if (root != NULL)
	{
		if (root->isLeaf && !root->IsEmpty())
		{
			DrawAABB(root->aabb, translate);
		}

		for (int i=0; i<8; i++)
		{
			DrawOctree(root->children[i], translate);
		}
	}
}

void OctreeManager::DrawAABBWrapper(AABB aabb, vec3f translate)
{
	aabbMesh->clear();

	DrawAABB(aabb, translate);

	lineProgram->getUniformMatrix4f("MVP")->setMatrix(camera->VP);
	screenFb->draw(lineProgram, *aabbMesh);
}

void OctreeManager::DrawAABB(AABB aabb, vec3f translate)
{
	aabb.TranslateSelf(translate);//偏移一下

	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMin, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMax));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMax, aabb.zMin));
	aabbMesh->addVertex(vec3f(aabb.xMax, aabb.yMin, aabb.zMin));
}

void OctreeManager::SplitAABB(const AABB &aabb, vector<AABB> &splitedAABBs)//把一个AABB均分成8分，返回一个数组
{
	float xHalf = (aabb.xMax - aabb.xMin) / 2.0f;
	float yHalf = (aabb.yMax - aabb.yMin) / 2.0f;
	float zHalf = (aabb.zMax - aabb.zMin) / 2.0f;

	splitedAABBs[0] = AABB(aabb.xMin, aabb.xMin + xHalf,
		aabb.yMin + yHalf, aabb.yMax,
		aabb.zMin, aabb.zMin + zHalf);
	splitedAABBs[1] = AABB(aabb.xMin + xHalf, aabb.xMax,
		aabb.yMin + yHalf, aabb.yMax,
		aabb.zMin, aabb.zMin + zHalf);
	splitedAABBs[2] = AABB(aabb.xMin + xHalf, aabb.xMax,
		aabb.yMin + yHalf, aabb.yMax,
		aabb.zMin + zHalf, aabb.zMax);
	splitedAABBs[3] = AABB(aabb.xMin, aabb.xMin + xHalf,
		aabb.yMin + yHalf, aabb.yMax,
		aabb.zMin + zHalf, aabb.zMax);

	splitedAABBs[4] = AABB(aabb.xMin, aabb.xMin + xHalf,
		aabb.yMin, aabb.yMin + yHalf,
		aabb.zMin, aabb.zMin + zHalf);
	splitedAABBs[5] = AABB(aabb.xMin + xHalf, aabb.xMax,
		aabb.yMin, aabb.yMin + yHalf,
		aabb.zMin, aabb.zMin + zHalf);
	splitedAABBs[6] = AABB(aabb.xMin + xHalf, aabb.xMax,
		aabb.yMin, aabb.yMin + yHalf,
		aabb.zMin + zHalf, aabb.zMax);
	splitedAABBs[7] = AABB(aabb.xMin, aabb.xMin + xHalf,
		aabb.yMin, aabb.yMin + yHalf,
		aabb.zMin + zHalf, aabb.zMax);
}

void OctreeManager::GetBoundFaces(vector<Face> &boundFaces, vector<Face> &faces, AABB aabb)
{
	//遍历mesh的所有face
	for (vector<Face>::iterator fIter = faces.begin(); fIter != faces.end(); fIter++)
	{
		for (vector<Refer>::iterator rIter = fIter->refers.begin(); rIter != fIter->refers.end(); rIter++)
		{
			vec3f v = VBO::vertices[rIter->v];

			if (aabb.IsVertexIn(v))
			{
				boundFaces.push_back(*fIter);
				break;
			}
		}
	}
}

void OctreeManager::Delete(OctreeNode *root)//删除
{
	if (root != NULL)
	{
		for (int i = 0; i < 8; i++)
		{
			Delete(root->children[i]);
		}
		delete root;
	}
}

int OctreeManager::GetDepth(OctreeNode *root)//获取最大深度
{
	if (root == NULL)
	{
		return -1;
	}

	vector<int> depth(8);
	for (int i = 0; i < 8; i++)
	{
		depth[i] = GetDepth(root->children[i]);
	}

	return GetVectorMax(depth) + 1;
}