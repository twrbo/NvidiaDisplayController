#include "control.h"

//--------------- Marco Define ---------------

#define DEBUG
//#define SHOW_ALL_DISPLAY
#define SHOW_RESOLUTION

//---------------------------------------------

//-------------- Global Variable --------------

const char* VERSION = "1.0.3";
const unsigned int PASSWORD = 1111;
extern NvAPI_Status status;
extern NvAPI_ShortString estring;
//---------------------------------------------

int main(int argc, char** argv)
{
	// Initialize NVAPI
	status = NvAPI_Initialize();
	if (status != NVAPI_OK)
	{
		IsNvAPI_Error("NvAPI_Initialize");
#ifndef DEBUG
		system("pause");
		return status;
#endif // DEBUG
	}

	// Get all display linked in GPU
	vector<GPU_DISPLAY> gpuDisplay;
	NvU32 gpuCount = 0;
	status = GetGPUInfo(gpuDisplay, gpuCount);
	if (status != NVAPI_OK)
	{
		IsNvAPI_Error("GetGPUInfo");
#ifndef DEBUG
		system("pause");
		return status;
#endif // DEBUG
	}

	// Launch INFO
	printf(" NvAPIs initialized successfully !\n");
	CLI_NewLine();
	printf(" Nvidia display controller is launched\n");
	CLI_NewLine();
	printf(" - Version: %s\n", VERSION);
	printf(" - Company: AUO Corporation\n");
	printf(" - Author : Tony Kuo\n");

	// Set CLI
	int gpuNum = gpuCount;
	int* displayNum = new int[gpuNum];
	int gpuIndex = 0, displayIndex = 0;
	Config configOption = Config::flip;
	bool successed = true;
	unsigned int password = 0;

	// Start program
	while (true)
	{
		// OUTPUT menu option
		CLI_ConfigOption();

		// INPUT config option
		printf(">");
		while (scanf_s("%d", &configOption) != 1) {
			CLI_NewLine();
			getchar();
			printf("*** INVALID config option ***\n");
			printf(" - Enter the valid number(integer only)\n");
			printf(">");
		}

		CLI_NewLine();

		switch (configOption)
		{
		case Config::flipAll:
			status = FlipAll(gpuDisplay, gpuCount);
			break;
		case Config::superUser:
			password = 0;
			// Check password
			while (password != PASSWORD)
			{
				printf(" - Enter the password\n");
				printf(" - Exit code: %d\n", EXIT);
				printf(">");
				scanf_s("%d", &password);
				if (password == EXIT)
					break;

				getchar();
				if (password != PASSWORD)
					printf("*** INVALID password ***\n");
				CLI_NewLine();
			}

			if (password != PASSWORD)
				break;

			while (true) {
				// OUTPUT menu option
				CLI_AdvancedOption();

				// INPUT config option
				printf(">");
				while (scanf_s("%d", &configOption) != 1) {
					CLI_NewLine();
					getchar();
					printf("*** INVALID config option ***\n");
					printf(" - Enter the valid number(integer only)\n");
					printf(">");
				}

				CLI_NewLine();

				switch (configOption)
				{
				case Config::flipAll:
					status = FlipAll(gpuDisplay, gpuCount);
					break;
				case Config::flip:
					// Entered INFO
					CLI_Divider();
					printf("===          Config %d - Single Flip         ===\n\n", Config::flip);
					CLI_GPU_INFO(gpuDisplay, gpuNum, displayNum);

					// GPU INPUT INFO and check index is valid or not
					CLI_InputInfo("GPU", gpuIndex, gpuNum);
					if (gpuIndex == EXIT)
						break;

					// Display INPUT INFO and check index is valid or not
					CLI_NewLine();
					CLI_InputInfo("DISPLAY", displayIndex, displayNum[gpuIndex]);
					if (displayIndex == EXIT)
						break;

					// Flip
					status = Flip(gpuDisplay, gpuIndex, displayIndex);
					if (status != NVAPI_OK)
						printf("\n*** Single Flip FAILED ***\n");
					else
						printf("\n Single Flip Succeeded !\n");

					break;
				case Config::reset:
					// Entered INFO
					CLI_Divider();
					printf("===          Config %d - Single Reset          ===\n\n", Config::reset);
					CLI_GPU_INFO(gpuDisplay, gpuNum, displayNum);

					// GPU INPUT INFO and check index is valid or not
					CLI_InputInfo("GPU", gpuIndex, gpuNum);
					if (gpuIndex == EXIT)
						break;

					// Display INPUT INFO and check index is valid or not
					CLI_NewLine();
					CLI_InputInfo("DISPLAY", displayIndex, displayNum[gpuIndex]);
					if (displayIndex == EXIT)
						break;

					// Reset
					status = FlipReset(gpuDisplay, gpuIndex, displayIndex);
					if (status != NVAPI_OK)
						printf("\n*** Single Reset FAILED ***\n");
					else
						printf("\n Single Reset Succeeded !\n");

					break;
				case Config::resetByGPU:
					// Entered INFO
					CLI_Divider();
					printf("===       Config %d - reset by GPU index       ===\n\n", Config::resetByGPU);
					CLI_GPU_INFO(gpuDisplay, gpuNum, displayNum);

					// GPU INPUT INFO and check index is valid or not
					CLI_InputInfo("GPU", gpuIndex, gpuNum);
					if (gpuIndex == EXIT)
						break;

					// Reset
					status = FlipReset(gpuDisplay, gpuIndex);
					if (status != NVAPI_OK)
						printf("\n*** All Reset FAILED ***\n");
					else
						printf("\n All Reset Succeeded !\n");

					break;
				case Config::resetAll:
					status = FlipResetAll(gpuDisplay, gpuCount);
					break;
				case EXIT:
					break;
				default:
					printf("*** INVALID config option ***\n");
					printf(" - Enter the valid number(integer only)\n");
					break;
				}

				if (configOption == EXIT)
					break;
			}
		case EXIT:
			break;
		default:
			printf("*** INVALID config option ***\n");
			printf(" - Enter the valid number(integer only)\n");
			break;
		}
	}
}
