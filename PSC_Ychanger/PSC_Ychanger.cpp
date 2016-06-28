// PSC_Ychanger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/// \brief Klasa przechowujaca parametry dla watku 

/// \details Klasa przechowuje takie parametry jak indeks startowy dla watku, krok przeszukiwania wektora oraz wskazniki dla wektora sciezek zrodlowych oraz wektora obrazow docelowych
/// \param <i>startIndex</i> indeks od którego watek zacznie przetwarzanie
/// \param <i>step</i> co ktory obraz bedzie przetwarzany przez watek (zazwyczaj rowne liczbie watkow)
/// \param <i>&start_img</i> referencja do wektora ze siezkami obrazow startowych
/// \param <i>&final_img</i> referencja do wektora obrazow przetworzonych

struct EqImgsParam {
	int startIndex;
	int step;
	vector<string> &start_img;
	vector<Mat> &final_img;
};

	/// \file

	/// \brief Funkcja sluzaca do wyrownwania jasnosci obrazu

	/// \details Funkcja dokonuje operacji wyrownania jasnosci na wszystkich obrazach znajdujacych sie wewnatrz wektora <i>data</i>

	/// \param <i>data</i> struktura przechowujaca parametry konwersji

void  EqualizeImages(void *data) {
	EqImgsParam *params = (EqImgsParam*)data;				// Rzutowanie z powrotem na typ EqImgsParam. _beginthreadex() oczekuje void FunkcjaWatku(void*).
	for (int i = params->startIndex; i < params->start_img.size(); i += params->step)
	{
		Mat temporary;										//macierz przechowujaca zmieniane zdjecie
		Mat start_image;									//macierz przechowujaca zdjecie poczatkowe
		start_image = imread(params->start_img[i]);			//wczytanie zdjecia z zadanej sciezki (start_img[i] to sciezka)

		vector<Mat> channels;								//vektor macierzy przechowujacy obraz w 3 skladowych (Y, Cr,Cb)
		cvtColor(start_image, start_image, CV_BGR2YCrCb);	//zamiana z RGB na YCrCb

		split(start_image, channels);						//rozdzielenie skladowych obrazu na 3 macierze
		equalizeHist(channels[0], channels[0]);				//wyrownanie histogramu na podstawie skladowej luminancji

		merge(channels, temporary);							//zastosowanie zmienionej macierzy luminancji do obrazu

		cvtColor(start_image, start_image, CV_YCrCb2BGR);	//powrot z YCrCb do RGB pliku podstawowego
		cvtColor(temporary, temporary, CV_YCrCb2BGR);		//zamiana z YCrCb do RGB pliku po wyrownaniu histogramu
		params->final_img.push_back(temporary);				//wpisanie obrazu wynikowego do vektora przechowywujšcego obrazy wynikowe
	}
}


int main()
{
	struct stat info;
	
	vector<Mat> start_img, final_img;						//deklaracja vektorow

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
	
	int n = FileList.size();										//zmienna przechowujaca ilosc odczytanych scierzek obrazow w folderze
	
	
	int row = ceil(sqrt((double)n));								//wyliczanie ilosci obrazow w wierszu mozaikach
	int column = ceil((double)n / row);								//wyliczanie ilosci wierszy w mozaice

	int s = 800. / column;											//wyznaczanie szerokosci pojedynczego obrazu mozaiki
	int w = 0.75*s;													//wyliczanie wysokosci pojedynczego obrazu mozaiki

	Mat mozaika(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));			//deklaracja mozaiki dla obrazow wejsciowych
	Mat mozaika2(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));		//deklaracja mozaiki dla obrazow wyjsciowych
	


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

	};									// inicjalizacja parametrow dla podzialu pracy na 2 watki
										//watek 0: co 2 obraz od obrazu 0, watek 1: co 2 obraz od obrazu 1	

	

	HANDLE threads[threadCount];

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i] = (HANDLE)_beginthread(&EqualizeImages, 0, &params[i]);	//Mozemy przekazac tylko 1 parametr do funkcji. Stad wymagane jest uzycie struktury.
	}

		WaitForMultipleObjects(threadCount, threads, TRUE, INFINITE); //oczekiwanie az watki zakoncza dzialanie

	/*
		Petla w ktorej nastepuje sklejanie obrazow w mozaiki
	*/
	for (int i = 0; i < FileList.size(); i++)
	{
		int x = i%column*s;									//wyliczanie skladowej x do wklejenia obrazu do mozaiki
		int y = i / column*w;								//wyliczanie skladowej y do wklejenia obrazu do mozaiki
		int z;
		if (i % 2 != 0)										//ustawianie kolejnosci wklejania obrazow przetworzonych do mozaiki obrazow wyjsciowych
		{
			z = i - i / 2 - 1;
		}
		else
		{
			z = FileList.size() / 2 + i / 2;
		}
		Mat roi = mozaika(Rect(x, y, s, w));				//utworzenie prostokatow, do ktorych zostana wklejone obrazy wejsciowe
		Mat roi2 = mozaika2(Rect(x, y, s, w));				//utworzenie prostokatow, do ktorych zostana wklejone obrazy przetworzone
		resize(imread(FileList[i]), roi, roi.size());		//dopasowanie obrazow wejsciowych do rozmiarow pola mozaiki
		resize(final_img[z], roi2, roi2.size());			//dopasowanie obrazow przetworzonych do rozmiarow pola mozaiki
		namedWindow("Start_images", CV_WINDOW_KEEPRATIO);	//nazywanie okna pokazujacego mozaike z obrazami wejsciowymi
		namedWindow("Final_images", CV_WINDOW_KEEPRATIO);	//nazywanie okna pokazujacego mozaike z obrazami wyjsciowymi
		imshow("Start_images", mozaika);					//wyswietlanie mozaiki plikow wejsciowych
		imshow("Final_images", mozaika2);					//wyswietlanie mozaiki plikow wyjjciowych
	}
	imwrite(write_path + "/Start_images.jpg", mozaika);		//zapisanie utworzonej mozaiki obrazow wejsciowych do zadanej scierzki
	imwrite(write_path + "/Final_images.jpg", mozaika2);	//zapisanie utworzonej mozaiki obrazow wyjsciowych do zadanej scierzki
	waitKey(0);												//kilkniecie "Enter" podczas otworzonego ktorejkowiek mozaiki powodoje zamkniecie programu
    return 0;
}

