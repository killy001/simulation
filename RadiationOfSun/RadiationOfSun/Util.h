#ifndef _UTIL_H_
#define _UTIL_H_

#include <atltime.h>
#include <vector>
#include <string>
#include <map>

#include"Model.h"
#include"tinyxml.h"
#include "Object.h"
#include "Scene.h"
#include "WeatherCondition.h"
#include "public.h"
#include "Schedule.h"

using namespace std;

const float PI=3.1415927;   
const float SunNormalFormat=100000000;     //�ٶ�̫������
const float SkynormalFormat=30000000;
//const glm::vec3 verticalNormal(0,0,5000); //�Զ������ֱ��������

//��ȡ��Ӧĳ�����µ�һ����ֵ�������Ƿ��նȡ������Ȼ����͸���ʣ������������list����
float getWaveValue(System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ list,float waveLen);

//����¼ʱ����ַ���ת��Ϊ��������ں�ʱ����ֵ�����Ҽ�������������ǵ���ĵڼ���
DateTime dateTo365(string daytime);

//������Ƭ������
void getFaceNormal(TriangleFace &face,vector<glm::vec3> &vertexs,glm::vec3 &faceNormal);

/*
 * ��ù�Դ��λ��
 * zenith ��Դ�춥��
 * azimuth ��Դ��λ��
 * normalFormat ���٣����ٶ��Ĺ�Դ����
 */
glm::vec3 getlightPosition(float zenith,float azimuth,float normalFormat);

//̫�������䷽����������Ƭ�������ļн�����
float getAngleCos(glm::vec3 sunNormal,glm::vec3 triangleNormal);

//�������ļ��л������״��,�������ڡ�̫���춥�ǡ�̫����λ�ǡ��ܼ��ȡ�����ģʽ�����ܽ�����
WeatherCondition loadWeatherXML(const char * path);

//��ȡ���������ļ������������ļ�������״���ļ��������ļ���
Simulation loadSimXML(const char * path);

Scene loadSceneXML(const char * path);

Schedule loadScheduleXML(const char * path);


//��ȡ�������ɼ��Խ���ļ�
vector<VisibleModel> getVisibleModel(const char* filenametemp);

//�ж�ĳ����Ƭ�Ŀɼ���--���ڳ���(0 : invisible ; 1 : visible)
int isFaceVisible(TriangleFace face, std::vector<Model*> models, int modelID, int faceID, glm::vec3 position);

//����ֱ�ߵķ�������
glm::vec3 lineNormal(glm::vec3 p1,glm::vec3 p2);

//�жϵ��Ƿ�����������
bool pointInTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p0);

float selectMin(float f1,float f2,float f3);

float selectMax(float f1,float f2,float f3);

vector<int> loadMTMFile(const char* fileName, int & length, int & width);
vector<MtlModel> loadMtlFile(const char* filepath);

vector<map<int,float>> getMaterialInfo(Model* model);

void getModelMatrix(float modelMatrix[4][4], float translateMatrix[4][4], float rotateXMatrix[4][4], float rotateYMatrix[4][4], float rotateZMatrix[4][4], float scaleMatrix[4][4]);

glm::vec3 transformToScene(glm::vec3 modelPosition, float modelMatrix[4][4]);

string num2str(float num);
string num2str(int num);
float str2num(string str);

//��̫������Ľ��д���ļ�
void writeToFile(const char* filePath,vector<vector<FileFormat>> sceneModelSunIRs);

//��������������д���ļ�
void writeToFile(const char* filePath,vector<vector<SubFileFormat>> sceneModelSkyIRs);

//��ģ���е�ÿ����Ƭ�����������������Ľ��д���ļ�
void writeToFile(const char* filePath,map<int,map<int,float>> sceneFaceSumSkyIEnergy);

Entry getFilePath(int flag,vector<Entry> inEntries);

//��ù۲ⷽ����̫���ⷽ�����Է�λ��,����ֵΪ�Ƕ�
float getRelativeAzimuth(float viewAzimuth, float solarAzimuth);

int getFileCount( Scene scene, Schedule schedule);


#endif



