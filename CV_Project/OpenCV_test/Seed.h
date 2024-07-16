#ifndef SEED_H
#define SEED_H

#include <opencv2/highgui.hpp>

class Seed {
public:
	int X;
	int Y;
	int Left;
	int Top;
	int Width;
	int Height;
	int AreaConnComp;

	Seed(int x, int y, int left, int top, int width, int height, int areaConnComp);
	Seed split(); // returns a splitted seed form itself 
};

#endif