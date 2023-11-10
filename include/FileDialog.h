#pragma once

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <string>

using namespace std;

class FileDialog 
{
public:
	static string OpenFile(const char* filter);
	static string SaveFile(const char* filter);
};

#endif