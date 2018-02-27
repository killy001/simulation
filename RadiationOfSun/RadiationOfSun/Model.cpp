#include "Model.h"
#include "Util.h"

using namespace std;

bool Model::loadOBJ(Object object){
	const char * path=object.filePath.c_str();

	printf("Loading OBJ file %s...\n",path );

	ifstream in(path);

	if(!in.good()){
		cout<<"Open File "<<path<<" Error!"<<endl;
		return false;
	}

	this->modelName=object.name;

	//获取obj模型文件所在目录，用以查找与obj模型文件在同一目录下的mtl、mtm等文件
	size_t found=object.filePath.find(object.name);
	if(found!=string::npos){
		modelFileLocation=object.filePath.substr(0,found);
	}else{
		cout<<"截取obj文件所在位置失败！"<<endl;
		exit(0);
	}

	//cout<<modelFileLocation<<endl;

	//记录面片编号
	int number = 0;

	//面片所用材质
	std::string faceMaterial;

	while(1){
		char buffer[128];			//每次循环重新定义，为了清空以前所使用时的旧数据
		glm::vec3 vertex;
		glm::vec3 uv;
		glm::vec3 normal;
		TriangleFace face;

		//判断文件是否结束
		if(in.getline(buffer,127).eof()){
			break;
		}

		switch(buffer[0])
		{
		case '#'://comment
			break;
		case 'v':
			switch(buffer[1])
			{
			case ' '://vertex
				if(sscanf(buffer,"v %f %f %f",&vertex.x,&vertex.y,&vertex.z)==3)
				{
					this->vertices.push_back(transformToScene(vertex, this->modelMatrix));
				}
				break;
			case 't'://texture
				if(sscanf(buffer,"vt %f %f %f",&uv.x ,&uv.y,&uv.z)==3)
				{
					this->uvs.push_back(uv);
				}
				break;
			case 'n'://normal
				if(sscanf(buffer,"vn %f %f %f",&normal.x,&normal.y,&normal.z)==3)
				{
					this->normals.push_back(transformToScene(normal, this->modelMatrix));
				}
				break;
			default:
				break;
			}
			break;
		//face
		case 'f':{
			int matches=sscanf(buffer,"f %d/%d/%d %d/%d/%d %d/%d/%d",
				&face.vertexIndex1,&face.uvIndex1,&face.normalIndex1,
				&face.vertexIndex2,&face.uvIndex2,&face.normalIndex2,
				&face.vertexIndex3,&face.uvIndex3,&face.normalIndex3);

			if (matches != 9){
				printf("File can't be read by our simple parser");
				return false;
			}
			
			int vertexSize = this->vertices.size();
			int normalSize = this->normals.size();
			int uvSize = this->uvs.size();

			//解决负索引问题
			if(face.vertexIndex1 < 0){
				face.vertexIndex1 = vertexSize + face.vertexIndex1 + 1;
			}
			if(face.vertexIndex2 < 0){
				face.vertexIndex2 = vertexSize + face.vertexIndex2 + 1;
			}
			if(face.vertexIndex3 < 0){
				face.vertexIndex3 = vertexSize + face.vertexIndex3 + 1;
			}
			if(face.normalIndex1 < 0){
				face.normalIndex1 = normalSize + face.normalIndex1 + 1;
			}
			if(face.normalIndex2 < 0){
				face.normalIndex2 = normalSize + face.normalIndex2 + 1;
			}
			if(face.normalIndex3 < 0){
				face.normalIndex3 = normalSize + face.normalIndex3 + 1;
			}
			if(face.uvIndex1 < 0){
				face.uvIndex1 = uvSize + face.uvIndex1 + 1;
			}
			if(face.uvIndex2 < 0){
				face.uvIndex2 = uvSize + face.uvIndex2 + 1;
			}
			if(face.uvIndex3 < 0){
				face.uvIndex3 = uvSize + face.uvIndex3 + 1;
			}

			//三角形三个顶点
			glm::vec3 p1 = this->vertices[face.vertexIndex1 - 1];
			glm::vec3 p2 = this->vertices[face.vertexIndex2 - 1];
			glm::vec3 p3 = this->vertices[face.vertexIndex3 - 1];

			//三角形中心点
			face.center.x = (p1.x + p2.x + p3.x) / 3;
			face.center.y = (p1.y + p2.y + p3.y) / 3;
			face.center.z = (p1.z + p2.z + p3.z) / 3;

			//三角形边长p1->p2
			glm::vec3 p12,p13;
			p12.x=p1.x-p2.x;
			p12.y=p1.y-p2.y;
			p12.z=p1.z-p2.z;

			//三角形边长p1->p3
			p13.x=p1.x-p3.x;
			p13.y=p1.y-p3.y;
			p13.z=p1.z-p3.z;

			//平面法向量p=p12 X p13
			face.triangleNormal.x = p12.y*p13.z-p12.z*p13.y;
			face.triangleNormal.y = p12.z*p13.x-p12.x*p13.z;
			face.triangleNormal.z = p12.x*p13.y-p12.y*p13.x;

			//面片标识
			face.faceTag = number++;

			//面片材质
			face.material = faceMaterial;

			this->faces.push_back(face);
			break;
		}
		case 'm'://mtllib
			char materialFileName[128];
			if (sscanf(buffer, "mtllib %s", materialFileName) == 1)
			{
				this->materialFileName = materialFileName;
			}
			break;
		case 'u'://usemtl
			char material[128];
			if (sscanf(buffer, "usemtl %s", material) == 1)
			{
				faceMaterial = material;
			}
			break;
		case 'g'://group
			break;
		default:
			break;
		}
	}
	in.close();
	return true;
}


