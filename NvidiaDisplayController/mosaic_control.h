#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "nvapi.h"
#include <vector>
using namespace std;

#define EXIT 99

typedef struct _GPU_DISPLAY {
	int displayCount = 0;
	vector<NV_GPU_DISPLAYIDS> displayIDs;
}GPU_DISPLAY;

enum Config
{
	rotate = 1,
	reset,
	resetAll
};

int main(int argc, char** argv);

NvAPI_Status GetGPUInfo(vector<GPU_DISPLAY>& gpuDisplay, NvU32& gpuCount);

NvAPI_Status Rotate_180(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int dispIndex);

NvAPI_Status RotateReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int displayIndex);

NvAPI_Status RotateReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex);

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name);

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name, NvU32 displayId);

void CLI_GPU_INFO(vector<GPU_DISPLAY> gpuDisplay, int gpuNum, int*& displayNum);

void CLI_ConfigOption();

void CLI_NewLine();

void CLI_Divider();

void CLI_InputInfo(const char* indexName, int& index, int indexNum);