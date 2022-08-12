#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "nvapi.h"
#include <vector>

#ifndef CONTROL_H
#define CONTROL_H

using namespace std;

#define EXIT 99

typedef struct _GPU_DISPLAY {
	int displayCount = 0;
	vector<NV_GPU_DISPLAYIDS> displayIDs;
}GPU_DISPLAY;

enum Config
{
	flipAll = 1, 
	flip,
	reset,
	resetByGPU,
	resetAll,
	superUser
};

NvAPI_Status GetGPUInfo(vector<GPU_DISPLAY>& gpuDisplay, NvU32& gpuCount);

NvAPI_Status Flip(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int dispIndex);

NvAPI_Status FlipAll(vector<GPU_DISPLAY> gpuDisplay, int gpuNum);

NvAPI_Status FlipReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int displayIndex);

NvAPI_Status FlipReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex);

NvAPI_Status FlipResetAll(vector<GPU_DISPLAY> gpuDisplay, int gpuNum);

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name);

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name, NvU32 displayId);

void CLI_GPU_INFO(vector<GPU_DISPLAY> gpuDisplay, int &gpuNum, int*& displayNum);

void CLI_ConfigOption();

void CLI_AdvancedOption();

void CLI_NewLine();

void CLI_Divider();

void CLI_InputInfo(const char* indexName, int& index, int indexNum);
#endif // CONTROL_H
