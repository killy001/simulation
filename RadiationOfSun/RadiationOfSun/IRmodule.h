#ifndef _IRMODULE_H_
#define _IRMODULE_H_

#include "Util.h"

//models场景模型，sunVisibleModelList场景中的可见性列表，weatherCon天气条件，sunNormal太阳入射方向向量，
//dtWaveValue下行透过率，srWaveValue大气层外太阳辐照度
//此函数只能算单一波长的太阳辐照度 

//vector<vector<FileFormat>> sunToObject(vector<Model *> models,vector<vector<int>> sunVisibleModelList,
//	WeatherCondition weatherCon,glm::vec3 sunNormal,float dtWaveValue,float srWaveValue,string filepath);

vector<vector<FileFormat>> sunToObject(vector<Model *> models,vector<vector<int>> sunVisibleModelList, 
	WeatherCondition weatherCon,glm::vec3 sunNormal,float dtWaveValue,float srWaveValue);




//models场景模型，sunVisibleModelList场景中的可见性列表，viewZenith观测天顶角，
//viewAzimuth观测方位角，waveValue半球天空下行辐射，
//sceneFaceSumSkyIEnergy记录场景里每个面片半球天空的辐照度
vector<vector<SubFileFormat>> skyToObject(vector<Model *> models,vector<VisibleModel> visibleModelList,
	float viewZenith,float relativeAzimuth,float zenithAngleDelta,float azimuthAngleDelta,
	float waveValue);

#endif

                

