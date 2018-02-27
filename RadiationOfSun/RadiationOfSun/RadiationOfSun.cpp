// RadiationOfSun.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include  <math.h>
#include "iostream"
#include "fstream"
#include "IRmodule.h"
using namespace std;

void main(int argc, char* argv[])
{

	char* simFile;
	if (argc <= 1)
	{
		return;
	}

	simFile = argv[1];

	string	simFilePath = simFile;
	cout << simFilePath << endl;
	int lastpos = simFilePath.find_last_of('\\');
	string	simFileDir = simFilePath.substr(0, lastpos);

	//���ط��泡���ļ�
	Simulation sim = loadSimXML(simFile);

	string weatherFilePath = simFileDir + sim.weatherconditions.substr(1);
	string scheduleFilePath = simFileDir + sim.schedueinfor.substr(1);
	string sceneFilePath = simFileDir + sim.scene.substr(1);

	//�������߼�����ұ��ļ�
	string atmfile = simFileDir + "\\atm\\atm0316.lut";
	System::String^ newAtmFile = gcnew System::String(atmfile.c_str());

	//ͨ�����泡���ļ��ҵ������Ĳ����ļ���ȡ�ò���
	WeatherCondition weather = loadWeatherXML(weatherFilePath.c_str());
	Schedule schedule = loadScheduleXML(scheduleFilePath.c_str());
	Scene scene = loadSceneXML(sceneFilePath.c_str());

	DateTime dateTime = dateTo365(weather.date);
	if (dateTime.dayInYear <= 0) {
		cout << "Simulation Date Error!" << endl;
		return;
	}

	vector<Entry> inEntries = schedule.inEntries;

	//����춥�ǡ���λ�����ò���
	Angle skyZenithAngle = scene.zenithAngle;
	Angle skyAzimuthAngle = scene.azimuthAngle;
	Entry skyEntry = scene.skyEntry;

	//ȡ�õ����ļ��еĸ��ֲ���
	WaveBand waveband = schedule.waveBand;

	vector<Model *> models;
	for (int objIndex = 0; objIndex < scene.objects.size(); ++objIndex)
	{
		Object object = scene.objects[objIndex];
		Model * model = new Model();

		//ƽ�ƾ���
		float translateMatrix[4][4] = { 1, 0, 0, object.transform[0], 0, 1, 0, object.transform[1], 0, 0, 1, object.transform[2], 0, 0, 0, 1 };
		//��ת����
		float rotateXMatrix[4][4] = { 1, 0, 0, 0, 0, cos(object.transform[3]), -sin(object.transform[3]), 0, 0, sin(object.transform[3]), cos(object.transform[3]), 0, 0, 0, 0, 1 };
		float rotateYMatrix[4][4] = { cos(object.transform[4]), 0, sin(object.transform[4]), 0, 0, 1, 0, 0, -sin(object.transform[4]), 0, cos(object.transform[4]), 0, 0, 0, 0, 1 };
		float rotateZMatrix[4][4] = { cos(object.transform[5]), -sin(object.transform[5]), 0, 0, sin(object.transform[5]), cos(object.transform[5]), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		//���ž���
		float scaleMatrix[4][4] = { object.transform[6], 0, 0, 0, 0, object.transform[7], 0, 0, 0, 0, object.transform[8], 0, 0, 0, 0, 1 };
		//����任����
		getModelMatrix(model->modelMatrix, translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix);

		if (!model->loadOBJ(object)) {
			cout << "Load Scene Object Error!" << endl;
			return;
		}
		models.push_back(model);
	}

	//����̫�����䷽������
	glm::vec3 sunNormal = getlightPosition(weather.zenith, weather.azimuth, SunNormalFormat);

	//ʵʱ����̫����ɼ���
	vector<vector<int>> sceneSunVisibleList;
	for (int i = 0; i < models.size(); ++i)
	{
		Model * model = models[i];
		vector<int> isVisible;
		int modelFaceCount = model->faces.size();
		for (int j = 0; j < modelFaceCount; ++j)
		{
			TriangleFace face = model->faces[j];
			isVisible.push_back(isFaceVisible(face, models, i, face.faceTag, sunNormal));
		}
		sceneSunVisibleList.push_back(isVisible);
	}


	ATMLUT::ATMLookUpTable ^c = gcnew ATMLUT::ATMLookUpTable(newAtmFile);
	System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ dtlist = c->GetTao(weather.atmosphereModel, weather.aerosolModel,
		weather.visibility, 0.06f, weather.zenith);
	System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ srlist = c->GetSolarE(dateTime.dayInYear);

	string inFileName;
	string sunInFilePath, skyInFilePath, bakInFilePath;

	for (float waveLen = waveband.beginband; waveLen <= waveband.endband; waveLen += waveband.deltaband)
	{
		//̫������
		string filename = scene.name + "_" + num2str(dateTime.year) + num2str(dateTime.month) + num2str(dateTime.day) + num2str(dateTime.hour) +
			"_" + num2str(weather.atmosphereModel) + "_" + num2str(weather.aerosolModel) + "_" + num2str(waveLen) + "_" + num2str(weather.zenith) + "_" + num2str(weather.azimuth);
		Entry entry = getFilePath(1, inEntries);
		sunInFilePath = simFileDir + entry.directory.substr(1) + filename + entry.postfix;
		cout << sunInFilePath << endl;

		float dtWaveValue = getWaveValue(dtlist, waveLen);
		float srWaveValue = getWaveValue(srlist, waveLen);

		//���㳡����̫��������
		vector<vector<FileFormat>> sceneModelSunIRs = sunToObject(models, sceneSunVisibleList, weather,
			sunNormal, dtWaveValue, srWaveValue);

		//��̫��������д���ļ�
		writeToFile(sunInFilePath.c_str(), sceneModelSunIRs);

		map<int, map<int, float>> sceneFaceSumSkyIEnergy;		//map<ģ�ͱ�ţ�ģ����Ϣ>,ģ����Ϣ��map<��Ƭ��ţ����������ն�>��ʾ

		vector<vector<vector<SubFileFormat>>> sceneModelSkyIRList;		//sceneModelSkyIRList���ڴ洢ÿ����Ƭ��ÿ�����������ϵľ�����Ϣ

		//�������
		for (float vzenith = skyZenithAngle.beginAngle; vzenith <= skyZenithAngle.endAngle; vzenith += skyZenithAngle.delta)
		{
			for (float vazimuth = skyAzimuthAngle.beginAngle; vazimuth < skyAzimuthAngle.endAngle; vazimuth += skyAzimuthAngle.delta)
			{
				string filename = scene.name + "_" + num2str(dateTime.year) + num2str(dateTime.month) + num2str(dateTime.day) + num2str(dateTime.hour) +
					"_" + num2str(weather.atmosphereModel) + "_" + num2str(weather.aerosolModel) + "_" + num2str(waveLen) + "_" + num2str(vzenith) + "_" + num2str(vazimuth);

				Entry entry = getFilePath(2, inEntries);
				skyInFilePath = simFileDir + entry.directory.substr(1) + filename + entry.postfix;
				cout << skyInFilePath << endl;

				vector<VisibleModel> visibleModelList;


				//ʵʱ������չ�ɼ���
				glm::vec3 skyNormal;
				skyNormal = getlightPosition(vzenith, vazimuth, SkynormalFormat);

				int modelCount = models.size();

				for (int i = 0; i < modelCount; i++)
				{
					Model* model = models[i];
					VisibleModel modelVisibleResult;

					int modelFaceCount = model->faces.size();
					for (int j = 0; j < modelFaceCount; j++)
					{
						TriangleFace face = model->faces[j];
						if (isFaceVisible(face, models, i, face.faceTag, skyNormal))
						{
							modelVisibleResult.vFaceNums.push_back(j);
						}

					}

					visibleModelList.push_back(modelVisibleResult);
				}



				float relativeAzimuth = getRelativeAzimuth(vazimuth, weather.azimuth);


				System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ drlist = c->GetDownRad(
					weather.atmosphereModel, weather.aerosolModel, dateTime.dayInYear,
					weather.visibility, 0.06f, weather.zenith, vzenith, relativeAzimuth);
				float waveValue = getWaveValue(drlist, waveLen);

				//�������������������������sceneModelSkyIRs�洢���ǵ�һ�����������������������;
				vector<vector<SubFileFormat>> sceneModelSkyIRs = skyToObject(models, visibleModelList,
					vzenith, vazimuth, skyZenithAngle.delta, skyAzimuthAngle.delta,
					waveValue);

				//��ÿ����Ƭ��ÿ����������ľ�����Ϣ�洢��sceneModelSkyIRList��
				sceneModelSkyIRList.push_back(sceneModelSkyIRs);

				//�����д�뵽�ļ�
				writeToFile(skyInFilePath.c_str(), sceneModelSkyIRs);


				//����ÿ����Ƭ�����в�������ĺ�
				int sceneModelCount = sceneModelSkyIRs.size();

				for (int i = 0; i < sceneModelCount; i++)
				{
					vector<SubFileFormat> modelSingleOfSky = sceneModelSkyIRs[i];
					map<int, float> modelSumOfSky;

					map<int, map<int, float>>::iterator it = sceneFaceSumSkyIEnergy.find(i);
					if (it != sceneFaceSumSkyIEnergy.end())
					{
						modelSumOfSky = sceneFaceSumSkyIEnergy[i];
					}

					int modelFaceCount = modelSingleOfSky.size();
					for (int j = 0; j < modelFaceCount; j++)
					{
						SubFileFormat faceSingOfSky = modelSingleOfSky[j];
						float tempSum = 0;
						map<int, float>::iterator faceIt = modelSumOfSky.find(j);
						if (faceIt != modelSumOfSky.end())
						{
							tempSum = modelSumOfSky[j];
						}

						modelSumOfSky[j] = tempSum + faceSingOfSky.fskySumValue;

					}

					sceneFaceSumSkyIEnergy[i] = modelSumOfSky;
				}//����ÿ����Ƭ�����в�������ĺ�--����
			}


			//���������д��������ļ�
			string filename = scene.name + "_" + num2str(dateTime.year) + num2str(dateTime.month) + num2str(dateTime.day) + num2str(dateTime.hour) +
				"_" + num2str(weather.atmosphereModel) + "_" + num2str(weather.aerosolModel) + "_" + num2str(waveLen);
			Entry entry = getFilePath(2, inEntries);

			string skySumEnergyFilePath = simFileDir + entry.directory.substr(1, entry.directory.size() - 1) + filename + entry.postfix;
			writeToFile(skySumEnergyFilePath.c_str(), sceneFaceSumSkyIEnergy);
		}

	}
}







