#ifndef HEADER_MODEL
#define HEADER_MODEL
#include "glm.hpp"
#include "vector"
#include "TriangleFace.h"
#include "fstream"
#include "iostream"
#include "Object.h"


class Model{
public:
	std::string modelName;					//ģ������
	std::string materialFileName;			//ģ�����ò����ļ���
	std::string modelFileLocation;			//ģ���ļ��������ڵ����Ŀ¼
	std::vector<glm::vec3> vertices;		//ģ�͵Ķ�����������
	std::vector<glm::vec3> normals;			//ģ�͵�ÿ������ķ�������
	std::vector<glm::vec3> uvs;				//ģ�͵�������������
	std::vector<TriangleFace> faces;		//ģ�͵�����������Ƭ���飬��ŵ���ǰ���������������ֵ

	float modelMatrix[4][4];				//ģ�ͱ任����

	bool loadOBJ(Object object);			//����objectģ�ͣ�object��Ŵ������ļ����ȡ��ģ����Ϣ��������.obj�ļ�·����Ϣ
};

#endif
