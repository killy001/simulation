#ifndef HEADER_SCENE
#define HEADER_SCENE
#include "Object.h"
#include "vector"
#include "Angle.h"
#include "Entry.h"

class Scene{
public:
	std::string name;
	float latitude;
	float longtitude;
	float altitude;
	std::string coordinate;
	std::vector<Object> objects;

	Angle azimuthAngle;
	Angle zenithAngle;
	Entry skyEntry;
	Entry bakEntry;
};

#endif