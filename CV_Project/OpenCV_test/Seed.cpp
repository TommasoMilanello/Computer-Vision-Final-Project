#include "Seed.h"

Seed::Seed(int x, int y, int left, int top, int width, int height, int areaConnComp) {
	int X = x;
	int Y = y;
	int Left = left;
	int Top = top;
	int Width = width;
	int Height = height;
	int AreaConnComp = areaConnComp;
}

Seed Seed::split() {
	if (this->Width >= this->Height) {
		int newWidth = static_cast<int>(static_cast<float>(this->Width) / 2);

		this->X = this->Left + static_cast<int>(static_cast<float>(newWidth) / 2);
		this->Width = newWidth;

		return Seed(this->X + newWidth, this->Y, this->Left, this->Top, newWidth, this->Height, this->AreaConnComp);
	}
	else {
		int newHeight = static_cast<int>(static_cast<float>(this->Height) / 2);

		this->X = this->Left + static_cast<int>(static_cast<float>(newHeight) / 2);
		this->Height = newHeight;

		return Seed(this->X + newHeight, this->Y, this->Left, this->Top, newHeight, this->Height, this->AreaConnComp);
	}
}