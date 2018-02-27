// RadiationOfSun.cpp : 定义控制台应用程序的入口点。
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

	//加载仿真场景文件
	Simulation sim = loadSimXML(simFile);

	string weatherFilePath = simFileDir + sim.weatherconditions.substr(1);
	string scheduleFilePath = simFileDir + sim.schedueinfor.substr(1);
	string sceneFilePath = simFileDir + sim.scene.substr(1);

	//大气离线计算查找表文件
	string atmfile = simFileDir + "\\atm\\atm0316.lut";
	System::String^ newAtmFile = gcnew System::String(atmfile.c_str());

	//通过仿真场景文件找到其它的参数文件，取得参数
	WeatherCondition weather = loadWeatherXML(weatherFilePath.c_str());
	Schedule schedule = loadScheduleXML(scheduleFilePath.c_str());
	Scene scene = loadSceneXML(sceneFilePath.c_str());

	DateTime dateTime = dateTo365(weather.date);
	if (dateTime.dayInYear <= 0) {
		cout << "Simulation Date Error!" << endl;
		return;
	}

	vector<Entry> inEntries = schedule.inEntries;

	//获得天顶角、方位角配置参数
	Angle skyZenithAngle = scene.zenithAngle;
	Angle skyAzimuthAngle = scene.azimuthAngle;
	Entry skyEntry = scene.skyEntry;

	//取得调度文件中的各种参数
	WaveBand waveband = schedule.waveBand;

	vector<Model *> models;
	for (int objIndex = 0; objIndex < scene.objects.size(); ++objIndex)
	{
		Object object = scene.objects[objIndex];
		Model * model = new Model();

		//平移矩阵
		float translateMatrix[4][4] = { 1, 0, 0, object.transform[0], 0, 1, 0, object.transform[1], 0, 0, 1, object.transform[2], 0, 0, 0, 1 };
		//旋转矩阵
		float rotateXMatrix[4][4] = { 1, 0, 0, 0, 0, cos(object.transform[3]), -sin(object.transform[3]), 0, 0, sin(object.transform[3]), cos(object.transform[3]), 0, 0, 0, 0, 1 };
		float rotateYMatrix[4][4] = { cos(object.transform[4]), 0, sin(object.transform[4]), 0, 0, 1, 0, 0, -sin(object.transform[4]), 0, cos(object.transform[4]), 0, 0, 0, 0, 1 };
		float rotateZMatrix[4][4] = { cos(object.transform[5]), -sin(object.transform[5]), 0, 0, sin(object.transform[5]), cos(object.transform[5]), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		//缩放矩阵
		float scaleMatrix[4][4] = { object.transform[6], 0, 0, 0, 0, object.transform[7], 0, 0, 0, 0, object.transform[8], 0, 0, 0, 0, 1 };
		//坐标变换矩阵
		getModelMatrix(model->modelMatrix, translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix);

		if (!model->loadOBJ(object)) {
			cout << "Load Scene Object Error!" << endl;
			return;
		}
		models.push_back(model);
	}

	//计算太阳入射方向向量
	glm::vec3 sunNormal = getlightPosition(weather.zenith, weather.azimuth, SunNormalFormat);

	//实时计算太阳光可见性
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
		//太阳入射
		string filename = scene.name + "_" + num2str(dateTime.year) + num2str(dateTime.month) + num2str(dateTime.day) + num2str(dateTime.hour) +
			"_" + num2str(weather.atmosphereModel) + "_" + num2str(weather.aerosolModel) + "_" + num2str(waveLen) + "_" + num2str(weather.zenith) + "_" + num2str(weather.azimuth);
		Entry entry = getFilePath(1, inEntries);
		sunInFilePath = simFileDir + entry.directory.substr(1) + filename + entry.postfix;
		cout << sunInFilePath << endl;

		float dtWaveValue = getWaveValue(dtlist, waveLen);
		float srWaveValue = getWaveValue(srlist, waveLen);

		//计算场景的太阳入射结果
		vector<vector<FileFormat>> sceneModelSunIRs = sunToObject(models, sceneSunVisibleList, weather,
			sunNormal, dtWaveValue, srWaveValue);

		//将太阳入射结果写入文件
		writeToFile(sunInFilePath.c_str(), sceneModelSunIRs);

		map<int, map<int, float>> sceneFaceSumSkyIEnergy;		//map<模型编号，模型信息>,模型信息用map<面片编号，天空漫射辐照度>表示

		vector<vector<vector<SubFileFormat>>> sceneModelSkyIRList;		//sceneModelSkyIRList用于存储每个面片的每个采样方向上的具体信息

		//天空漫射
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


				//实时计算天空光可见性
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

				//计算整个场景的天空漫射结果，sceneModelSkyIRs存储的是单一方向整个场景的天空漫射结果;
				vector<vector<SubFileFormat>> sceneModelSkyIRs = skyToObject(models, visibleModelList,
					vzenith, vazimuth, skyZenithAngle.delta, skyAzimuthAngle.delta,
					waveValue);

				//将每个面片的每个采样方向的具体信息存储在sceneModelSkyIRList中
				sceneModelSkyIRList.push_back(sceneModelSkyIRs);

				//将结果写入到文件
				writeToFile(skyInFilePath.c_str(), sceneModelSkyIRs);


				//计算每个面片的所有采样方向的和
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
				}//计算每个面片的所有采样方向的和--结束
			}


			//天空漫射结果写入二进制文件
			string filename = scene.name + "_" + num2str(dateTime.year) + num2str(dateTime.month) + num2str(dateTime.day) + num2str(dateTime.hour) +
				"_" + num2str(weather.atmosphereModel) + "_" + num2str(weather.aerosolModel) + "_" + num2str(waveLen);
			Entry entry = getFilePath(2, inEntries);

			string skySumEnergyFilePath = simFileDir + entry.directory.substr(1, entry.directory.size() - 1) + filename + entry.postfix;
			writeToFile(skySumEnergyFilePath.c_str(), sceneFaceSumSkyIEnergy);
		}

	}
}







