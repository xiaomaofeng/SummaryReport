// SummaryReportDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <io.h>
#include <direct.h>
#include <algorithm>
//#include<windows.h>
using namespace std;
#include "CSVReportFileDataOperator.h"
//--------rapidjson----------
#include"rapidjson/stringbuffer.h"
#include"rapidjson/document.h"
#include"rapidjson/istreamwrapper.h"
#include"rapidjson/writer.h"
using namespace rapidjson;
//---------logger------------
#include"logger/logger.h"
using namespace LOGGER;
CLogger logger(LogLevel_Info, CLogger::GetAppPathA().append("log\\"));

//#include "SummaryReportDLL.h"

string GetCurrentTimeString()
{
	time_t timer;
	struct tm *pTm;
	timer = time(NULL);
	pTm = localtime(&timer);
	char szFileName[256] = { 0 };
	pTm->tm_mon;
	pTm->tm_mday;
	pTm->tm_hour;
	pTm->tm_min;
	pTm->tm_sec;
	sprintf(szFileName, "%d%02d%02d%02d%02d%02d", pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	string strTime = szFileName;
	return strTime;
}

template <class output_type, class input_type>
output_type myConvert(input_type input)
{
	stringstream ss;
	ss << input;
	output_type result;
	ss >> result;
	return result;
}
string FilterReportFileName(string strFilePath)
{
	string strResult = "";
	string strFileName = strFilePath.rfind('\\') != string::npos ? strFilePath.substr(strFilePath.rfind("\\") + strlen("\\")) : strFilePath;
	strResult += strFileName.substr(0, strFileName.find('_'));
	strResult += strFileName.substr(strFileName.rfind('_'), strFileName.rfind('.') - strFileName.rfind('_'));
	return strResult;
}
void CreateFileDir(string strOutputFilePath) {
	char *fileName = (char*)strOutputFilePath.c_str(), *pDir;
	int pos = 0;
	while ((pos = strOutputFilePath.find('\\', pos)) != string::npos)
	{
		string subStr = strOutputFilePath.substr(0, pos);
		if (_access(subStr.c_str(), 6) == -1)
		{
			_mkdir(subStr.c_str());
		}
		pos++;
	}

}

string FormatOutputDirectory(string strOutputDirectory)
{
	string strTime = GetCurrentTimeString();
	if (strOutputDirectory.find_last_of('\\') != (strOutputDirectory.length() - 1))
		strOutputDirectory = strOutputDirectory + "\\" + strTime + "\\";
	else
		strOutputDirectory = strOutputDirectory + strTime + "\\";
	return strOutputDirectory;
}
string FindCommonDirPath(string str1, string str2)
{
	string strResult = "";
	int pos = 0;
	while ((pos = str1.find('\\', pos)) != string::npos && str2.find("\\", pos) != string::npos)
	{
		pos++;
		string strSub1 = str1.substr(0, pos);
		transform(strSub1.begin(), strSub1.end(), strSub1.begin(), ::tolower);
		string strSub2 = str2.substr(0, pos);
		transform(strSub2.begin(), strSub2.end(), strSub2.begin(), ::tolower);
		if (strSub1 == strSub2)
		{
			strResult = strSub1;
		}
		else
			break;
	}
	return strResult;
}
int SplitTestDataImagesBasedOnSummary(vector<string> vctSummaryFilePath)
{
	for (int iFileIndex = 0; iFileIndex < vctSummaryFilePath.size(); iFileIndex++)
	{
		string strSummaryFilePath = vctSummaryFilePath.at(iFileIndex);
		CSVReportFileDataOperator* FileData_Summary = new CSVReportFileDataOperator();
		FileData_Summary->LoadFileContentData(strSummaryFilePath);
		if (FileData_Summary->GetLineCount() < 1)
			continue;
		string strTestImageFileBaseDir = "*.*";
		for (int iLineIndex = 0; iLineIndex < FileData_Summary->GetLineCount(); iLineIndex++)
		{
			string strTestImageFilePath = FileData_Summary->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage);
			if (_access(strTestImageFilePath.c_str(), 0) == -1)
			{
				continue;
			}
			if (strTestImageFileBaseDir == "*.*")
			{
				strTestImageFileBaseDir = strTestImageFilePath;
			}
			strTestImageFileBaseDir = FindCommonDirPath(strTestImageFileBaseDir, strTestImageFilePath);
		}
		for (int iLineIndex = 1; iLineIndex < FileData_Summary->GetLineCount(); iLineIndex++)
		{
			string strTestImageFilePath = FileData_Summary->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage);
			if (_access(strTestImageFilePath.c_str(), 0) == -1)
			{
				continue;
			}
			//string strTestImageFileName = strTestImageFilePath.rfind('\\') != string::npos ? strTestImageFilePath.substr(strTestImageFilePath.rfind("\\") + strlen("\\")) : strTestImageFilePath;
			string strCopyToImageFileBaseDir = strSummaryFilePath.substr(0, strSummaryFilePath.find_last_of(".")) + "\\";
			string strCopyToImageFilePath = strCopyToImageFileBaseDir + strTestImageFilePath.substr(strTestImageFileBaseDir.length());
			CreateFileDir(strCopyToImageFilePath);
			CopyFileA(strTestImageFilePath.c_str(), strCopyToImageFilePath.c_str(), false);
		}
	}
	return 0;
}

void CSVReportFileDataOperator::GenerateDiffSummary(CSVReportFileDataOperator* FileData_A, CSVReportFileDataOperator* FileData_B, CSVReportFileDataOperator* FileData_S)
{
	int iFileLineCount_A = FileData_A->GetLineCount();
	int iFileLineCount_B = FileData_B->GetLineCount();
	if (iFileLineCount_A != iFileLineCount_B || iFileLineCount_A <= 1)
	{
		return;
	}

	int iTotalFiles_S = 0;
	double dTotalTimeCost_S = 0;
	int iTotalBarcodeCount_S = 0;

	vector<string> vctLineData_temp;

	//Title for Summary File
	//S2
	vctLineData_temp.push_back("NO., Image Source, Time Cost(ms), Barcode Count");
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();

	for (int i = 1; i < iFileLineCount_A; i++)
	{
		vector<string> vctLineData = FileData_A->GetLineData(i);
		if (vctLineData.size() == 0)
			continue;
		string strField_BarcodeCount_A = FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_BarcodeCount);
		string strField_BarcodeCount_B = FileData_B->GetFieldDataFromLine(i, CSV_FieldIndex_BarcodeCount);
		//S2
		if ((strField_BarcodeCount_A != "" && strField_BarcodeCount_A != "0") &&
			(strField_BarcodeCount_B == "" || strField_BarcodeCount_B == "0"))
		{
			iTotalFiles_S++;
			vctLineData_temp.push_back(FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_Number));
			vctLineData_temp.push_back(FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_SourceImage));
			//A Time Cost
			string strTimeCost_A = FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_TimeCost);
			dTotalTimeCost_S += myConvert<double>(strTimeCost_A);
			vctLineData_temp.push_back(strTimeCost_A);
			//A Barcode Count
			iTotalBarcodeCount_S += myConvert<int>(strField_BarcodeCount_A);
			vctLineData_temp.push_back(strField_BarcodeCount_A);
			//save one line
			FileData_S->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();
		}
	}
	vctLineData_temp.push_back("");
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	//Summary info
	vctLineData_temp.push_back(",Total Image Count:," + myConvert<string>(iTotalFiles_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Barcode Count:," + myConvert<string>(iTotalBarcodeCount_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Time Cost:," + myConvert<string>(dTotalTimeCost_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Avg Time Per Image (ms):," + myConvert<string>(dTotalTimeCost_S / iTotalFiles_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Avg Time Per Barcode (ms):," + myConvert<string>(dTotalTimeCost_S / iTotalBarcodeCount_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	FileData_S->AppendLineData(vctLineData_temp);
}

int CSVReportFileDataOperator::GenerateSummary(vector<string> vctReportFiles, string strOutputDirectory)
{
	string strTime = GetCurrentTimeString();
	if (strOutputDirectory.find_last_of('\\') != (strOutputDirectory.length() - 1))
		strOutputDirectory = strOutputDirectory + "\\" + strTime + "\\";
	else
		strOutputDirectory = strOutputDirectory + strTime + "\\";
	CreateFileDir(strOutputDirectory);

	vector<CSVReportFileDataOperator*> vctReportFileData, vctSummaryFileData;
	int iReportFileCount = vctReportFiles.size();
	for (int iFileIndex = 0; iFileIndex <iReportFileCount; iFileIndex++)
	{
		vctReportFileData.push_back(new CSVReportFileDataOperator());
		vctReportFileData.at(iFileIndex)->LoadFileContentData(vctReportFiles.at(iFileIndex));
	}
	//S_A - S_B summary file count: iReportFileCount*(iReportFileCount-1)
	int iSummaryFileIndex = 0;
	vector<string> vctSummaryFilePath;

	for (int iFileIndex_A = 0; iFileIndex_A < iReportFileCount; iFileIndex_A++)
	{
		for (int iFileIndex_B = 0; iFileIndex_B < iReportFileCount; iFileIndex_B++)
		{
			if (iFileIndex_A == iFileIndex_B)
				continue;
			vctSummaryFileData.push_back(new CSVReportFileDataOperator());
			vctSummaryFilePath.push_back(strOutputDirectory + FilterReportFileName(vctReportFiles.at(iFileIndex_A)) + " - " + FilterReportFileName(vctReportFiles.at(iFileIndex_B)) + ".csv");
			vector<string> vctLineData_temp;

			//File A and File B info
			vctLineData_temp.push_back("File " + myConvert<string>(iFileIndex_A + 1) + ": " + vctReportFiles.at(iFileIndex_A));
			vctSummaryFileData.at(iSummaryFileIndex)->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();
			vctLineData_temp.push_back("File " + myConvert<string>(iFileIndex_B + 1) + ": " + vctReportFiles.at(iFileIndex_B));
			vctSummaryFileData.at(iSummaryFileIndex)->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();
			if(Summary_Diff=="true"||Summary_Diff=="TRUE")
			{
			GenerateDiffSummary(vctReportFileData.at(iFileIndex_A), vctReportFileData.at(iFileIndex_B), vctSummaryFileData.at(iSummaryFileIndex));
			}
			vctSummaryFileData.at(iSummaryFileIndex)->SaveFileContentData(vctSummaryFilePath.at(iSummaryFileIndex));
			iSummaryFileIndex++;
		}
	}

	CSVReportFileDataOperator* FileData_Summary_AllCan = new CSVReportFileDataOperator();
	CSVReportFileDataOperator* FileData_Summary_AllCannot = new CSVReportFileDataOperator();
	CSVReportFileDataOperator* FileData_Summary_AnyoneCan = new CSVReportFileDataOperator();

	string strSummaryFile_Summary_AllCan = strOutputDirectory  + "AllCan.csv";
	string strSummaryFile_Summary_AllCannot = strOutputDirectory  + "AllCannot.csv";
	string strSummaryFile_Summary_AnyoneCan = strOutputDirectory  + "AnyOneCan.csv";
	vctSummaryFilePath.push_back(strSummaryFile_Summary_AllCan);
	vctSummaryFilePath.push_back(strSummaryFile_Summary_AllCannot);
	vctSummaryFilePath.push_back(strSummaryFile_Summary_AnyoneCan);


	int iFileLineCount = vctReportFileData.at(0)->GetLineCount();
	int iFileFieldCount = vctReportFileData.at(0)->GetFieldCount();

	int iTotalFiles_Summary_AllCan = -1;
	int iTotalFiles_Summary_AllCannot = -1;
	int iTotalFiles_Summary_AnyoneCan = -1;

	double dTotalTimeCost_Summary_AllCan_A = 0;
	double dTotalTimeCost_Summary_AllCan_B = 0;

	int iTotalBarcodeCount_Summary_AllCan_A = 0;
	int iTotalBarcodeCount_Summary_AllCan_B = 0;

	vector<string> vctLineData_temp;

	//File info
	for (int iFileIndex = 0; iFileIndex < iReportFileCount; iFileIndex++)
	{
		vctLineData_temp.push_back("File " + myConvert<string>(iFileIndex + 1) + ": " + vctReportFiles.at(iFileIndex));
		FileData_Summary_AllCan->AppendLineData(vctLineData_temp);
		FileData_Summary_AllCannot->AppendLineData(vctLineData_temp);
		FileData_Summary_AnyoneCan->AppendLineData(vctLineData_temp);
		vctLineData_temp.clear();
	}

	for (int iLineIndex = CSV_LineIndex_Title; iLineIndex < iFileLineCount; iLineIndex++)
	{
		vector<string> vctLineData = vctReportFileData.at(0)->GetLineData(iLineIndex);
		if (vctLineData.size() == 0)
			continue;
		bool bSaveToSummary_AllCan = true;
		bool bSaveToSummary_AllCannot = true;
		bool bSaveToSummary_AnyoneCan = false;
		for (int iFileIndex = 0; iFileIndex < iReportFileCount; iFileIndex++)
		{
			string strField_BarcodeCount = vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_BarcodeCount);
			if (strField_BarcodeCount != "" && strField_BarcodeCount != "0")
			{
				bSaveToSummary_AllCan &= true;
				bSaveToSummary_AllCannot &= false;
				bSaveToSummary_AnyoneCan |= true;
			}
			else
			{
				bSaveToSummary_AllCan &= false;
				bSaveToSummary_AllCannot &= true;
				bSaveToSummary_AnyoneCan |= false;
			}
		}
		//Summary_AllCan
		if (Summary_AllCan == "true"||Summary_AllCan=="TRUE")
		{
			if (bSaveToSummary_AllCan || iLineIndex == CSV_LineIndex_Title)
			{
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_Number));
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage));

				iTotalFiles_Summary_AllCan++;
				for (int iFileIndex = 0; iFileIndex < iReportFileCount; iFileIndex++)
				{
					for (int iFieldIndex = 2; iFieldIndex < iFileFieldCount; iFieldIndex++)
					{
						vctLineData_temp.push_back(vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, iFieldIndex));
					}
					vctLineData_temp.push_back("");
				}
				//save one line
				FileData_Summary_AllCan->AppendLineData(vctLineData_temp);
				vctLineData_temp.clear();

			}
		}
		//Summary_AllCannot
		if (Summary_AllCannot == "true"||Summary_AllCannot=="TRUE")
		{
			if (bSaveToSummary_AllCannot || iLineIndex == CSV_LineIndex_Title)
			{
				iTotalFiles_Summary_AllCannot++;
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_Number));
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage));
				//save one line
				FileData_Summary_AllCannot->AppendLineData(vctLineData_temp);
				vctLineData_temp.clear();
			}
		}
		//Summary_AnyoneCan
		if (Summary_AnyoneCan == "true" || Summary_AnyoneCan == "TRUE")
		{
			if (bSaveToSummary_AnyoneCan || iLineIndex == CSV_LineIndex_Title)
			{
				iTotalFiles_Summary_AnyoneCan++;
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_Number));
				vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage));
				for (int iFileIndex = 0; iFileIndex < iReportFileCount; iFileIndex++)
				{
					for (int iFieldIndex = 2; iFieldIndex < iFileFieldCount; iFieldIndex++)
					{
						vctLineData_temp.push_back(vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, iFieldIndex));
					}
					vctLineData_temp.push_back("");
				}
				//save one line
				FileData_Summary_AnyoneCan->AppendLineData(vctLineData_temp);
				vctLineData_temp.clear();
			}
		}

	}
	vctLineData_temp.push_back("");
	FileData_Summary_AllCan->AppendLineData(vctLineData_temp);
	FileData_Summary_AllCannot->AppendLineData(vctLineData_temp);
	FileData_Summary_AnyoneCan->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	//Summary info
	vctLineData_temp.push_back(",Total Image Count:," + myConvert<string>(iTotalFiles_Summary_AllCan));
	FileData_Summary_AllCan->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Image Count:," + myConvert<string>(iTotalFiles_Summary_AllCannot));
	FileData_Summary_AllCannot->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Image Count:," + myConvert<string>(iTotalFiles_Summary_AnyoneCan));
	FileData_Summary_AnyoneCan->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();

	vctSummaryFileData.push_back(FileData_Summary_AllCan); //Summary for All can
	vctSummaryFileData.push_back(FileData_Summary_AllCannot); //Summary for All cannot
	vctSummaryFileData.push_back(FileData_Summary_AnyoneCan); //Summary for any one can

	FileData_Summary_AllCan->SaveFileContentData(strSummaryFile_Summary_AllCan);
	FileData_Summary_AllCannot->SaveFileContentData(strSummaryFile_Summary_AllCannot);
	FileData_Summary_AnyoneCan->SaveFileContentData(strSummaryFile_Summary_AnyoneCan);

	SplitTestDataImagesBasedOnSummary(vctSummaryFilePath);

	return 0;
}

void CSVReportFileDataOperator::GenerateDiffSummary_ByTime(CSVReportFileDataOperator* FileData_A, CSVReportFileDataOperator* FileData_B, CSVReportFileDataOperator* FileData_S, int iTimeOffset)
{
	int iFileLineCount_A = FileData_A->GetLineCount();
	int iFileLineCount_B = FileData_B->GetLineCount();
	if (iFileLineCount_A != iFileLineCount_B || iFileLineCount_A <= 1)
	{
		return;
	}

	int iTotalFiles_S = 0;
	double dTotalTimeCost_S = 0;
	int iTotalBarcodeCount_S = 0;

	vector<string> vctLineData_temp;

	//Title for Summary File
	//S2
	vctLineData_temp.push_back("NO., Image Source, Time Cost(ms), Barcode Count");
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();

	for (int i = 1; i < iFileLineCount_A; i++)
	{
		vector<string> vctLineData = FileData_A->GetLineData(i);
		if (vctLineData.size() == 0)
			continue;
		string strField_BarcodeCount_A = FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_BarcodeCount);
		string strField_BarcodeCount_B = FileData_B->GetFieldDataFromLine(i, CSV_FieldIndex_BarcodeCount);
		string strField_TimeCost_A = FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_TimeCost);
		string strField_TimeCost_B = FileData_B->GetFieldDataFromLine(i, CSV_FieldIndex_TimeCost);

		//S2
		if ((strField_BarcodeCount_A != "" && strField_BarcodeCount_A != "0" && strField_BarcodeCount_B != "" && strField_BarcodeCount_B != "0") &&
			(myConvert<int>(strField_TimeCost_A) + iTimeOffset < myConvert<int>(strField_TimeCost_B)))
		{
			iTotalFiles_S++;
			vctLineData_temp.push_back(FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_Number));
			vctLineData_temp.push_back(FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_SourceImage));
			//A Time Cost
			string strTimeCost_A = FileData_A->GetFieldDataFromLine(i, CSV_FieldIndex_TimeCost);
			dTotalTimeCost_S += myConvert<double>(strTimeCost_A);
			vctLineData_temp.push_back(strTimeCost_A);
			//A Barcode Count
			iTotalBarcodeCount_S += myConvert<int>(strField_BarcodeCount_A);
			vctLineData_temp.push_back(strField_BarcodeCount_A);
			//save one line
			FileData_S->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();
		}
	}
	vctLineData_temp.push_back("");
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	//Summary info
	vctLineData_temp.push_back(",Total Image Count:," + myConvert<string>(iTotalFiles_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Barcode Count:," + myConvert<string>(iTotalBarcodeCount_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Total Time Cost:," + myConvert<string>(dTotalTimeCost_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Avg Time Per Image (ms):," + myConvert<string>(dTotalTimeCost_S / iTotalFiles_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	vctLineData_temp.push_back(",Avg Time Per Barcode (ms):," + myConvert<string>(dTotalTimeCost_S / iTotalBarcodeCount_S));
	FileData_S->AppendLineData(vctLineData_temp);
	vctLineData_temp.clear();
	FileData_S->AppendLineData(vctLineData_temp);
}

int CSVReportFileDataOperator::GenerateSummary_DiffByTime(vector<string> vctReportFiles, string strOutputDirectory,int iTimeOffset)
{
	string strTime = GetCurrentTimeString();
	if (strOutputDirectory.find_last_of('\\') != (strOutputDirectory.length() - 1))
		strOutputDirectory = strOutputDirectory + "\\" + strTime + "\\";
	else
		strOutputDirectory = strOutputDirectory + strTime + "\\";
	CreateFileDir(strOutputDirectory);

	vector<CSVReportFileDataOperator*> vctReportFileData, vctSummaryFileData;
	int iReportFileCount = vctReportFiles.size();
	for (int iFileIndex = 0; iFileIndex <iReportFileCount; iFileIndex++)
	{
		vctReportFileData.push_back(new CSVReportFileDataOperator());
		vctReportFileData.at(iFileIndex)->LoadFileContentData(vctReportFiles.at(iFileIndex));
	}
	//S_A - S_B summary file count: iReportFileCount*(iReportFileCount-1)
	int iSummaryFileIndex = 0;
	vector<string> vctSummaryFilePath;

	for (int iFileIndex_A = 0; iFileIndex_A < iReportFileCount; iFileIndex_A++)
	{
		for (int iFileIndex_B = 0; iFileIndex_B < iReportFileCount; iFileIndex_B++)
		{
			if (iFileIndex_A == iFileIndex_B)
				continue;
			vctSummaryFileData.push_back(new CSVReportFileDataOperator());
			vctSummaryFilePath.push_back(strOutputDirectory + FilterReportFileName(vctReportFiles.at(iFileIndex_A)) + " - " + FilterReportFileName(vctReportFiles.at(iFileIndex_B)) + ".csv");
			vector<string> vctLineData_temp;

			//File A and File B info
			vctLineData_temp.push_back("File " + myConvert<string>(iFileIndex_A + 1) + ": " + vctReportFiles.at(iFileIndex_A));
			vctSummaryFileData.at(iSummaryFileIndex)->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();
			vctLineData_temp.push_back("File " + myConvert<string>(iFileIndex_B + 1) + ": " + vctReportFiles.at(iFileIndex_B));
			vctSummaryFileData.at(iSummaryFileIndex)->AppendLineData(vctLineData_temp);
			vctLineData_temp.clear();

			GenerateDiffSummary_ByTime(vctReportFileData.at(iFileIndex_A), vctReportFileData.at(iFileIndex_B), vctSummaryFileData.at(iSummaryFileIndex), iTimeOffset);
			vctSummaryFileData.at(iSummaryFileIndex)->SaveFileContentData(vctSummaryFilePath.at(iSummaryFileIndex));
			iSummaryFileIndex++;
		}
	}

	SplitTestDataImagesBasedOnSummary(vctSummaryFilePath);

	return 0;
}

typedef struct _tagBarCodeValue {
	string strTextMessage;
	string strCodeFormat;
	string strHexMessage;
}BCODE_VALUE;

void CSVReportFileDataOperator:: Init()
{
	
	ifstream fJsonFile(CLogger::GetAppPathA().append("\\") + "init.json");
	logger.TraceInfo("	Open init.json.	");

	IStreamWrapper iStreamWrap(fJsonFile);
	Document Document;
	Document.ParseStream(iStreamWrap);
	//---------------------------------------
	if (Document.HasMember("Summary_AllCan"))
	{
		Summary_AllCan = Document["Summary_AllCan"].GetString();
		logger.TraceInfo("	Import Param [Summary_AllCan]=%s.	", Summary_AllCan.c_str());
	}
	else
	{
		logger.TraceWarning("	init.json have not Param[Summary_AllCan].	");
		bInitNormal = FALSE;
		return;
	}
	//------------------------------
	if (Document.HasMember("Summary_AllCannot"))
	{
		Summary_AllCannot = Document["Summary_AllCannot"].GetString();
		logger.TraceInfo("	Import Param [Summary_AllCannot]=%s.	", Summary_AllCannot.c_str());
	}
	else
	{
		logger.TraceWarning("	init.json have not Param[Summary_AllCannot].	");
		bInitNormal = FALSE;
		return;
	}
	//--------------------------------------------------------
	if (Document.HasMember("Summary_AnyoneCan"))
	{
		Summary_AnyoneCan = Document["Summary_AnyoneCan"].GetString();
		logger.TraceInfo("	Import Param [Summary_AnyoneCan]=%s.	", Summary_AnyoneCan.c_str());
	}
	else
	{
		logger.TraceWarning("	init.json have not Param[Summary_AnyoneCan].	");
		bInitNormal = FALSE;
		return;
	}
	//-------------------------------------------------
	if (Document.HasMember("Summary_Diff"))
	{
		Summary_Diff = Document["Summary_Diff"].GetString();
		logger.TraceInfo("	Import Param [Summary_Diff]=%s.	", Summary_Diff.c_str());
	}
	else
	{
		logger.TraceWarning("	init.json have not Param[Summary_Diff].	");
		bInitNormal = FALSE;
		return;
	}
	fJsonFile.close();
}

//
//int GenerateExpectedBarcodeResult(vector<string> vctReportFiles, string strOutputDirectory)
//{
//	strOutputDirectory = FormatOutputDirectory(strOutputDirectory);
//	CreateFileDir(strOutputDirectory);
//
//	vector<CSVReportFileDataOperator*> vctReportFileData, vctSummaryFileData;
//	int iReportFileCount = vctReportFiles.size();
//	for (int iFileIndex = 0; iFileIndex <iReportFileCount; iFileIndex++)
//	{
//		vctReportFileData.push_back(new CSVReportFileDataOperator());
//		vctReportFileData.at(iFileIndex)->LoadFileContentData(vctReportFiles.at(iFileIndex));
//	}
//
//	CSVReportFileDataOperator* FileData_Summary_AnyoneCan = new CSVReportFileDataOperator();
//
//	string strSummaryFile_Summary_AnyoneCan = strOutputDirectory + "AnyOneCan.csv";
//
//	vector<string> vctLineData_temp;
//
//	for (int iLineIndex = CSV_LineIndex_Title; iLineIndex < iFileLineCount; iLineIndex++)
//	{
//		vector<string> vctLineData = vctReportFileData.at(0)->GetLineData(iLineIndex);
//		if (vctLineData.size() == 0)
//			continue;
//
//			vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_Number));
//			vctLineData_temp.push_back(vctReportFileData.at(0)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_SourceImage));
//			vector<BCODE_VALUE> vctExpectedBarcodeInfo;
//			for (int iFileIndex = 0; iFileIndex < iReportFileCount; iFileIndex++)
//			{
//				string strBarcodeCount = FileData_Summary->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_BarcodeCount);
//				if (strBarcodeCount != "" && strBarcodeCount != "0")
//				{
//					vector<BCODE_VALUE> vctSourceBarcodeInfo;
//					for (int iBarcodeCount = 1; iBarcodeCount <= myConvert<int>(strBarcodeCount); iBarcodeCount++)
//					{
//						BCODE_VALUE bcodeValue;
//						bcodeValue.strTextMessage = vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_BarcodeText + (iBarcodeCount - 1)*CSV_BarcodeValueItemsCount);
//						//bcodeValue.strCodeFormat = vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, CSV_FieldIndex_BarcodeType + (iBarcodeCount - 1)*CSV_BarcodeValueItemsCount);
//						vctSourceBarcodeInfo.push_back(bcodeValue);
//					}
//				}
//
//				if (_access(strTestImageFilePath.c_str(), 0) == -1)
//				{
//					continue;
//				}
//				if (strTestImageFileBaseDir == "*.*")
//				{
//					strTestImageFileBaseDir = strTestImageFilePath;
//				}
//				strTestImageFileBaseDir = FindCommonDirPath(strTestImageFileBaseDir, strTestImageFilePath);
//					vctBarcodeResult.push_back(vctReportFileData.at(iFileIndex)->GetFieldDataFromLine(iLineIndex, iFieldIndex));
//			}
//			//save one line
//			vctLineData_temp.push_back(GetCommonBarcodeResult(vctBarcodeResult));
//			FileData_Summary_AnyoneCan->AppendLineData(vctLineData_temp);
//			vctLineData_temp.clear();
//
//
//	}
//
//	FileData_Summary_AnyoneCan->SaveFileContentData(strSummaryFile_Summary_AnyoneCan);
//	return 0;
//}
//
//string GetCommonString(string str1, string str2)
//{
//	string strResult = "";
//	if (str1 == "" || str2 == "")
//		return strResult;
//	string strLong, strShort;
//	if (str1.length() <= str2.length())
//	{
//		strLong = str2;
//		strShort = str1;
//	}
//	else
//	{
//		strLong = str1;
//		strShort = str2;
//	}
//	int pos = 0;
//	while (pos < strLong.length())
//	{
//		for (int iStartIndex = 0; iStartIndex < strShort.length(); iStartIndex++)
//		{
//			for (int iLength = strShort.length() - iStartIndex; iLength > 0; iLength--)
//			{
//				string strSubShort = strShort.substr(iStartIndex, iLength);
//				int iposFound = 0;
//				if ((iposFound = strLong.find(strSubShort, pos)) != string::npos)
//				{
//					strResult = "*" + strSubShort;
//					pos = iposFound + iLength;
//				}
//			}
//		}
//	}
//
//	return strResult;
//}
//string GetCommonBarcodeResult(vector<vector<string>> vctBarcodeResult)
//{
//	string strResult = "";
//
//	string strCommonBarcodeText = "*.*";
//	for (int iCount = 0; iCount < vctBarcodeResult->size(); iCount++)
//	{
//		string strType = vctBarcodeResult.at(iCount).at(0);
//		string strText = vctBarcodeResult.at(iCount).at(1);
//		if (_access(strTestImageFilePath.c_str(), 0) == -1)
//		{
//			continue;
//		}
//		if (strTestImageFileBaseDir == "*.*")
//		{
//			strTestImageFileBaseDir = strTestImageFilePath;
//		}
//		strTestImageFileBaseDir = FindCommonDirPath(strTestImageFileBaseDir, strTestImageFilePath);
//	}
//
//	return strResult;
//}