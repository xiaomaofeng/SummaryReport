#pragma once

#ifndef __MY_LIB__
#define __MY_LIB__
#include <string>
#include <vector>
using namespace std;
extern "C" int GenerateSummary(vector<string> strReportFiles, string strOutputDirectory);
extern "C" int GenerateSummary_DiffByTime(vector<string> strReportFiles, string strOutputDirectory, int iTimeOffset);
extern "C" int SplitTestDataImagesBasedOnSummary(vector<string> vctSummaryFilePath);
#endif