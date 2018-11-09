#include "stdafx.h"
#include "CSVReportFileDataOperator.h"


CSVReportFileDataOperator::CSVReportFileDataOperator()
{
	bInitNormal = true;
}


CSVReportFileDataOperator::~CSVReportFileDataOperator()
{
	m_vctFileData.clear();
}

string CSVReportFileDataOperator::GetImageSource(string strOneCell)
{
		size_t temppos = strOneCell.find_last_of('\\');
		string tempImageSource;
		tempImageSource = strOneCell.substr(0, temppos);
		size_t pos = tempImageSource.find_last_of('\\');
		ImageSource = tempImageSource.substr(pos + 1, string::npos);
		return ImageSource;
}

void CSVReportFileDataOperator::LoadFileContentData(string strFilePath)
{
	ifstream isFile(strFilePath, ios::binary);
	if (!isFile.is_open())
	{
		cerr << "can not open file: " + strFilePath << endl;
		return;
	}
	string strOneLine;
	int ilineIndex = 0;
	bneedChangeSuffix = false;
	bool bsummaryOrderIspositive = false;
	bool bstartPush = false;
	bool bstopPush = false;
	while (getline(isFile, strOneLine))
	{
		int ioneCellIndex = 0;
		ilineIndex += 1;
		if ((strOneLine.length() != 0) &&
			(strOneLine.at(strOneLine.length() - 1) == '\r' || strOneLine.at(strOneLine.length() - 1) == '\n'))
			strOneLine = strOneLine.substr(0, strOneLine.length() - 1);
		stringstream ssLine(strOneLine);
		vector<string> vctLineData;
		string strOneCell, strOnecomma;
		bool bNeedCombine = false;		
		while (getline(ssLine, strOnecomma, ','))
		{
			
			ioneCellIndex += 1;
			bool bfirstGetline=false;
			
			if (m_vctFileData.size() == 0)
			{
				bfirstGetline = true;
			}
			
			
			strOneCell += strOnecomma;		
			if (count(strOneCell.begin(), strOneCell.end(), '\"') % 2 == 1)
			{
				bNeedCombine = true;
				strOneCell += ",";
			}
			else
				bNeedCombine = false;
			if (bfirstGetline)
			{
				if (strOneCell == "NO.")
				{
					bsummaryOrderIspositive = true;
					bstartPush = true;
				}
			}
			if (bsummaryOrderIspositive)
			{
				if (ilineIndex == 2)
				{
					if (ioneCellIndex == 2)
					{
						GetImageSource(strOneCell);
						if (ImageSource == "201806_NoPDF_TIF_GIF")
						{
							bneedChangeSuffix = true;
						}
					}
				}
				if (strOneCell == "Total Image Count:")
				{
					bstopPush = true;
				}
			}
			else
			{
				if (ilineIndex == 15)
				{
					if (ioneCellIndex == 2)
					{
						GetImageSource(strOneCell);
						if (ImageSource == "201806_NoPDF_TIF_GIF")
						{
							bneedChangeSuffix = true;
						}
					}
				}
				if (strOneCell == "NO.")
				{
					bstartPush = true;
				}
		
			}
			if (bneedChangeSuffix)
			{
				size_t pos = 0;
				pos = strOneCell.find_last_of('.');
				if (pos != -1)
				{
					string suffix = strOneCell.substr(pos, string::npos);
					if (suffix == ".jpg"|| suffix == ".JPG" || suffix == ".jpeg")
					{
						strOneCell = strOneCell.substr(0, pos) + ".png";
					}
				}
			}
			
			if (bstartPush)
			{
				if (bNeedCombine == false)
				{
					if (!bstopPush)
					{
						vctLineData.push_back(strOneCell);
						strOneCell = "";
					}
				}
			}
		}
		if (vctLineData.size() != 0)
		{
			m_vctFileData.push_back(vctLineData);
		}
	}
	isFile.close();
}
void CSVReportFileDataOperator::SaveFileContentData(string strFilePath)
{
	ofstream osFile(strFilePath, ios::trunc | ios::out | ios::in);
	for (int iLine = 0; iLine < m_vctFileData.size(); iLine++)
	{
		vector<string> vctLineData = m_vctFileData.at(iLine);
		for (int iField = 0; iField < vctLineData.size(); iField++)
		{
			string strFieldData = vctLineData.at(iField);
			osFile << strFieldData << ",";
		}
		osFile << endl;
	}
	osFile.close();
}

int CSVReportFileDataOperator::GetLineCount()
{
	return m_vctFileData.size();
}
int CSVReportFileDataOperator::GetFieldCount()
{
	int iCount = 0;
	try {
		iCount = m_vctFileData.at(CSV_LineIndex_Title).size();
	}
	catch (out_of_range& e)
	{
	}
	return iCount;
}

string CSVReportFileDataOperator::GetFieldDataFromLine(int iLineIndex, int iFieldIndex)
{
	string strData = "";
	try {
		strData = m_vctFileData.at(iLineIndex).at(iFieldIndex);
	}
	catch (out_of_range& e)
	{
	}
	return strData;
}
void CSVReportFileDataOperator::SetFieldDataFromLine(int iLineIndex, int iFieldIndex, string strFieldData)
{
	try {
		m_vctFileData.at(iLineIndex).at(iFieldIndex) = strFieldData;
	}
	catch (out_of_range& e)
	{
	}
}
vector<string> CSVReportFileDataOperator::GetLineData(int iLineIndex)
{
	vector<string> vctLineData;
	try {
		vctLineData = m_vctFileData.at(iLineIndex);
	}
	catch (out_of_range& e)
	{
	}
	return vctLineData;
}
void CSVReportFileDataOperator::SetLineData(int iLineIndex, vector<string> vctLineData)
{
	try {
		m_vctFileData.at(iLineIndex) = vctLineData;
	}
	catch (out_of_range& e)
	{
	}
}
void CSVReportFileDataOperator::AppendLineData(vector<string> vctLineData)
{
	m_vctFileData.push_back(vctLineData);
}
