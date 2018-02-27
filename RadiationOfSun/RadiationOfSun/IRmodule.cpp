#include "IRmodule.h"
#include "iostream"

//models场景模型，sunVisibleModelList场景中的可见性列表，weatherCon天气条件，sunNormal太阳入射方向向量，
//dtWaveValue下行透过率，srWaveValue大气层外太阳辐照度
//此函数只能算单一波长的太阳辐照度 
vector<vector<FileFormat>> sunToObject(vector<Model *> models,vector<vector<int>> sunVisibleModelList, WeatherCondition weatherCon,
	glm::vec3 sunNormal,float dtWaveValue,float srWaveValue)                             
{
	FileFormat wfile;
	vector<FileFormat> wfList;
	vector<vector<FileFormat>> sceneModelSunIRs;
	

	int modelCount=models.size();
	for (int i=0;i<modelCount;i++)
	{
		vector<int> visibleModel=sunVisibleModelList[i];
		Model* model=models[i];
		int modelFaceCount=model->faces.size();

		for (int j=0;j<modelFaceCount;j++)
		{
			if(visibleModel[j]){
				float sfcos=getAngleCos(sunNormal,model->faces[j].triangleNormal);     //sfcos是太阳方向向量与面片法向量夹角余弦值
				
				
				//夹角大于90度时
				if(sfcos<0){
					sfcos=0;
				}
				float result=dtWaveValue*srWaveValue*sfcos; 
				wfile.fvalue=result;
			}else{
				
				wfile.fvalue=0;
			}
			wfile.faceId=model->faces[j].faceTag;
			wfile.zenith=weatherCon.zenith;
			wfile.azimuth=weatherCon.azimuth;

			wfList.push_back(wfile);
		}

		sceneModelSunIRs.push_back(wfList);
		wfList.clear();
	}

	return sceneModelSunIRs;
}




//models场景模型，sunVisibleModelList场景中的可见性列表，viewZenith观测天顶角，
//relativeAzimuth观测方向与太阳光的相对方位角，waveValue半球天空下行辐射,
//sceneFaceSumSkyIEnergy记录场景里每个面片半球天空的辐照度
vector<vector<SubFileFormat>> skyToObject(vector<Model *> models,vector<VisibleModel> visibleModelList,
	float viewZenith,float relativeAzimuth,float zenithAngleDelta,float azimuthAngleDelta,
	float waveValue)
{
	SubFileFormat wfile;
	vector<SubFileFormat> wfList;
	vector<vector<SubFileFormat>> sceneModelSkyIRs;


	int modelCount=models.size();
	for (int i=0;i<modelCount;i++)
	{
		Model* model=models[i];
		VisibleModel visibleModel=visibleModelList[i];
		

		int modelFaceCount=model->faces.size();
		int vFaceCount=visibleModel.vFaceCount;
		//cout<<"model:"<<i<<";  vcount:"<<vFaceCount<<endl;
		for (int k=0,j=0;k<modelFaceCount;k++)
		{
			float temSum=0;
			if(j<vFaceCount)
			{
				int visibleFaceId=visibleModel.vFaceNums[j];
				//cout<<visibleFaceId<<endl;
				if (k==visibleFaceId)
				{
					float Lzenith=viewZenith*PI/180; //三角函数所用为弧度
					float deltaZenith=zenithAngleDelta*PI/180;
					float deltaAzimuth=azimuthAngleDelta*PI/180;
					float uniDirection=waveValue;//每个面片确定的观测天顶角和方位角的辐亮度
					temSum=cos(Lzenith)*sin(Lzenith)*waveValue*deltaZenith*deltaAzimuth;//每个面片确定的观测天顶角和方位角的能量值
					//cout<<temSum<<endl;
					wfile.fvalue=uniDirection;
					++j;
				}
				else{
					wfile.fvalue=0;
				}
			}else{
				wfile.fvalue=0;
			}

			wfile.fskySumValue=temSum;
			wfile.faceId=model->faces[k].faceTag;
			wfile.zenith=viewZenith;
			wfile.azimuth=relativeAzimuth;
				
			wfList.push_back(wfile);

		}

		sceneModelSkyIRs.push_back(wfList);
		wfList.clear();
	}
		
	return sceneModelSkyIRs;
}



