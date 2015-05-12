#ifndef _OCTREE_H
#define  _OCTREE_H

#include "geometry.h"

#define MAX_DEPTH 30
#define MIN_FACE_NUM 100

#define NOT_EMPTY_STANDRAD 50

const ork::vec3f OctreeLineColor(0, 1, 0);

class OctreeNode
{
public:
	OctreeNode *children[8];//8�����ӽڵ�
	OctreeNode *parent;//���ڵ�
	AABB aabb;//��Χ��
	int index;//�ڼ�������
	int depth;//���
	bool isLeaf;//�Ƿ�ΪҶ�ڵ�

	vector<Face> boundFaces;//�� for single mesh

	OctreeNode(OctreeNode *parent = NULL, AABB aabb = INFINITY_AABB, int index = -1, int depth = 0, bool isLeaf = false)
		: parent(parent), aabb(aabb), index(index), depth(depth), isLeaf(isLeaf)
	{
		for (int i=0; i<8; i++)
		{
			children[i] = NULL;
		}
	}

	~OctreeNode()
	{
		boundFaces.clear();
	}

	bool IsEmpty()
	{
		return boundFaces.size() < NOT_EMPTY_STANDRAD;
	}
};

class OctreeManager
{
public:
	static void Init();//��ʼ��OctreeManager

	static void CreateOctree(OctreeNode *node, int depth = 0);//����ģ�͵İ˲��� face-based

	static void DrawOctreeWrapper(OctreeNode *root, ork::vec3f translate);
	static void DrawAABBWrapper(AABB aabb, ork::vec3f translate);

	static void DrawOctree(OctreeNode *root, ork::vec3f translate);
	static void DrawAABB(AABB aabb, ork::vec3f translate);

	static void SplitAABB(const AABB &aabb, vector<AABB> &splitedAABBs);//��һ��AABB���ֳ�8�֣�����һ������
	static void GetBoundFaces(vector<Face> &boundFaces, vector<Face> &faces, AABB aabb);

	static void Delete(OctreeNode *root);//ɾ��
	static int GetDepth(OctreeNode *root);//��ȡ������

	static ork::ptr<ork::Program> lineProgram;
	static ork::ptr< ork::Mesh<ork::vec3f, unsigned int> > aabbMesh;
};

#endif //_OCTREE_H