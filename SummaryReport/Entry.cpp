#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <conio.h>
using namespace std;


int GenerateSummary(vector<string> vctReportFiles, string strOutputDirectory);
int GenerateSummary_DiffByTime(vector<string> strReportFiles, string strOutputDirectory, int iTimeOffset);
int GenerateExpectedBarcodeResult(vector<string> vctReportFiles, string strOutputDirectory);
void ShowConsoleCmd()
{
	std::cout << "*   I-Input csv report file (call multi times if multi files) " << std::endl;
	std::cout << "*   O-Output directory for summary result (Default is './')  " << std::endl;
	std::cout << "*   R-Run the barcode reader                         " << std::endl;
	std::cout << "*   Q-Quit!                          " << std::endl;

}

int main(int argc, char* argv[])
{
	string strOutputDirectory = ".\\";
	vector<string> vctReportFiles;
	int iTimeOffset = 0;
	bool bByTime = false;

	if (argc >= 3 && argc % 2 == 1)
	{
		for (int i = 1; i < argc - 1; i++)
		{
			string str = argv[i];
			if (str == "-i")
			{
				string strReportFile = argv[i + 1];
				vctReportFiles.push_back(strReportFile);
			}
			if (str == "-o")
			{
				strOutputDirectory = argv[i + 1];
			}			
			if (str == "-t")
			{
				iTimeOffset = atoi(argv[i + 1]);
				bByTime = true;
			}
		}
		if (bByTime)
			GenerateSummary_DiffByTime(vctReportFiles, strOutputDirectory, iTimeOffset);
		else
			GenerateSummary(vctReportFiles, strOutputDirectory);
			//GenerateExpectedBarcodeResult(vctReportFiles, strOutputDirectory);
		std::cout << "Complete!" << std::endl;
		return 0;
	}

	ShowConsoleCmd();
	char szBuffer[256] = { 0 };
	string strGettingMessage = "";
	char ichar = ' ', iType = ' ';
	while (ichar != 'q')
	{
		ichar = _getche();//need press the enter. getchar():need press the enter
		if (ichar == '\0')
			ichar = _getche();
		switch (ichar)
		{
		case 'i':
		case 'I':
			std::cout << "\n Please input the report file path:";
			memset(szBuffer, 0, sizeof(szBuffer));
			strGettingMessage = gets_s(szBuffer, 256);
			vctReportFiles.push_back(strGettingMessage);
			break;

		case 'o':
		case 'O':
		{
			std::cout << "\n Please input the output directory for summary result:";
			memset(szBuffer, 0, sizeof(szBuffer));
			strGettingMessage = gets_s(szBuffer, 256);
			strOutputDirectory = strGettingMessage;
		}
		break;

		case 't':
		case 'T':
			std::cout << "\n Please input the time offset:";
			memset(szBuffer, 0, sizeof(szBuffer));
			strGettingMessage = gets_s(szBuffer, 256);
			iTimeOffset = atoi(strGettingMessage.c_str());
			bByTime = true;
			break;
		case 'r':
		case 'R':
		{
			std::cout << "\n";
			if (bByTime)
				GenerateSummary_DiffByTime(vctReportFiles, strOutputDirectory, iTimeOffset);
			else
				GenerateSummary(vctReportFiles, strOutputDirectory);
			std::cout << "Complete!" << std::endl;
			//_getche();
			//memset(szBuffer, 0, sizeof(szBuffer));
		}
		break;
		case 'q':
		case 'Q':
			ichar = 'q';
			break;
		case ' ':
			break;
		default:
		{
			std::cout << "\n The input commands invalid!" << std::endl;
		}

		}
		ShowConsoleCmd();
	}

}