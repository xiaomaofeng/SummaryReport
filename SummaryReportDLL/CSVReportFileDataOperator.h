#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<vector>

using namespace std;
#define CSV_FieldIndex_Number  0
#define CSV_FieldIndex_SourceImage  1
#define CSV_FieldIndex_TimeCost  2
#define CSV_FieldIndex_BarcodeCount  3

#define CSV_BarcodeValueItemsCount  3
#define CSV_FieldIndex_BarcodeType  4
#define CSV_FieldIndex_BarcodeHex  5
#define CSV_FieldIndex_BarcodeText  6

#define CSV_LineIndex_Title  0

class CSVReportFileDataOperator
{
public:
	CSVReportFileDataOperator();
	~CSVReportFileDataOperator();
private:
	vector<vector<string>> m_vctFileData;
public:
	string Summary_AllCan;
	string Summary_AllCannot;
	string Summary_AnyoneCan;
	string Summary_Diff;
	string ImageSource;
	bool bInitNormal;
	bool bneedChangeSuffix;
	void Init();
	string GetImageSource(string strOneCell);
	int GenerateSummary(vector<string> vctReportFiles, string strOutputDirectory);
	void GenerateDiffSummary(CSVReportFileDataOperator* FileData_A, CSVReportFileDataOperator* FileData_B, CSVReportFileDataOperator* FileData_S);
	void GenerateDiffSummary_ByTime(CSVReportFileDataOperator* FileData_A, CSVReportFileDataOperator* FileData_B, CSVReportFileDataOperator* FileData_S, int iTimeOffset);
	int GenerateSummary_DiffByTime(vector<string> vctReportFiles, string strOutputDirectory, int iTimeOffset);
	void LoadFileContentData(string strFilePath);
	void SaveFileContentData(string strFilePath);
	int GetLineCount();
	int GetFieldCount();
	string GetFieldDataFromLine(int iLineIndex, int iFieldIndex);
	void SetFieldDataFromLine(int iLineIndex, int iFieldIndex,string strFieldData);
	vector<string> GetLineData(int iLineIndex);
	void SetLineData(int iLineIndex, vector<string> vctLineData);
	void AppendLineData(vector<string> vctLineData);
private:	
};

