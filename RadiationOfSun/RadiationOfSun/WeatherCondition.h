#ifndef HEADER_WEATHER_CONDITIONS
#define HEADER_WEATHER_CONDITIONS

class WeatherCondition
{
public:
	std::string date;
	float azimuth;
	float zenith;
	float visibility;
	int atmosphereModel;
	int aerosolModel;
};

#endif