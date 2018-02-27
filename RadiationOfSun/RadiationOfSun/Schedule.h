#ifndef HEADER_SCHEDULE
#define HEADER_SCHEDULE
#include "WaveBand.h"
#include "Entry.h"
#include "vector"

class Schedule{
public:
	int nzenith,nazimuth;
	WaveBand waveBand;
	std::vector<Entry> inEntries;
	std::vector<Entry> outEntries;
};

#endif