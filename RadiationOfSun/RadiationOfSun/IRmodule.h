#ifndef _IRMODULE_H_
#define _IRMODULE_H_

#include "Util.h"

//models����ģ�ͣ�sunVisibleModelList�����еĿɼ����б�weatherCon����������sunNormal̫�����䷽��������
//dtWaveValue����͸���ʣ�srWaveValue��������̫�����ն�
//�˺���ֻ���㵥һ������̫�����ն� 

//vector<vector<FileFormat>> sunToObject(vector<Model *> models,vector<vector<int>> sunVisibleModelList,
//	WeatherCondition weatherCon,glm::vec3 sunNormal,float dtWaveValue,float srWaveValue,string filepath);

vector<vector<FileFormat>> sunToObject(vector<Model *> models,vector<vector<int>> sunVisibleModelList, 
	WeatherCondition weatherCon,glm::vec3 sunNormal,float dtWaveValue,float srWaveValue);




//models����ģ�ͣ�sunVisibleModelList�����еĿɼ����б�viewZenith�۲��춥�ǣ�
//viewAzimuth�۲ⷽλ�ǣ�waveValue����������з��䣬
//sceneFaceSumSkyIEnergy��¼������ÿ����Ƭ������յķ��ն�
vector<vector<SubFileFormat>> skyToObject(vector<Model *> models,vector<VisibleModel> visibleModelList,
	float viewZenith,float relativeAzimuth,float zenithAngleDelta,float azimuthAngleDelta,
	float waveValue);

#endif

                

