// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <atlstr.h>
#include <sys/stat.h>
#include <sys/stat.h>

#include <iostream>
#include <Windows.h>
#include <vector>
#include "string.h"
#include "conio.h"
#include "strsafe.h"
#include <process.h>

#include "INIReader.h"
#include "ini.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;


/// \brief Funkcja sluzaca do wyrownwania jasnosci obrazu

/// \details Funkcja dokonuje operacji wyrownania jasnosci na wszystkich obrazach znajdujacych sie wewnatrz wektora <i>data</i>

/// \param <i>data</i> wektor zawierajacy sciezki do obrazow

void  EqualizeImages(void *data);

// TODO: reference additional headers your program requires here
