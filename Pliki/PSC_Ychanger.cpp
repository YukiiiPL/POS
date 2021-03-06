// PSC_Ychanger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	vector<Mat> start_img, final_img;

	WIN32_FIND_DATA FileName;
	TCHAR scierzka[] = TEXT("C:/POSproj");
	TCHAR rozszerzenie[] = TEXT("/*.jpg\0");
	TCHAR roz_scierzka[MAX_PATH];
	vector<string> FileList;

	StringCchCopy(roz_scierzka, MAX_PATH, scierzka);
	StringCchCat(roz_scierzka, MAX_PATH, rozszerzenie);
	
	HANDLE hfind = FindFirstFile(roz_scierzka, &FileName);
		
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t *f_name = FileName.cFileName;
			wchar_t *sciezka_file_name = scierzka;
			wstring ws(f_name);
			wstring ws2(sciezka_file_name);
			string str(ws.begin(), ws.end());
			string file_name(ws2.begin(), ws2.end());
			string full_file_name = file_name + "/" + str;
			FileList.push_back(full_file_name);
			cout << full_file_name <<"\n";
		} while (FindNextFile(hfind, &FileName));
	}
	
	int n = FileList.size();
	for (int i = 0; i < FileList.size(); i++)
	{
		start_img.push_back(imread(FileList[i]));
		imshow("Start_images"+i,start_img[i]);
	}
	
	/*
	int n = 1;
	string table[3];
	table[0] = "C:/POSproj/test_image.jpg";
	//table[1] = "C:/Users/Tomek/Pictures/PSC/Start_image/sowa.jpg";
	//table[2] = "C:/Users/Tomek/Pictures/PSC/Start_image/sowa2.jpg";

	for (int i = 0; i < n; i++)
	{
		start_img.push_back(imread(table[i]));
	}*/
	
	int row = ceil(sqrt((double)n));
	int column = ceil((double)n / row);

	int s = 800. / column;
	int w = 0.75*s;

	Mat mozaika(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));
	Mat mozaika2(row*w, column*s, CV_8UC3, CV_RGB(0, 0, 0));
	
	for (int i = 0; i < start_img.size(); i++)
	{
		Mat temporary;
		vector<Mat> channels;
		cvtColor(start_img[i], start_img[i], CV_BGR2YCrCb);

		split(start_img[i], channels);
		equalizeHist(channels[0], channels[0]);

		merge(channels, temporary);

		cvtColor(start_img[i], start_img[i], CV_YCrCb2BGR);
		cvtColor(temporary, temporary, CV_YCrCb2BGR);
		final_img.push_back(temporary);
	}
	for (int i = 0; i < start_img.size(); i++)
	{
		int x = i%column*s;
		int y = i / column*w;
		Mat roi = mozaika(Rect(x, y, s, w));
		Mat roi2 = mozaika2(Rect(x, y, s, w));
		resize(start_img[i], roi, roi.size());
		resize(final_img[i], roi2, roi2.size());
		namedWindow("Start_images", CV_WINDOW_KEEPRATIO);
		namedWindow("Final_images", CV_WINDOW_KEEPRATIO);
		imshow("Start_images", mozaika);
		imshow("Final_images", mozaika2);
	}
	imwrite("C:/POSproj/Start_images.jpg", mozaika);
	imwrite("C:/POSproj/Final_images.jpg", mozaika2);
	waitKey(0);
    return 0;
}

