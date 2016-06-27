// PSC_Ychanger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/// \brief Klasa przechowujaca parametry dla watku 

/// \details Klasa przechowuje takie parametry jak indeks startowy dla watku, krok przeszukiwania wektora oraz wskazniki dla wektora sciezek zrodlowych oraz wektora obrazow docelowych

struct EqImgsParam {
	int startIndex;
	int step;
	vector<string> &start_img;
	vector<Mat> &final_img;
};

	/// \file

	/// \brief Funkcja sluzaca do wyrownwania jasnosci obrazu

	/// \details Funkcja dokonuje operacji wyrownania jasnosci na wszystkich obrazach znajdujacych sie wewnatrz wektora <i>data</i>

	/// \param <i>data</i> wektor zawierajacy sciezki do obrazow

void  EqualizeImages(void *data) {
	EqImgsParam *params = (EqImgsParam*)data;
	for (int i = params->startIndex; i < params->start_img.size(); i += params->step)
	{
		Mat temporary;
		Mat start_image;
		start_image = imread(params->start_img[i]);

		vector<Mat> channels;
		cvtColor(start_image, start_image, CV_BGR2YCrCb);

		split(start_image, channels);
		equalizeHist(channels[0], channels[0]);

		merge(channels, temporary);

		cvtColor(start_image, start_image, CV_YCrCb2BGR);
		cvtColor(temporary, temporary, CV_YCrCb2BGR);
		params->final_img.push_back(temporary);
	}
}


int main()
{
	struct stat info;

	
	vector<Mat> start_img, final_img;

	INIReader reader("Path.ini");

	// Sprawdzenie czy plik ini istnieje
	if (reader.ParseError() < 0) {
		cout << "Can't load 'Path.ini'\n";
		cout << "Press any key." << endl;
		_getch();
		return 1;
	}


	WIN32_FIND_DATA FileName;
	string read_path = reader.Get("PSC", "ReadPath", "UNKNOWN");
	string write_path = reader.Get("PSC", "WritePath", read_path);

	/*
		Ponizej znajduje sie kod odpowiedzialny za sprawdzenie poprawnosci sciezek zdefiniowanych w pliku .ini
		Wykorzystywana jest funkcja stat sluzaca do zwracania informacji o podanym pliku badz sciezce
		W przypadku kiedy sciezka zrodlowa nie istnieje dzialanie programu jest przerywane,
		jezeli sciezka docelowa nie istnieje najpierw podejmowana jest proba jej utworzenia, w przypadku
		niepowodzenia dzialanie programu jest przerywane
	*/

	if (stat(read_path.c_str(), &info) != 0)
	{
		printf("Brak dostepu do sciezki odczytu %s\n", read_path.c_str());
		printf("Przerwanie wykonywania programu.\nNacisnij dowolny klawisz.");
		_getch();
		return 0;
	}
	else if (info.st_mode & S_IFDIR)
		printf("Sciezka odczytu - poprawna.\n");
	else
	{
		printf("Podana sciezka odczytu nie istnieje.\n");
		printf("Przerwanie wykonywania programu.\nNacisnij dowolny klawisz.");
		_getch();
		return 0;
	}

	if (stat(write_path.c_str(), &info) != 0)
	{
		printf("Brak dostepu do sciezki zapisu %s\n", write_path.c_str());
		printf("Sciezka moze nie istniec.\nPodjecie proby utworzenia sciezki.\n");
		int success_flag = CreateDirectoryA(write_path.c_str(), NULL);
		if (success_flag != 0)
			printf("Utworzono sciezke zapisu.\n");
		else
		{
			printf("Utworznie sciezki nie powiodlo sie.\n");
			printf("Przerwanie wykonywania programu.\nNacisniej dowolny klawisz.");
			_getch();
			return 0;
		}
	}
	else if (info.st_mode & S_IFDIR)
		printf("Sciezka zapisu - poprawna.\n");
	else
	{
		printf("Podana sciezka zapisu nie istnieje.\n");
		printf("Sciezka zapisu zostanie utworzona.\n");
		int success_flag = CreateDirectoryA(write_path.c_str(), NULL);
		if (success_flag != 0)
			printf("Utworzono sciezke zapisu.\n");
		else
		{
			printf("Utworznie sciezki nie powiodlo sie.\n");
			printf("Przerwanie wykonywania programu.\nNacisnij dowolny klawisz.");
			_getch();
			return 0;
		}
	}

	TCHAR sciezka[MAX_PATH];
	_tcscpy_s(sciezka, CA2T(read_path.c_str()));
	TCHAR rozszerzenie[] = TEXT("/*.jpg\0");
	TCHAR roz_sciezka[MAX_PATH];
	vector<string> FileList;
	
	
	
	StringCchCopy(roz_sciezka, MAX_PATH, sciezka);
	StringCchCat(roz_sciezka, MAX_PATH, rozszerzenie);
	
	
	HANDLE hfind = FindFirstFile(roz_sciezka, &FileName);
	
	/*
		Ponizej znajduje sie petla przegladajaca sciezke zrodlowa w poszukiwaniu plikow .jpg
	*/

	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t *f_name = FileName.cFileName;
			wchar_t *sciezka_file_name = sciezka;
			wstring ws(f_name);
			wstring ws2(sciezka_file_name);
			string str(ws.begin(), ws.end());
			string file_name(ws2.begin(), ws2.end());
			string full_file_name = file_name + "/" + str;
			FileList.push_back(full_file_name);
		} while (FindNextFile(hfind, &FileName));
	}
	
	int n = FileList.size();
	
	
	int row = ceil(sqrt((double)n));
	int column = ceil((double)n / row);

	int s = 800. / column;
	int w = 0.75*s;

	Mat mozaika(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));
	Mat mozaika2(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));
	


	const int threadCount = 2;
	EqImgsParam params[threadCount] = {
		{
			0,
			2,
			FileList,
			final_img
		},
		{
			1,
			2,
			FileList,
			final_img
		}

	};

	

	HANDLE threads[threadCount];

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i] = (HANDLE)_beginthread(&EqualizeImages, 0, &params[i]);
	}

		WaitForMultipleObjects(threadCount, threads, TRUE, INFINITE); //oczekiwanie az watki zakoncza dzialanie

	/*
		Petla w ktorej nastepuje sklejanie obrazow w mozaiki
	*/
	for (int i = 0; i < FileList.size(); i++)
	{
		int x = i%column*s;
		int y = i / column*w;
		int z;
		if (i % 2 != 0)
		{
			z = i - i / 2 - 1;
		}
		else
		{
			z = FileList.size() / 2 + i / 2;
		}
		Mat roi = mozaika(Rect(x, y, s, w));
		Mat roi2 = mozaika2(Rect(x, y, s, w));
		resize(imread(FileList[i]), roi, roi.size());
		resize(final_img[z], roi2, roi2.size());
		namedWindow("Start_images", CV_WINDOW_KEEPRATIO);
		namedWindow("Final_images", CV_WINDOW_KEEPRATIO);
		imshow("Start_images", mozaika);
		imshow("Final_images", mozaika2);
	}
	imwrite(write_path + "/Start_images.jpg", mozaika);
	imwrite(write_path + "/Final_images.jpg", mozaika2);
	waitKey(0);
    return 0;
}

