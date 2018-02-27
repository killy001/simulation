#ifndef HEADER_TRIANGLE_FACE
#define HEADER_TRIANGLE_FACE
#include "glm.hpp"
#include <string>

class TriangleFace{
public:
	int vertexIndex1,vertexIndex2,vertexIndex3;
	int normalIndex1,normalIndex2,normalIndex3;
	int uvIndex1,uvIndex2,uvIndex3;

	int faceTag;//��Ƭ��ʶ
	glm::vec3 triangleNormal;//������Ƭ������
	glm::vec3 center;//������Ƭ���ĵ�
	std::string material;
};

#endif