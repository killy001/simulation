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
	std::string modelName;					//模型名称
	std::string materialFileName;			//模型所用材质文件名
	std::string modelFileLocation;			//模型文件本身所在的相对目录
	std::vector<glm::vec3> vertices;		//模型的顶点坐标数组
	std::vector<glm::vec3> normals;			//模型的每个顶点的法线数组
	std::vector<glm::vec3> uvs;				//模型的纹理坐标数组
	std::vector<TriangleFace> faces;		//模型的所有三角面片数组，存放的是前面三个数组的索引值

	float modelMatrix[4][4];				//模型变换矩阵

	bool loadOBJ(Object object);			//加载object模型，object存放从配置文件里读取的模型信息，里面有.obj文件路径信息
};

#endif
