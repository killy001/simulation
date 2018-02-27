#ifndef HEADER_TRIANGLE_FACE
#define HEADER_TRIANGLE_FACE
#include "glm.hpp"
#include <string>

class TriangleFace{
public:
	int vertexIndex1,vertexIndex2,vertexIndex3;
	int normalIndex1,normalIndex2,normalIndex3;
	int uvIndex1,uvIndex2,uvIndex3;

	int faceTag;//面片标识
	glm::vec3 triangleNormal;//三角面片法向量
	glm::vec3 center;//三角面片中心点
	std::string material;
};

#endif