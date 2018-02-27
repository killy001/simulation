
#include "Util.h"
#include "tinyxml.h"
#include "sstream"
#include "fstream"
#include "iostream"

//获取对应某波长下的一个数值，可能是辐照度、辐亮度或大气透过率，依据所传入的list而定
float getWaveValue(System::Collections::Generic::List<ATMLUT::SpecturmPoint^>^ list, float waveLen) {
	ATMLUT::SpecturmPoint^ specturm1, ^ specturm2;
	if (list->Count == 0)
	{
		cout << "getWaveValue Error!" << endl;
	}
	specturm1 = list[0];
	if (waveLen < specturm1->waveNumber)
	{
		if (specturm1->waveNumber - waveLen >= 10) {
			cout << "waveLen is too small!" << endl;
			return -1;
		}
		else return specturm1->value;
	}
	else
	{
		int i;
		for (i = 1; i < list->Count; ++i) {
			specturm1 = list[i - 1];
			specturm2 = list[i];
			if (waveLen >= specturm1->waveNumber&&waveLen < specturm2->waveNumber)
			{
				return (waveLen - specturm1->waveNumber) <= (specturm2->waveNumber - waveLen) ? specturm1->value : specturm2->value;
			}
		}
		specturm2 = list[i - 1];

		if (waveLen - specturm2->waveNumber > 10) {
			cout << "Error:the waveLen is out of range!" << endl;
			return -1;
		}
		else {
			return specturm2->value;
		}

	}
}

//将记录时间的字符串转换为具体的日期和时间数值，并且计算出所给日期是当年的第几天
DateTime dateTo365(string daytime) {
	int year = -1, month = -1, day = -1, hour = -1, minute = -1, second = -1;
	const char *str = daytime.c_str();
	//if(sscanf(str,"%d-%d-%d",&year,&month,&day)==3)
	DateTime dateTime;
	if (sscanf(str, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
		/*printf("%d  %d  %d\n",hour,minute,second);*/
		ATL::CTime curtime = ATL::CTime(year, month, day, hour, minute, second);
		ATL::CTime begintime = ATL::CTime(year, 1, 1, 0, 0, 0);
		ATL::CTimeSpan span;
		span = curtime - begintime;
		dateTime.year = year;
		dateTime.month = month;
		dateTime.day = day;
		dateTime.hour = hour;
		dateTime.minute = minute;
		dateTime.second = second;
		dateTime.dayInYear = span.GetDays() + 1;
	}
	else {
		printf("Analyze daytime error!\n");
	}
	return dateTime;
}

/*
 * 获得光源的位置
 * zenith 光源天顶角
 * azimuth 光源方位角
 * normalFormat 量纲，即假定的光源距离
 */
glm::vec3 getlightPosition(float zenith, float azimuth, float normalFormat)
{
	//转换成弧度
	float Lzenith = zenith*PI / 180;
	float Lazimuth = azimuth*PI / 180;

	float sinz = sin(Lzenith);
	float cosz = cos(Lzenith);
	float sina = sin(Lazimuth);
	float cosa = cos(Lazimuth);

	glm::vec3 sunNormal;

	//cout<<normalFormat<<endl;

	if (sinz<0.00001&&sinz>-0.00001) {
		sinz = 0;
	}

	if (cosz<0.00001&&cosz>-0.00001) {
		cosz = 0;
	}

	if (sina<0.00001&&sina>-0.00001) {
		sina = 0;
	}

	if (cosa<0.00001&&cosa>-0.00001) {
		cosa = 0;
	}

	//为了消除0的负号
	if (sinz && cosa)
	{
		sunNormal.x = sinz*cosa*normalFormat;
	}
	else
	{
		sunNormal.x = 0;
	}

	if (sinz && sina)
	{
		sunNormal.y = sinz*sina*normalFormat;
	}
	else
	{
		sunNormal.y = 0;
	}

	if (cosz)
	{
		sunNormal.z = cosz*normalFormat;
	}
	else
	{
		sunNormal.z = 0;
	}
	return sunNormal;
}

//太阳光入射方向向量和面片法向量的夹角余弦
float getAngleCos(glm::vec3 sunNormal, glm::vec3 triangleNormal)
{
	float temp = sunNormal.x*triangleNormal.x + sunNormal.y*triangleNormal.y + sunNormal.z*triangleNormal.z;
	float sunNormalwidth = sqrt(sunNormal.x*sunNormal.x + sunNormal.y*sunNormal.y + sunNormal.z*sunNormal.z);
	float triangleNormalwidth = sqrt(triangleNormal.x*triangleNormal.x + triangleNormal.y*triangleNormal.y + triangleNormal.z*triangleNormal.z);
	return temp / (sunNormalwidth*triangleNormalwidth);
}


//从配置文件中获得天气状况,包括日期、太阳天顶角、太阳方位角、能见度、大气模式、气溶胶类型
WeatherCondition loadWeatherXML(const char * path)
{
	TiXmlDocument document(path);
	bool result = document.LoadFile();
	if (!result)
	{
		cout << "can not load file" << path << endl;
		exit(0);
	}

	TiXmlElement * root = document.RootElement();
	TiXmlNode * pNode = NULL;
	WeatherCondition weatherCondition;
	for (pNode = root->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		TiXmlElement * element = pNode->ToElement();
		string nodeName(element->Value());
		if (nodeName.compare("reference") == 0)
		{
			TiXmlElement * dateElement = element->FirstChildElement();
			weatherCondition.date = dateElement->GetText();
		}
		if (nodeName.compare("atmosinfo") == 0)
		{
			for (TiXmlElement * atmosElement = element->FirstChildElement(); atmosElement; atmosElement = atmosElement->NextSiblingElement())
			{
				string atmosNodeName(atmosElement->Value());
				if (atmosNodeName.compare("solarazimuth") == 0)
				{
					weatherCondition.azimuth = atof(atmosElement->GetText());
				}
				if (atmosNodeName.compare("solarzenith") == 0)
				{
					weatherCondition.zenith = atof(atmosElement->GetText());
				}
				if (atmosNodeName.compare("atmospheremodel") == 0)
				{
					weatherCondition.atmosphereModel = atoi(atmosElement->GetText());
				}
				if (atmosNodeName.compare("aerosolmodel") == 0)
				{
					weatherCondition.aerosolModel = atoi(atmosElement->GetText());
				}
				if (atmosNodeName.compare("visibility") == 0)
				{
					weatherCondition.visibility = atof(atmosElement->GetText());
				}
			}
		}
	}

	return weatherCondition;
}

//读取仿真配置文件，包括场景文件、天气状况文件、调度文件等
Simulation loadSimXML(const char * path)
{
	TiXmlDocument document(path);
	bool result = document.LoadFile();
	if (!result)
	{
		cout << "can not load file" << path << endl;
		exit(0);
	}

	TiXmlElement * root = document.RootElement();
	TiXmlNode * pNode = NULL;
	Simulation simulation;

	for (pNode = root->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		TiXmlElement * element = pNode->ToElement();
		string nodeName(element->Value());
		if (nodeName.compare("scene") == 0)
		{
			simulation.scene = element->FirstAttribute()->Value();
		}
		if (nodeName.compare("weatherconditions") == 0)
		{
			simulation.weatherconditions = element->FirstAttribute()->Value();
		}
		if (nodeName.compare("platform") == 0)
		{
			simulation.platform = element->FirstAttribute()->Value();
		}
		if (nodeName.compare("platformmotion") == 0)
		{
			simulation.platformmotion = element->FirstAttribute()->Value();
		}
		if (nodeName.compare("schedueinfor") == 0)
		{
			simulation.schedueinfor = element->FirstAttribute()->Value();
		}
	}

	return simulation;
}

//解析XML文件
Scene loadSceneXML(const char * path)
{
	Scene scene;
	TiXmlDocument document(path);
	bool result = document.LoadFile();
	if (!result)
	{
		std::cout << "can not load file" << path << std::endl;
		exit(0);
	}

	TiXmlElement * root = document.RootElement();
	TiXmlNode * pNode = NULL;

	for (pNode = root->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		TiXmlElement * element = pNode->ToElement();
		std::string nodeName(element->Value());
		if (nodeName.compare("SCENENAME") == 0)
		{
			scene.name = element->GetText();
		}
		if (nodeName.compare("LATITUDE") == 0)
		{
			scene.latitude = (float)atof(element->GetText());
		}
		if (nodeName.compare("LONGTITUDE") == 0)
		{
			scene.longtitude = (float)atof(element->GetText());
		}
		if (nodeName.compare("ALTITUDE") == 0)
		{
			scene.altitude = (float)atof(element->GetText());
		}
		if (nodeName.compare("COORDINATE") == 0)
		{
			scene.coordinate = element->GetText();
		}
		if (nodeName.compare("OBJECTS") == 0)
		{
			for (TiXmlNode * pChild = element->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				Object obj;
				TiXmlElement * objElement = pChild->ToElement();
				obj.name = objElement->Attribute("name");
				TiXmlElement * objFileElement = objElement->FirstChildElement();
				obj.filePath = objFileElement->GetText();
				TiXmlElement * unitsElement = objFileElement->NextSiblingElement();
				obj.units = unitsElement->GetText();
				TiXmlElement * transformElement = unitsElement->NextSiblingElement();
				std::string transforms = transformElement->FirstChildElement()->GetText();
				sscanf(transforms.c_str(), "%f,%f,%f,%f,%f,%f,%f,%f,%f",
					&obj.transform[0], &obj.transform[1], &obj.transform[2],
					&obj.transform[3], &obj.transform[4], &obj.transform[5],
					&obj.transform[6], &obj.transform[7], &obj.transform[8]);
				//平移-单位转换  m->cm
				obj.transform[0] *= 100;
				obj.transform[1] *= 100;
				obj.transform[2] *= 100;

				//旋转-单位转换 度->弧度
				obj.transform[3] = obj.transform[3] * PI / 180;
				obj.transform[4] = obj.transform[4] * PI / 180;
				obj.transform[5] = obj.transform[5] * PI / 180;

				scene.objects.push_back(obj);
			}
		}
		if (nodeName.compare("VISIBILITY") == 0)
		{
			TiXmlElement * skyVisibilityElement = element->FirstChildElement();
			scene.skyEntry.type = "sky";
			for (TiXmlNode * child = skyVisibilityElement->FirstChild(); child != 0; child = child->NextSibling())
			{
				TiXmlElement * element = child->ToElement();
				std::string childNodeName = element->Value();
				if (childNodeName.compare("zenithinfo") == 0)
				{
					scene.zenithAngle.beginAngle = (float)atof(element->Attribute("begin"));
					scene.zenithAngle.endAngle = (float)atof(element->Attribute("end"));
					scene.zenithAngle.delta = (float)atof(element->Attribute("delta"));
				}
				else if (childNodeName.compare("azimuthinfo") == 0)
				{
					scene.azimuthAngle.beginAngle = (float)atof(element->Attribute("begin"));
					scene.azimuthAngle.endAngle = (float)atof(element->Attribute("end"));
					scene.azimuthAngle.delta = (float)atof(element->Attribute("delta"));
				}
				else if (childNodeName.compare("postifix") == 0)
				{
					scene.skyEntry.postfix = element->GetText();
				}
				else if (childNodeName.compare("directory") == 0)
				{
					scene.skyEntry.directory = element->GetText();
				}

			}
			TiXmlElement * bakVisibilityElement = skyVisibilityElement->NextSiblingElement();
			scene.bakEntry.type = "bak";
			scene.bakEntry.postfix = bakVisibilityElement->FirstChild()->ToElement()->GetText();
			scene.bakEntry.directory = bakVisibilityElement->LastChild()->ToElement()->GetText();
		}
	}

	return scene;
}

Schedule loadScheduleXML(const char * path)
{
	TiXmlDocument document(path);
	bool result = document.LoadFile();
	if (!result)
	{
		cout << "can not load file" << path << endl;
		exit(0);
	}

	TiXmlElement * root = document.RootElement();
	TiXmlNode * pNode = NULL;

	Schedule schedule;

	for (pNode = root->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		TiXmlElement * element = pNode->ToElement();
		string nodeName(element->Value());
		if (nodeName.compare("wavebandinfo") == 0)
		{
			for (TiXmlElement * bandElement = element->FirstChildElement(); bandElement; bandElement = bandElement->NextSiblingElement())
			{
				string bandNodeName(bandElement->Value());
				if (bandNodeName.compare("beginband") == 0)
				{
					schedule.waveBand.beginband = atof(bandElement->GetText());
				}
				if (bandNodeName.compare("endband") == 0)
				{
					schedule.waveBand.endband = atof(bandElement->GetText());
				}
				if (bandNodeName.compare("deltaband") == 0)
				{
					schedule.waveBand.deltaband = atoi(bandElement->GetText());
				}
			}
		}
		if (nodeName.compare("inirradiance") == 0)
		{
			TiXmlElement * outFileElement = element->FirstChildElement();
			for (TiXmlElement * entryElement = outFileElement->FirstChildElement(); entryElement; entryElement = entryElement->NextSiblingElement())
			{
				Entry entry;
				entry.type = entryElement->FirstAttribute()->Value();
				entry.postfix = entryElement->LastAttribute()->Value();
				entry.directory = entryElement->FirstChildElement()->GetText();
				schedule.inEntries.push_back(entry);
			}
		}
		if (nodeName.compare("outirradiance") == 0)
		{
			for (TiXmlElement * childElement = element->FirstChildElement(); childElement; childElement = childElement->NextSiblingElement())
			{
				string childNodeName(childElement->Value());
				if (childNodeName.compare("sample") == 0)
				{
					schedule.nzenith = atoi(childElement->FirstAttribute()->Value());
					schedule.nazimuth = atoi(childElement->LastAttribute()->Value());
				}
				if (childNodeName.compare("outfilename") == 0)
				{
					for (TiXmlElement * entryElement = childElement->FirstChildElement(); entryElement; entryElement = entryElement->NextSiblingElement())
					{
						Entry entry;
						entry.type = entryElement->FirstAttribute()->Value();
						entry.postfix = entryElement->LastAttribute()->Value();
						entry.directory = entryElement->FirstChildElement()->GetText();
						schedule.outEntries.push_back(entry);
					}
				}
			}

		}
	}
	return schedule;
}

//vector<VisibleModel> getVisibleModel(const char* filenametemp){
//
//	vector<VisibleModel> visibleModelList;
//	VisibleModel visibleModel;
//	//const char* filenametemp="d:/scenevisibility/pq_0_0.kvr";
//	ifstream in(filenametemp);
//
//
//	if(!in.good())
//	{
//		cout<<"Open File "<<filenametemp<<" Error!"<<endl;
//	}
//
//	while(!in.eof()){
//		char buffer[32];
//		in.getline(buffer,31);
//		if (strstr(buffer,"No"))
//		{
//			int num;
//			int flag=sscanf(buffer,"%*[^.].%d",&num);
//			if(flag!=1){
//				cout<<"read Visible model num error!"<<endl;
//			}
//			while (1)
//			{
//				char buffer1[32];
//				in.getline(buffer1,31);
//				if(strstr(buffer1,"visible face")){
//					sscanf(buffer1,"visible face: %d",&visibleModel.vFaceCount);
//					visibleModelList.push_back(visibleModel);
//					visibleModel.vFaceNums.clear();
//					break;
//				}else
//				{
//					int i;
//					sscanf(buffer1,"%d",&i);
//					visibleModel.vFaceNums.push_back(i-1);
//				}
//			}
//		}
//
//	}
//	in.close();
//	return visibleModelList;
//}


vector<VisibleModel> getVisibleModel(const char* filenametemp) {

	vector<VisibleModel> visibleModelList;

	FILE *infile;
	infile = fopen(filenametemp, "rb");

	if (infile == NULL)
	{
		cout << "can not open file" << filenametemp << endl;
	}

	BYTE fileHeader;
	WORD modelCount;
	fread(&fileHeader, 1, 1, infile);
	fread(&modelCount, 2, 1, infile);
	fseek(infile, 24L, 0);

	//cout<<"fileHeader: "<<(int)fileHeader<<"    modelCount: "<<modelCount<<endl;

	float zenith, azimuth;
	fread(&zenith, 4, 1, infile);
	fread(&azimuth, 4, 1, infile);

	//cout<<"zenith: "<<zenith<<"    azimuth: "<<azimuth<<endl;

	WORD visibleModelCount;
	fread(&visibleModelCount, 2, 1, infile);

	//cout<<"visibleModelCount: "<<visibleModelCount<<endl;

	for (int i = 0; i < modelCount; i++)
	{
		VisibleModel visibleModel;
		int modelId;
		fread(&modelId, 2, 1, infile);
		fread(&visibleModel.vFaceCount, 4, 1, infile);
		//cout<<"Model Id:"<<modelId<<endl;
		//cout<<"vFaceCount: "<<visibleModel.vFaceCount<<endl;
		for (int j = 0; j < visibleModel.vFaceCount; j++)
		{
			int faceId;
			fread(&faceId, sizeof(int), 1, infile);

			//cout<<"faceId: "<<faceId<<endl;


			visibleModel.vFaceNums.push_back(faceId);
		}
		visibleModelList.push_back(visibleModel);
	}

	fclose(infile);

	return visibleModelList;
}

//计算两点之间的距离
float dist(glm::vec3 p1, glm::vec3 p2)
{
	float dist = ((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) + (p1.z - p2.z)*(p1.z - p2.z));
	return sqrt(dist);
}

//计算直线的方向向量
glm::vec3 lineNormal(glm::vec3 p1, glm::vec3 p2)
{
	glm::vec3 lineNormal;
	lineNormal.x = p1.x - p2.x;
	lineNormal.y = p1.y - p2.y;
	lineNormal.z = p1.z - p2.z;
	return lineNormal;
}

//判断点是否在三角形中
bool pointInTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p0)
{
	glm::vec3 v0, v1, v2;
	v0.x = p2.x - p1.x;
	v0.y = p2.y - p1.y;
	v0.z = p2.z - p1.z;

	v1.x = p3.x - p1.x;
	v1.y = p3.y - p1.y;
	v1.z = p3.z - p1.z;

	v2.x = p0.x - p1.x;
	v2.y = p0.y - p1.y;
	v2.z = p0.z - p1.z;

	float dot00 = v0.x*v0.x + v0.y*v0.y + v0.z*v0.z;
	float dot01 = v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;
	float dot02 = v0.x*v2.x + v0.y*v2.y + v0.z*v2.z;
	float dot11 = v1.x*v1.x + v1.y*v1.y + v1.z*v1.z;
	float dot12 = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

	float inverDeno = 1 / (dot00*dot11 - dot01*dot01);
	float u = (dot11*dot02 - dot01*dot12)*inverDeno;

	if (u < 0 || u > 1)
	{
		return false;
	}
	float v = (dot00*dot12 - dot01*dot02)*inverDeno;
	if (v < 0 || v > 1)
	{
		return false;
	}

	return u + v <= 1;
}


//点到直线的距离
float pointToLine(glm::vec3 point, glm::vec3 sunPosition, glm::vec3 sunLine)
{
	float t = (sunLine.x*(point.x - sunPosition.x) + sunLine.y*(point.y - sunPosition.y) + sunLine.z*(point.z - sunPosition.z)) / (sunLine.x*sunLine.x + sunLine.y*sunLine.y + sunLine.z*sunLine.z);
	//点在直线上的投影
	glm::vec3 temp;
	temp.x = sunPosition.x + sunLine.x*t;
	temp.y = sunPosition.y + sunLine.y*t;
	temp.z = sunPosition.z + sunLine.z*t;
	return dist(point, temp);
}



//判断某个面片的可见性--基于场景(0 : invisible ; 1 : visible)
int isFaceVisible(TriangleFace face, std::vector<Model*> models, int modelID, int faceID, glm::vec3 position)
{
	int result = 1;
	glm::vec3 centerTarget = face.center;
	glm::vec3 ln = lineNormal(position, centerTarget);

	for (int modelIndex = 0; modelIndex < models.size(); ++modelIndex)
	{
		if (result == 0)
		{
			break;
		}

		Model * model = models[modelIndex];
		std::vector<TriangleFace> faces = model->faces;
		std::vector<glm::vec3> vertices = model->vertices;
		for (int faceIndex = 0; faceIndex < faces.size(); ++faceIndex)
		{
			if (modelIndex == modelID && faceIndex == faceID)
			{
				continue;
			}
			glm::vec3 centerCurrent = faces[faceIndex].center;
			glm::vec3 tn = faces[faceIndex].triangleNormal;

			//平面法向量*直线法向量
			float temp1 = tn.x*ln.x + tn.y*ln.y + tn.z*ln.z;
			if (temp1 == 0.0)
			{
				//平行或者在平面上（可见）
				continue;
			}
			//平面方程常数项
			float d = -(tn.x*centerCurrent.x + tn.y*centerCurrent.y + tn.z*centerCurrent.z);
			float temp2 = tn.x*centerTarget.x + tn.y*centerTarget.y + tn.z*centerTarget.z + d;
			//直线参数方程的参数t
			float t = -temp2 / temp1;

			//直线和平面的交点p
			glm::vec3 p;
			p.x = ln.x*t + centerTarget.x;
			p.y = ln.y*t + centerTarget.y;
			p.z = ln.z*t + centerTarget.z;

			//判断交点是否在线段上
			glm::vec3 ln1 = lineNormal(p, centerTarget);
			glm::vec3 ln2 = lineNormal(p, position);
			float direction = ln1.x*ln2.x + ln1.y*ln2.y + ln1.z*ln2.z;

			if (direction > 0.0)
			{
				//交点不在线段上
				continue;
			}

			glm::vec3 p1 = vertices[faces[faceIndex].vertexIndex1 - 1];
			glm::vec3 p2 = vertices[faces[faceIndex].vertexIndex2 - 1];
			glm::vec3 p3 = vertices[faces[faceIndex].vertexIndex3 - 1];

			if (!pointInTriangle(p1, p2, p3, p))
			{
				continue;
			}

			result = 0;
			break;
		}
	}
	return result;
}



float selectMin(float f1, float f2, float f3) {
	float temp;
	if (f1 <= f2) {
		temp = f1;
	}
	else {
		temp = f2;
	}

	if (temp > f3) {
		temp = f3;
	}
	return temp;
}

float selectMax(float f1, float f2, float f3) {
	float temp;
	if (f1 >= f2) {
		temp = f1;
	}
	else {
		temp = f2;
	}

	if (f3 > temp) {
		temp = f3;
	}
	return temp;
}

//读取MTM文件
vector<int> loadMTMFile(const char* fileName, int & length, int & width)
{
	FILE *fp;
	int allPixels;
	if ((fp = fopen(fileName, "rb")) == NULL)
	{
		cout << "can not open file" << fileName << endl;
	}
	fread(&length, 4, 1, fp);
	fread(&width, 4, 1, fp);
	int materialID;
	vector<int> materialIDs;
	allPixels = length*width;
	for (int i = 0; i < allPixels; ++i)
	{
		fread(&materialID, 4, 1, fp);
		materialIDs.push_back(materialID);
	}
	fclose(fp);
	return materialIDs;
}


//解析MTL文件
vector<MtlModel> loadMtlFile(const char* filepath) {

	vector<MtlModel> materials;
	ifstream in(filepath);

	if (!in.good()) {
		cout << "Open File " << filepath << " Error!" << std::endl;
		return materials;
	}

	while (1)
	{
		char buffer[128];

		if (in.getline(buffer, 127).eof())
		{
			break;
		}

		switch (buffer[0])
		{
		case '#'://comment
			break;
		case 'n'://newmtl
			MtlModel material;
			if (sscanf(buffer, "newmtl %s", material.name) == 1)
			{
				while (1)
				{
					char tempBuffer[128];
					in.getline(tempBuffer, 127);
					if (tempBuffer[0] == NULL)
					{
						break;
					}
					if (strstr(tempBuffer, "map_Ka"))
					{
						sscanf(tempBuffer, "	map_Ka %s", material.map_Ka);
					}
					else if (strstr(tempBuffer, "map_Kd"))
					{
						sscanf(tempBuffer, "	map_Kd %s", material.map_Kd);
					}
					else if (strstr(tempBuffer, "map_Ks"))
					{
						sscanf(tempBuffer, "	map_Ks %s", material.map_Ks);
					}
					if (in.eof())
					{
						break;
					}
				}
				materials.push_back(material);
			}
			break;
		default:
			break;
		}
	}
	in.close();
	return materials;
}

//计算每个面片的材质编号和所占百分比
vector<map<int, float>> getMaterialInfo(Model* model) {


	//(1)
	//获得obj文件所在位置,mtl和mtm文件与obj文件在同一目录下
	string modelFileLocation = model->modelFileLocation;
	string mtlFile_s = modelFileLocation + model->materialFileName;
	const char* materialMtlFile = mtlFile_s.c_str();

	//(2)
	//获取模型中所用到的所有材质信息，存放在materials中
	vector<MtlModel> materials = loadMtlFile(materialMtlFile);

	int width, length;  //width图像的宽（或者说是高），length表示图像的长
	vector<int> pixels;//将mtm里的所有像素数据存放在pixels数组里
	glm::vec3 uv1, uv2, uv3;
	vector<map<int, float>> faceMaterialInfo;	//
	string lastMaterialName = "nomaterial";

	for (int i = 0; i < model->faces.size(); i++)
	{

		//(3)
		string currentMaterial = model->faces[i].material;
		if (currentMaterial.compare(lastMaterialName) != 0) {
			for (int m = 0; m < materials.size(); m++)
			{
				MtlModel material = materials[m];
				if (currentMaterial.compare(material.name) == 0)
				{
					string tgafile = material.map_Ka;
					string mtmFilePath = modelFileLocation + tgafile.substr(0, tgafile.size() - 4) + ".mtm";
					const char* materialFile = mtmFilePath.c_str();
					pixels = loadMTMFile(materialFile, length, width);
					break;
				}
			}
			lastMaterialName = model->faces[i].material;
			//cout<<currentMaterial<<endl;
		}


		//(4)
		int uvindex1, uvindex2, uvindex3;
		uvindex1 = model->faces[i].uvIndex1;
		uvindex2 = model->faces[i].uvIndex2;
		uvindex3 = model->faces[i].uvIndex3;

		uv1.x = model->uvs[uvindex1 - 1].x;
		uv1.y = model->uvs[uvindex1 - 1].y;
		uv1.z = model->uvs[uvindex1 - 1].z;
		//uv1.z=0;
		uv2.x = model->uvs[uvindex2 - 1].x;
		uv2.y = model->uvs[uvindex2 - 1].y;
		uv2.z = model->uvs[uvindex2 - 1].z;
		//uv2.z=0;
		uv3.x = model->uvs[uvindex3 - 1].x;
		uv3.y = model->uvs[uvindex3 - 1].y;
		uv3.z = model->uvs[uvindex3 - 1].z;
		//uv3.z=0;

		if (uv1.x < -1) {
			uv1.x = uv1.x*(-1);
			uv1.x = uv1.x - (int)uv1.x;
		}
		else if (uv1.x >= -1 && uv1.x < 0) {
			uv1.x = uv1.x*(-1);
		}
		else if (uv1.x > 1) {
			uv1.x = uv1.x - (int)uv1.x;
		}

		if (uv1.y < -1) {
			uv1.y = uv1.y*(-1);
			uv1.y = uv1.y - (int)uv1.y;
		}
		else if (uv1.y >= -1 && uv1.y < 0) {
			uv1.y = uv1.y*(-1);
		}
		else if (uv1.y > 1) {
			uv1.y = uv1.y - (int)uv1.y;
		}


		if (uv2.x < -1) {
			uv2.x = uv2.x*(-1);
			uv2.x = uv2.x - (int)uv2.x;
		}
		else if (uv2.x >= -1 && uv2.x < 0) {
			uv2.x = uv2.x*(-1);
		}
		else if (uv2.x > 1) {
			uv2.x = uv2.x - (int)uv2.x;
		}

		if (uv2.y < -1) {
			uv2.y = uv2.y*(-1);
			uv2.y = uv2.y - (int)uv2.y;
		}
		else if (uv2.y >= -1 && uv2.y < 0) {
			uv2.y = uv2.y*(-1);
		}
		else if (uv2.y > 1) {
			uv2.y = uv2.y - (int)uv2.y;
		}


		if (uv3.x < -1) {
			uv3.x = uv3.x*(-1);
			uv3.x = uv3.x - (int)uv3.x;
		}
		else if (uv3.x >= -1 && uv3.x < 0) {
			uv3.x = uv3.x*(-1);
		}
		else if (uv3.x > 1) {
			uv3.x = uv3.x - (int)uv3.x;
		}

		if (uv3.y < -1) {
			uv3.y = uv3.y*(-1);
			uv3.y = uv3.y - (int)uv3.y;
		}
		else if (uv3.y >= -1 && uv3.y < 0) {
			uv3.y = uv3.y*(-1);
		}
		else if (uv3.y > 1) {
			uv3.y = uv3.y - (int)uv3.y;
		}

		//(5)
		float min_x, max_x, min_y, max_y;
		int minpixel_x, maxpixel_x, minpixel_y, maxpixel_y;

		min_x = selectMin(uv1.x, uv2.x, uv3.x);
		max_x = selectMax(uv1.x, uv2.x, uv3.x);
		min_y = selectMin(uv1.y, uv2.y, uv3.y);
		max_y = selectMax(uv1.y, uv2.y, uv3.y);

		//划定一个矩形区域
		minpixel_x = min_x*length;
		maxpixel_x = max_x*length;
		minpixel_y = min_y*width;
		maxpixel_y = max_y*width;

		uv1.x = (int)uv1.x*length;
		uv2.x = (int)uv2.x*length;
		uv3.x = (int)uv3.x*length;

		uv1.y = (int)uv1.y*width;
		uv2.y = (int)uv2.y*width;
		uv3.y = (int)uv3.y*width;

		if (maxpixel_x - minpixel_x == 0) {
			++maxpixel_x;
		}
		if (maxpixel_y - minpixel_y == 0) {
			++maxpixel_y;
		}

		//(6)
		map<int, int> typeStatMap;  //统计每类材质有多少个像素点
		int sumPixels = 0;   //共有多少个点在三角形中
		for (int y = minpixel_y; y < maxpixel_y; y++)
		{
			for (int x = minpixel_x; x < maxpixel_x; x++)
			{
				glm::vec3 temp(x, y, 0);

				bool flag = pointInTriangle(uv1, uv2, uv3, temp);
				if (flag) {
					++sumPixels;
					int m = (width - y - 1)*length + x;   //获得数组pixels的下标
					//cout<<m<<endl;
					typeStatMap[pixels[m]]++;    //将一个面片的所有材质进行统计
				}
			}
		}

		//cout<<"在三角形内的点数：  "<<sumPixels<<endl;

		//(7)
		//转换成比例
		map<int, float> typeInfoMap;
		map<int, int>::iterator it;
		for (it = typeStatMap.begin(); it != typeStatMap.end(); it++)
		{
			int temp = it->first;
			//cout<<temp<<endl;
			//cout<<it->second<<endl;
			float percent = it->second*(1.0) / sumPixels;
			typeInfoMap[temp] = percent;
			//cout<<temp<<": "<<percent<<endl;
		}
		faceMaterialInfo.push_back(typeInfoMap);
	}
	return faceMaterialInfo;
}


//求解变换矩阵
void getModelMatrix(float modelMatrix[4][4], float translateMatrix[4][4], float rotateXMatrix[4][4], float rotateYMatrix[4][4], float rotateZMatrix[4][4], float scaleMatrix[4][4])
{
	float temp1[4][4], temp2[4][4], temp3[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp1[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				temp1[i][j] += translateMatrix[i][k] * rotateXMatrix[k][j];
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp2[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				temp2[i][j] += temp1[i][k] * rotateYMatrix[k][j];
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp3[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				temp3[i][j] += temp2[i][k] * rotateZMatrix[k][j];
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			modelMatrix[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				modelMatrix[i][j] += temp3[i][k] * scaleMatrix[k][j];
			}
		}
	}
}

//模型坐标转换到场景坐标
glm::vec3 transformToScene(glm::vec3 modelPosition, float modelMatrix[4][4])
{
	float temp[4];
	glm::vec3 scenePosition;
	for (int i = 0; i < 4; i++)
	{
		temp[i] = modelPosition.x*modelMatrix[i][0] + modelPosition.y*modelMatrix[i][1] + modelPosition.z*modelMatrix[i][2] + modelMatrix[i][3];
	}
	scenePosition.x = temp[0];
	scenePosition.y = temp[1];
	scenePosition.z = temp[2];

	return scenePosition;
}

string num2str(float num) {
	stringstream ss;
	ss << num;
	return ss.str();
}

string num2str(int num) {

	stringstream ss;
	if (num < 10 && num>0) {
		ss << '0' << num;
	}
	else {
		ss << num;
	}
	return ss.str();
}

float str2num(string str) {
	float num;
	stringstream ss(str);
	ss >> num;
	return num;
}

//将太阳入射的结果写入文件
void writeToFile(const char* filePath, vector<vector<FileFormat>> sceneModelSunIRs) {

	FILE *outFile, *file;
	int sceneModelCount = sceneModelSunIRs.size();
	vector<FileFormat> wfList;
	float sunIRMax = 0;


	outFile = fopen(filePath, "rb");
	//判断文件是否已存在在，存在则删除原文件
	if (outFile)
	{
		//cout<<outSunfilepath<<endl;
		fclose(outFile);
		remove(filePath);
	}

	file = fopen(filePath, "ab");
	//预留24字节
	int num = 24;
	fwrite(&num, 1, 1, file);
	fwrite(&sceneModelCount, 2, 1, file);

	for (int m = 0; m < 21; m++) {
		char fillchar = 'a';
		fwrite(&fillchar, 1, 1, file);
	}


	for (int i = 0; i < sceneModelCount; i++)
	{
		wfList = sceneModelSunIRs[i];
		int modelFaceCount = wfList.size();

		fwrite(&i, 2, 1, file);
		fwrite(&modelFaceCount, 4, 1, file);
		for (int j = 0; j < modelFaceCount; j++)
		{
			if (sunIRMax <= wfList[j].fvalue)
			{
				sunIRMax = wfList[j].fvalue;
			}
			fwrite(&wfList[j], sizeof(FileFormat), 1, file);   //能否一次全部读写
		}
		//fwrite(&wfList[i],sizeof(FileFormat),wfList.size(),outSunfilepath);   //尝试一次全部读写
		wfList.clear();
	}
	fclose(file);

	file = fopen(filePath, "rb+");
	fseek(file, 3L, 0);
	fwrite(&sunIRMax, 4, 1, file);
	fclose(file);
}

//将天空漫射辐亮度写入文件
void writeToFile(const char* filePath, vector<vector<SubFileFormat>> sceneModelSkyIRs) {

	FILE *outFile, *file;
	int sceneModelCount = sceneModelSkyIRs.size();
	vector<SubFileFormat> wfList;

	FileFormat wFile;
	SubFileFormat sWFile;


	outFile = fopen(filePath, "rb");
	//判断文件是否已存在在，存在则删除原文件
	if (outFile)
	{
		//cout<<outSunfilepath<<endl;
		fclose(outFile);
		remove(filePath);
	}

	file = fopen(filePath, "ab");
	//预留24字节
	int num = 24;
	fwrite(&num, 1, 1, file);
	fwrite(&sceneModelCount, 2, 1, file);
	for (int m = 0; m < 21; m++) {
		char fillchar = 'a';
		fwrite(&fillchar, 1, 1, file);
	}


	for (int i = 0; i < sceneModelCount; i++)
	{
		wfList = sceneModelSkyIRs[i];
		int modelFaceCount = wfList.size();

		fwrite(&i, 2, 1, file);
		fwrite(&modelFaceCount, 4, 1, file);
		for (int j = 0; j < modelFaceCount; j++)
		{
			sWFile = wfList[j];
			wFile.azimuth = sWFile.azimuth;
			wFile.zenith = sWFile.zenith;
			wFile.faceId = sWFile.faceId;
			wFile.fvalue = sWFile.fvalue;
			fwrite(&wFile, sizeof(wFile), 1, file);   //能否一次全部读写
		}
		wfList.clear();
	}

	fclose(file);
}

//将模型中的每个面片的整个半球天空漫射的结果写入文件
void writeToFile(const char* filePath, map<int, map<int, float>> sceneFaceSumSkyIEnergy) {

	FILE *outFile, *file;
	float skyIRMax = 0;
	int sceneModelCount = sceneFaceSumSkyIEnergy.size();


	outFile = fopen(filePath, "rb");
	//判断文件是否已存在在，存在则删除原文件
	if (outFile)
	{
		//cout<<outSunfilepath<<endl;
		fclose(outFile);
		remove(filePath);
	}

	file = fopen(filePath, "ab");
	//预留24字节
	int num = 24;
	fwrite(&num, 1, 1, file);
	fwrite(&sceneModelCount, 2, 1, file);
	for (int m = 0; m < 21; m++) {
		char fillchar = 'a';
		fwrite(&fillchar, 1, 1, file);
	}


	for (int i = 0; i < sceneModelCount; i++)
	{
		map<int, float> modelSkyEnergy = sceneFaceSumSkyIEnergy[i];
		int modelFaceCount = modelSkyEnergy.size();

		fwrite(&i, 2, 1, file);
		fwrite(&modelFaceCount, 4, 1, file);
		for (int j = 0; j < modelFaceCount; j++)
		{
			FaceSkyEnergy faceSkyEnergy;
			faceSkyEnergy.faceId = j;
			faceSkyEnergy.faceEnergy = modelSkyEnergy[j];
			if (skyIRMax <= faceSkyEnergy.faceEnergy)
			{
				skyIRMax = faceSkyEnergy.faceEnergy;
			}
			//cout<<faceSkyEnergy.faceEnergy<<endl;
			fwrite(&faceSkyEnergy, sizeof(FaceSkyEnergy), 1, file);
		}
	}
	fclose(file);


	file = fopen(filePath, "rb+");
	fseek(file, 3L, 0);
	fwrite(&skyIRMax, 4, 1, file);
	fclose(file);

}

//根据标识flag，判断所需的是太阳入射的文件信息，还是天空漫射或背景对目标的文件信息
//flag=1是太阳入射的文件信息，flag=2是天空漫射的文件信息，flag=3是背景对目标的文件信息
Entry getFilePath(int flag, vector<Entry> inEntries) {

	string type;
	Entry entry;
	if (flag == 1) {
		type = "sun";
	}
	else if (flag == 2) {
		type = "sky";
	}
	else if (flag == 3) {
		type = "bak";
	}

	//cout<<type<<endl;

	for (int i = 0; i < inEntries.size(); ++i)
	{
		entry = inEntries[i];
		if (entry.type.compare(type) == 0)
		{
			//cout<<entry.type<<endl;
			return entry;
		}
	}
	cout << "entry is null!" << endl;
	return entry;
}

//获得观测方向与太阳光方向的相对方位角,返回值为角度
float getRelativeAzimuth(float viewAzimuth, float solarAzimuth) {

	float relativeViewAzimuth;

	if (solarAzimuth >= viewAzimuth)
	{
		if (solarAzimuth - viewAzimuth <= 180)
		{
			relativeViewAzimuth = solarAzimuth - viewAzimuth;
		}
		else {
			//消除-0的情况
			if (360 - (solarAzimuth - viewAzimuth))
			{
				relativeViewAzimuth = -1 * (360 - (solarAzimuth - viewAzimuth));
			}
			else {
				relativeViewAzimuth = 360 - (solarAzimuth - viewAzimuth);
			}
		}
	}
	else {
		if (viewAzimuth - solarAzimuth <= 180)
		{
			relativeViewAzimuth = -1 * (viewAzimuth - solarAzimuth);
		}
		else {
			relativeViewAzimuth = 360 - (viewAzimuth - solarAzimuth);
		}
	}

	return relativeViewAzimuth;

}

//获得应该生成的总文件个数
int getFileCount(Scene scene, Schedule schedule) {
	//获得天顶角、方位角配置参数
	Angle skyZenithAngle = scene.zenithAngle;
	Angle skyAzimuthAngle = scene.azimuthAngle;
	Entry skyEntry = scene.skyEntry;

	//取得调度文件中的各种参数
	WaveBand waveband = schedule.waveBand;
	int vzenithCount = 0;
	int azimuthCount = 0;
	int waveLenCount = 0;
	for (float temp = skyZenithAngle.beginAngle; temp <= skyZenithAngle.endAngle; temp += skyZenithAngle.delta)
	{
		vzenithCount++;
	}
	for (float temp = skyAzimuthAngle.beginAngle; temp < skyAzimuthAngle.endAngle; temp += skyAzimuthAngle.delta)
	{
		azimuthCount++;
	}

	for (float temp = waveband.beginband; temp <= waveband.endband; temp += waveband.deltaband)
	{
		waveLenCount++;
	}

	int skyFileCount = vzenithCount*azimuthCount + 1;
	int sunFileCount = 1;
	int mainCount = waveLenCount*(skyFileCount + sunFileCount);
	return mainCount;
}

