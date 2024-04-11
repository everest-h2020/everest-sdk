#ifndef FCD_HPP
#define FCD_HPP

#include <fstream>
#include "structs.h"
#include <string>
#include <map>
#include <vector>


class FCDrecord
{
public:
	double lat;
	double lon;
	int speed;
	int bearing;
	int tis;  // time info second
	std::string session;
	int linenr;  // debug purposes
};

using FCDlist = std::vector<FCDrecord>;

class FCD {

private:
	GpsRectangle boundary;
	std::ifstream& fcddata;
	FCDlist fcdlist;

	int fcd_index;
	std::map <std::string, GpsVector> trajectories;

private:	
	bool InBounds(double lat, double lon);

public:
	FCD(std::ifstream& _fcddata, GpsRectangle& _boundary);
	void initialize(int format);
	void reset();
	GpsVector get_next_vector(std::string& vid);
	int fcd_size() { return fcdlist.size(); }
	const FCDrecord& fcd_point(int i) { return fcdlist[i]; }

};

#endif
