#include "IRmodule.h"
#include "iostream"

//models����ģ�ͣ�sunVisibleModelList�����еĿɼ����б�weatherCon����������sunNormal̫�����䷽��������
//dtWaveValue����͸���ʣ�srWaveValue��������̫�����ն�
//�˺���ֻ���㵥һ������̫�����ն� 
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
				float sfcos=getAngleCos(sunNormal,model->faces[j].triangleNormal);     //sfcos��̫��������������Ƭ�������н�����ֵ
				
				
				//�нǴ���90��ʱ
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




//models����ģ�ͣ�sunVisibleModelList�����еĿɼ����б�viewZenith�۲��춥�ǣ�
//relativeAzimuth�۲ⷽ����̫�������Է�λ�ǣ�waveValue����������з���,
//sceneFaceSumSkyIEnergy��¼������ÿ����Ƭ������յķ��ն�
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
					float Lzenith=viewZenith*PI/180; //���Ǻ�������Ϊ����
					float deltaZenith=zenithAngleDelta*PI/180;
					float deltaAzimuth=azimuthAngleDelta*PI/180;
					float uniDirection=waveValue;//ÿ����Ƭȷ���Ĺ۲��춥�Ǻͷ�λ�ǵķ�����
					temSum=cos(Lzenith)*sin(Lzenith)*waveValue*deltaZenith*deltaAzimuth;//ÿ����Ƭȷ���Ĺ۲��춥�Ǻͷ�λ�ǵ�����ֵ
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



