#include "control.h"

NvAPI_Status status;
NvAPI_ShortString estring;

NvAPI_Status GetGPUInfo(vector<GPU_DISPLAY>& gpuDisplay, NvU32& gpuCount)
{
	NvPhysicalGpuHandle nvGPUHandles[NVAPI_MAX_PHYSICAL_GPUS];
	NvU32 dispIdCount = 0;
	NvU32 gpu;

	int activeGPU = 0;

	// Get GPU handles
	ZeroMemory(&nvGPUHandles, sizeof(nvGPUHandles));
	status = NvAPI_EnumPhysicalGPUs(nvGPUHandles, &gpuCount);
	if (status != NVAPI_OK)
	{
		IsNvAPI_Error("NvAPI_EnumPhysicalGPUs");
		return status;
	}

	// Reserve the capacity
	gpuDisplay.reserve(gpuCount);

	// Query the active physical display connected to each gpu.
	for (gpu = 0; gpu < gpuCount; gpu++)
	{
		status = NvAPI_GPU_GetConnectedDisplayIds(nvGPUHandles[gpu], NULL, &dispIdCount, 0);
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_GPU_GetConnectedDisplayIds:dispIdCount");
			return status;
		}
		if (dispIdCount == 0) {
			//gpuDisplay.push_back({ 0 });
			continue;
		}

#ifdef SHOW_ALL_DISPLAY
		gpuDisplay.push_back({ (int)dispIdCount - 1 });
		gpuDisplay[activeGPU].displayIDs.reserve(gpuDisplay[activeGPU].displayCount);
#endif // SHOW_ALL_DISPLAY

		NV_GPU_DISPLAYIDS* dispIds = NULL;
		dispIds = new NV_GPU_DISPLAYIDS[dispIdCount];
		dispIds->version = NV_GPU_DISPLAYIDS_VER;
		status = NvAPI_GPU_GetConnectedDisplayIds(nvGPUHandles[gpu], dispIds, &dispIdCount, 0);  // dispIdCount exclude mosaic mode
		if (status != NVAPI_OK)
		{
			delete[] dispIds;
			IsNvAPI_Error("NvAPI_GPU_GetConnectedDisplayIds:dispIds");
			return status;
		}


#ifndef SHOW_ALL_DISPLAY
		// Get all display except mosaic mode
		gpuDisplay.push_back({ (int)dispIdCount });

		// Reserve the capacity
		gpuDisplay[activeGPU].displayIDs.reserve(gpuDisplay[activeGPU].displayCount);
#endif // !SHOW_ALL_DISPLAY


		// Check the display is active or not 
		for (int i = 0; i < dispIdCount; i++) {
#ifdef SHOW_ALL_DISPLAY
			gpuDisplay[activeGPU].displayIDs.push_back(dispIds[i]);
#endif // SHOW_ALL_DISPLAY

#ifndef SHOW_ALL_DISPLAY
			if (dispIds[i].isActive)
			{
				gpuDisplay[activeGPU].displayIDs.push_back(dispIds[i]);
			}
			else
			{
				gpuDisplay[activeGPU].displayCount--; // Exclude ports with DHS inserted
			}
#endif // !SHOW_ALL_DISPLAY
		}

		activeGPU++;
		delete[] dispIds;
	}

	gpuCount = activeGPU;
	return status;
}

NvAPI_Status Flip(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int dispIndex) {
	NvSBox desktopRect;
	NvSBox scanoutRect; //portion of the desktop
	//NvSBox viewportRect; //the viewport which is a subregion of the scanout
	NvSBox osRect; //os coordinates of the desktop

	NvDisplayHandle disp = NULL;
	NvAPI_ShortString displayName;

	NV_SCANOUT_WARPING_DATA warpingData;
	int maxNumVertices = 0;
	int sticky = 0;

	if (gpuDisplay[gpuIndex].displayIDs[dispIndex].isActive) {
		ZeroMemory(&desktopRect, sizeof(desktopRect));
		ZeroMemory(&scanoutRect, sizeof(scanoutRect));
		ZeroMemory(&osRect, sizeof(osRect));
		//ZeroMemory(&viewportRect, sizeof(viewportRect));
		//printf("GPU %d, displayId 0x%08x\n", gpuIndex, gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId);

		// Query the desktop and scanout portion of each physical active display.
		status = NvAPI_GPU_GetScanoutConfiguration(gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId, &desktopRect, &scanoutRect);
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_GPU_GetScanoutConfiguration");
			return status;
		}

		// The below is optional for R331+ in cases where the viewport rect!=scanoutRect
		//NV_SCANOUT_INFORMATION scanoutInformation;
		//scanoutInformation.version = NV_SCANOUT_INFORMATION_VER;
		//status = NvAPI_GPU_GetScanoutConfigurationEx(gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId, &scanoutInformation);
		////if this new interface is supported fetch scanout data from it
		//if (status == NVAPI_OK)
		//{
		//	scanoutRect.sWidth = scanoutInformation.targetDisplayWidth;
		//	scanoutRect.sHeight = scanoutInformation.targetDisplayHeight;
		//	viewportRect = scanoutInformation.targetViewportRect;
		//}

		//Need to get osRect for this we need the NvDisplayHandle to get access to the displayName to be pased to win32
		status = NvAPI_DISP_GetDisplayHandleFromDisplayId(gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId, &disp);
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_DISP_GetDisplayHandleFromDisplayId");
			return status;
		}
		status = NvAPI_GetAssociatedNvidiaDisplayName(disp, displayName);
		//IsNvAPI_Error("NvAPI_GetAssociatedNvidiaDisplayName");
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_GetAssociatedNvidiaDisplayName");
			return status;
		}

		DEVMODEA dm = { 0 };
		dm.dmSize = sizeof(DEVMODEA);
		if (!EnumDisplaySettingsA(displayName, ENUM_CURRENT_SETTINGS, &dm)) {
			//TODO handle status
		}

		osRect.sX = dm.dmPosition.x;
		osRect.sY = dm.dmPosition.y;
		osRect.sWidth = dm.dmPelsWidth;
		osRect.sHeight = dm.dmPelsHeight;

#ifdef SHOW_RESOLUTION
		CLI_NewLine();
		printf(" desktopRect: sX = %6d, sY = %6d, sWidth = %6d sHeight = %6d\n", desktopRect.sX, desktopRect.sY, desktopRect.sWidth, desktopRect.sHeight);
		//printf(" scanoutRect: sX = %6d, sY = %6d, sWidth = %6d sHeight = %6d\n", scanoutRect.sX, scanoutRect.sY, scanoutRect.sWidth, scanoutRect.sHeight);
		//printf(" viewportRect: sX = %6d, sY = %6d, sWidth = %6d sHeight = %6d\n", viewportRect.sX, viewportRect.sY, viewportRect.sWidth, viewportRect.sHeight);
		printf(" osRect:      sX = %6d, sY = %6d, sWidth = %6d sHeight = %6d\n", osRect.sX, osRect.sY, osRect.sWidth, osRect.sHeight);
#endif // SHOW_RESOLUTION



		// desktopRect should now contain the area of the desktop which is scanned
		// out to the display given by the displayId.
		// scanoutRect should now contain the area of the scanout to which the 
		// desktopRect is scanned out. That will give information about the gpu 
		// internal output scaling before applying the warping and intensity control
		// However that doesn't give information about the scaling performed on 
		// the display side.
		//
		// With this information the intensity and warping coordinates can be 
		// computed. The example here warps the desktopRect to a trapezoid similar
		// to the one seen in the diagram.	
		//
		// Triangle strip with 4 vertices
		// The vertices are given as a 2D vertex strip because the warp
		// is a 2d operation. To be able to emulate 3d perspective correction, 
		// the texture coordinate contains 4 components, which needs to be
		// adjusted to get this correction.
		//
		// A trapezoid needs 4 vertices.
		// Format is xy for the vertex + yurq for the texture coordinate.
		// So we need 24 floats for that.
		//float vertices [4*6];
		//
		//  (0)  ----------------  (2)
		//       |             / |
		//       |            /  |
		//       |           /   |
		//       |          /    |
		//       |         /     |
		//       |        /      |
		//       |       /       |
		//       |      /        | 
		//       |     /         |
		//       |    /          |
		//       |   /           |
		//       |  /            |
		//       | /             |  
		//   (1) |---------------- (3)
		//
		// Original Vertices
		// Format is xy for the vertex + yurq for the texture coordinate.
		//float vertices[] = 
		//	//     x                        y                  u                           v                   r       q
		//{
		//	dstLeft,				     dstTop,				srcLeft,				srcTop,				0.0f,		1.0f,     // 0
		//	dstLeft-dstXShift,           dstTop+dstHeight,		srcLeft,				srcTop+srcHeight,	0.0f,		1.0f,     // 1
		//	dstLeft+dstWidth+dstXShift,	 dstTop,				srcLeft+srcWidth,		srcTop,				0.0f,		1.0f,     // 2
		//	dstLeft+dstWidth,	         dstTop+dstHeight,		srcLeft+srcWidth,		srcTop+srcHeight,	0.0f,		1.0f,     // 3
		//};

		// warp vertices are defined in scanoutRect coordinates    					
		float dstWidth = scanoutRect.sWidth;
		float dstHeight = scanoutRect.sHeight;
		float dstLeft = (float)scanoutRect.sX;
		float dstTop = (float)scanoutRect.sY;

		// texture coordinates
		// warp texture coordinates are defined in desktopRect coordinates
		float srcLeft = (float)desktopRect.sX;
		float srcTop = (float)desktopRect.sY;
		float srcWidth = desktopRect.sWidth;
		float srcHeight = desktopRect.sHeight;

		// ROTATE 180 matrix
		// 0 <-> 3;	1 <-> 2
		float vertices[] =
		{
			dstLeft + dstWidth,		dstTop + dstHeight,		srcLeft,				srcTop,					0.0f,		1.0f,     // 0
			dstLeft + dstWidth,		dstTop,					srcLeft,				srcTop + srcHeight,		0.0f,		1.0f,     // 1
			dstLeft,				dstTop + dstHeight,		srcLeft + srcWidth,		srcTop,					0.0f,		1.0f,     // 2
			dstLeft,				dstTop,					srcLeft + srcWidth,		srcTop + srcHeight,		0.0f,		1.0f,     // 3
		//	x|						y|						u						v						r			q
		};

		int maxnumvert = 4;

		//printf("vertices: %6.0f, %6.0f, %6.0f, %6.0f, %6.0f, %6.0f\n", vertices[0], vertices[1], vertices[2], vertices[3], vertices[4], vertices[5]);
		//printf("vertices: %6.0f, %6.0f, %6.0f, %6.0f, %6.0f, %6.0f\n", vertices[6], vertices[7], vertices[8], vertices[9], vertices[10], vertices[11]);
		//printf("vertices: %6.0f, %6.0f, %6.0f, %6.0f, %6.0f, %6.0f\n", vertices[12], vertices[13], vertices[14], vertices[15], vertices[16], vertices[17]);
		//printf("vertices: %6.0f, %6.0f, %6.0f, %6.0f, %6.0f, %6.0f\n", vertices[18], vertices[19], vertices[20], vertices[21], vertices[22], vertices[23]);
		//printf("vertices: %6.0f, %6.0f, %6.0f, %6.0f, %6.0f, %6.0f\n",vertices[24],vertices[25],vertices[26],vertices[27],vertices[28],vertices[29]);

		// Demo Warping
		//printf("Demo Warping\n");
		warpingData.version = NV_SCANOUT_WARPING_VER;
		warpingData.numVertices = maxnumvert;
		warpingData.vertexFormat = NV_GPU_WARPING_VERTICE_FORMAT_TRIANGLESTRIP_XYUVRQ;
		warpingData.textureRect = &osRect;
		warpingData.vertices = vertices;

		// This call does the Warp
		status = NvAPI_GPU_SetScanoutWarping(gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId, &warpingData, &maxNumVertices, &sticky);
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_GPU_SetScanoutWarping");
			return status;
		}
	}
	else {
		status = NvAPI_GPU_GetScanoutConfiguration(gpuDisplay[gpuIndex].displayIDs[dispIndex].displayId, &desktopRect, &scanoutRect);
		if (status != NVAPI_OK)
		{
			IsNvAPI_Error("NvAPI_GPU_SetScanoutWarping, display is not active");
			return status;
		}
	}

	return status;
}

NvAPI_Status FlipAll(vector<GPU_DISPLAY> gpuDisplay, int gpuNum) {
	bool isFlipAll = true;
	for (int gpuIndex = 0; gpuIndex < gpuNum; gpuIndex++) {
		for (int displayIndex = 0; displayIndex < gpuDisplay[gpuIndex].displayCount / 2; displayIndex++) {
			status = Flip(gpuDisplay, gpuIndex, displayIndex);
			if (status != NVAPI_OK) {
				printf("\n*** Flip FAILED *** GPUIndex: %d, DisplayIndex: %d\n", gpuIndex, displayIndex);
				isFlipAll = false;
			}
		}
	}
	if (isFlipAll && gpuNum != 0)
		printf("\nFlip the top half of the displays Successed !\n");

	return status;
}

NvAPI_Status FlipReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex, int displayIndex)
{
	NV_SCANOUT_WARPING_DATA warpingData;
	int maxNumVertices = 0;
	int sticky = 0;

	warpingData.version = NV_SCANOUT_WARPING_VER;
	warpingData.vertexFormat = NV_GPU_WARPING_VERTICE_FORMAT_TRIANGLESTRIP_XYUVRQ;
	warpingData.vertices = NULL;
	warpingData.textureRect = NULL;
	warpingData.numVertices = 0;

	status = NvAPI_GPU_SetScanoutWarping(gpuDisplay[gpuIndex].displayIDs[displayIndex].displayId, &warpingData, &maxNumVertices, &sticky);
	if (status != NVAPI_OK)
	{
		IsNvAPI_Error("NvAPI_GPU_SetScanoutWarping");
		return status;
	}

	return status;
}

NvAPI_Status FlipReset(vector<GPU_DISPLAY> gpuDisplay, int gpuIndex)
{
	NV_SCANOUT_WARPING_DATA warpingData;
	int maxNumVertices = 0;
	int sticky = 0;

	warpingData.version = NV_SCANOUT_WARPING_VER;
	warpingData.vertexFormat = NV_GPU_WARPING_VERTICE_FORMAT_TRIANGLESTRIP_XYUVRQ;
	warpingData.vertices = NULL;
	warpingData.textureRect = NULL;
	warpingData.numVertices = 0;

	for (int displayIndex = 0; displayIndex < gpuDisplay[gpuIndex].displayCount; displayIndex++) {
		if (gpuDisplay[gpuIndex].displayIDs[displayIndex].isActive) {
			status = NvAPI_GPU_SetScanoutWarping(gpuDisplay[gpuIndex].displayIDs[displayIndex].displayId, &warpingData, &maxNumVertices, &sticky);
			if (status != NVAPI_OK)
			{
				IsNvAPI_Error("NvAPI_GPU_SetScanoutWarping", gpuDisplay[gpuIndex].displayIDs[displayIndex].displayId);
				return status;
			}
		}
	}

	return status;
}

NvAPI_Status FlipResetAll(vector<GPU_DISPLAY> gpuDisplay, int gpuNum)
{
	bool isFlipAll = true;
	for (int gpuIndex = 0; gpuIndex < gpuNum; gpuIndex++) {
		for (int displayIndex = 0; displayIndex < gpuDisplay[gpuIndex].displayCount / 2; displayIndex++) {
			status = FlipReset(gpuDisplay, gpuIndex, displayIndex);
			if (status != NVAPI_OK) {
				printf("\n*** Flip Reset FAILED *** GPUIndex: %d, DisplayIndex: %d\n", gpuIndex, displayIndex);
				isFlipAll = false;
			}
		}
	}
	if (isFlipAll && gpuNum != 0)
		printf("\nFlip Reset Successed !\n");

	return status;
}

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name)
{
	NvAPI_GetErrorMessage(status, estring);
	CLI_NewLine();
	printf("*** %s Failed: %s ***\n", NvAPI_Name, estring);
	return status;
}

NvAPI_Status IsNvAPI_Error(const char* NvAPI_Name, NvU32 displayId)
{
	NvAPI_GetErrorMessage(status, estring);
	CLI_NewLine();
	printf("*** %s Failed: %s - 0x%08x ***\n", NvAPI_Name, estring, displayId);
	return status;
}

void CLI_GPU_INFO(vector<GPU_DISPLAY> gpuDisplay, int &gpuNum, int*& displayNum) {

#ifndef DEBUG
	for (int i = 0; i < gpuNum; i++)
	{
		displayNum[i] = gpuDisplay[i].displayCount;
	}
#endif // !DEBUG

#ifdef DEBUG
	NV_GPU_DISPLAYIDS *displayIDs = NULL;
	gpuNum = 3;
	for (int gpu = 0; gpu < gpuNum; gpu++)
	{
		displayNum[gpu] = 2;
		vector<NV_GPU_DISPLAYIDS> vector_displayIDs;
		displayIDs = new NV_GPU_DISPLAYIDS[displayNum[gpu]];

		for (int display = 0; display < displayNum[gpu]; display++) {
			vector_displayIDs.push_back(displayIDs[display]);
		}
		gpuDisplay.push_back({ displayNum[gpu], vector_displayIDs });
		vector_displayIDs.clear();
		vector_displayIDs.shrink_to_fit();

		delete[] displayIDs;
	}
#endif // DEBUG

	int formatLen = 12;
	const char* rowName[] = { "GPUIndex", "DisplayIndex", "DisplayID" };

	printf("---          GPU & DISPLAY INFO LIST          ---\n");
	CLI_NewLine();
	printf("%*s     %*s  %*s\n", formatLen, rowName[0], formatLen, rowName[1], formatLen, rowName[2]);
	CLI_NewLine();
	for (int gpu = 0; gpu < gpuNum; gpu++)
	{
		for (int display = 0; display < displayNum[gpu]; display++)
		{
			// #:0x,	0: padding with 0,	8: bit num
			//printf("%13d  %13d      %#08x", gpu, display, gpuDisplay[gpu].displayIDs[display].displayId);
			printf("%*d     %*d     %#08x",
				formatLen, gpu,
				formatLen, display,
				gpuDisplay[gpu].displayIDs[display].displayId);
			if (!gpuDisplay[gpu].displayIDs[display].isPhysicallyConnected)
				printf("(DHS)\n");
			else
				printf("\n");
			printf(" -------------------------------------------------\n");
		}
	}
	CLI_NewLine();
}

void CLI_ConfigOption() {
	CLI_NewLine();
	CLI_Divider();
	printf("===                Config Menu                ===\n\n");
	printf(" %d. Flip the top half of the displays\n(Plug the DP cable into the correct port)\n\n", Config::flipAll);
	printf(" %d. Superuser\n", Config::superUser);
	printf("\n - Enter the number to select the config\n");
}

void CLI_AdvancedOption() {
	CLI_NewLine();
	CLI_Divider();
	printf("===                Config Menu                ===\n\n");
	printf(" %d. Flip the top half of the displays\n(Plug the DP cable into the correct port)\n\n", Config::flipAll);
	printf(" %d. Single Flip\n", Config::flip);
	printf(" %d. Single Reset\n", Config::reset);
	printf(" %d. Reset by GPU index\n", Config::resetByGPU);
	printf(" %d. Reset All\n", Config::resetAll);
	printf("\n - Enter the number to select the config\n");
	printf(" - Exit code: %d\n", EXIT);
}

void CLI_NewLine() {
	printf("\n");
}

void CLI_Divider() {
	const char* divider = "-------------------------------------------------\n";
	printf("%s", divider);
}

void CLI_InputInfo(const char* indexName, int& index, int indexNum)
{
	printf(" - Enter the %s index(integer only)\n", indexName);
	printf(" - Exit code: %d\n", EXIT);
	printf(">");
	getchar();
	// check if is integer & index range
	while (scanf_s("%d", &index) != 1 || index < 0 || (index > indexNum - 1 && index != EXIT)) {
		CLI_NewLine();
		printf("*** INVALID %s index ***\n", indexName);
		printf(" - Enter the valid %s index(integer only)\n", indexName);
		printf(" - Exit code: %d\n", EXIT);
		printf(">");
		getchar();
		if (index == EXIT)
			break;
	}
}