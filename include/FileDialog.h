#pragma once

#include <string>

using namespace std;

class FileDialog 
{
public:
	static string OpenFile(const char* filter);
	static string SaveFile(const char* filter);
};