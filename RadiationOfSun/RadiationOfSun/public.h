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

//����д�ļ�ʱ�Ľṹ�壬��Ҫд������ݴ���ڴ�������У��ڽ���������д���ļ�
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

//�������ڴ�Ŷ�ȡmtl�ļ��Ľ��
class MtlModel{
public:
	char name[128];
	char map_Ka[128];
	char map_Kd[128];
	char map_Ks[128];
};

//���ڶ�ȡ�������ɼ����ļ��Ľ��
class VisibleModel
{
public:
	int vFaceCount;						//ģ���пɼ���Ƭ������
	std::vector<int> vFaceNums;			//ģ���пɼ���Ƭ�ı��
};


//����д�ļ�ʱ�Ľṹ�壬��Ҫд������ݴ���ڴ�������У��ڽ���������д���ļ�
class FaceSkyEnergy
{
public:
int faceId;				//��Ƭ���
float faceEnergy;		//ÿ����Ƭ�������������ķ��ն�
};

