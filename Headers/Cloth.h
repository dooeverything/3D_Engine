#pragma once

#include <iostream>

using namespace std;

class Cloth
{
public:
	Cloth(int width, int height);
	~Cloth();

	void update(float t);

private:
	void updateForce();
	
	int width;
	int height;
};