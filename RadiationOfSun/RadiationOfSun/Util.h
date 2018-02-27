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
const float SunNormalFormat=100000000;     //假定太阳距离
const float SkynormalFormat=30000000;
//const glm::vec3 verticalNormal(0,0,5000); //自定义的竖直方向向量

//获取对应某波长下的一个数值，可能是辐照度、辐亮度或大气透过率，依据所传入的list而定
float getWaveValue(System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ list,float waveLen);

//将记录时间的字符串转换为具体的日期和时间数值，并且计算出所给日期是当年的第几天
DateTime dateTo365(string daytime);

//计算面片法向量
void getFaceNormal(TriangleFace &face,vector<glm::vec3> &vertexs,glm::vec3 &faceNormal);

/*
 * 获得光源的位置
 * zenith 光源天顶角
 * azimuth 光源方位角
 * normalFormat 量纲，即假定的光源距离
 */
glm::vec3 getlightPosition(float zenith,float azimuth,float normalFormat);

//太阳光入射方向向量和面片法向量的夹角余弦
float getAngleCos(glm::vec3 sunNormal,glm::vec3 triangleNormal);

//从配置文件中获得天气状况,包括日期、太阳天顶角、太阳方位角、能见度、大气模式、气溶胶类型
WeatherCondition loadWeatherXML(const char * path);

//读取仿真配置文件，包括场景文件、天气状况文件、调度文件等
Simulation loadSimXML(const char * path);

Scene loadSceneXML(const char * path);

Schedule loadScheduleXML(const char * path);


//读取天空漫射可见性结果文件
vector<VisibleModel> getVisibleModel(const char* filenametemp);

//判断某个面片的可见性--基于场景(0 : invisible ; 1 : visible)
int isFaceVisible(TriangleFace face, std::vector<Model*> models, int modelID, int faceID, glm::vec3 position);

//计算直线的方向向量
glm::vec3 lineNormal(glm::vec3 p1,glm::vec3 p2);

//判断点是否在三角形中
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

//将太阳入射的结果写入文件
void writeToFile(const char* filePath,vector<vector<FileFormat>> sceneModelSunIRs);

//将天空漫射辐亮度写入文件
void writeToFile(const char* filePath,vector<vector<SubFileFormat>> sceneModelSkyIRs);

//将模型中的每个面片的整个半球天空漫射的结果写入文件
void writeToFile(const char* filePath,map<int,map<int,float>> sceneFaceSumSkyIEnergy);

Entry getFilePath(int flag,vector<Entry> inEntries);

//获得观测方向与太阳光方向的相对方位角,返回值为角度
float getRelativeAzimuth(float viewAzimuth, float solarAzimuth);

int getFileCount( Scene scene, Schedule schedule);


#endif



