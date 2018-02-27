#include "string"
#include <vector>
#include <map>


class DateTime{
public:
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int dayInYear;
};

//用于写文件时的结构体，将要写入的数据存放在此类对象中，在将对象整体写入文件
class FileFormat{
public:
	int faceId;
	float zenith;
	float azimuth;
	float fvalue;
};

class SubFileFormat{
public:
	int faceId;
	float zenith;
	float azimuth;
	float fvalue;
	float fskySumValue;
};

class Simulation{
public:
	std::string scene;
	std::string weatherconditions;
	std::string platform;
	std::string platformmotion;
	std::string schedueinfor;
};

//此类用于存放读取mtl文件的结果
class MtlModel{
public:
	char name[128];
	char map_Ka[128];
	char map_Kd[128];
	char map_Ks[128];
};

//用于读取天空漫射可见性文件的结果
class VisibleModel
{
public:
	int vFaceCount;						//模型中可见面片的总数
	std::vector<int> vFaceNums;			//模型中可见面片的编号
};


//用于写文件时的结构体，将要写入的数据存放在此类对象中，在将对象整体写入文件
class FaceSkyEnergy
{
public:
int faceId;				//面片编号
float faceEnergy;		//每个面片的整个天空漫射的辐照度
};

