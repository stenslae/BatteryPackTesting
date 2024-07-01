#include "LabJackM.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "LJM_StreamUtilities.h"

int main()
{
	int handle;
	int LJM = LJM_Open(LJM_dtANY,LJM_ctANY, "LJM_idANY", &handle);

	const int initScanRate = 2000;

	double ScanRate = initScanRate;

	const int ScansPerRead = initScanRate/2;

	const int NumAddresses = 1;

	const int Channel[1] = {0};

	double aData[ScansPerRead];	

	int DeviceBacklog, LJMBacklog;

	int LJM_Stream = LJM_eStreamStart(handle, ScansPerRead, NumAddresses, Channel, &ScanRate);

	int count = 0;

	for(int i = 0; i < 100; i++){
		LJM_Stream = LJM_eStreamRead(handle, aData, &DeviceBacklog, &LJMBacklog);
		if(aData[0] != -9999){
			std::cout << aData[0] << "\n";
		}else{
			std::cout << "NA \n";
		}
	};
	LJM_eStreamStop(handle);
	LJM = LJM_Close(handle);

	return 0;
}	