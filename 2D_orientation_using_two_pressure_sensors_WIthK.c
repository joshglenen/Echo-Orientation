#include<stdio.h>
#include<stdlib.h>
#include <math.h>


//Here is the extra code to determine k!
//here is my untested theory, diagram not present
//theory: if k = 1, _leftIsLeading will not switch if _leftIsLeading is true initially and robot is rotating ccw
//theoryL if k = 1, _leftIsLeading will switch if _leftIsLeading is false initially and robot is rotating ccw

bool isLeftLeading()
{

	bool _leftIsLeading = false;

	//which signal is leading?
	//this is a bit more difficult as there are four cases without using large samples and derivation to determine rise or fall
	// case 1: averageVrms is falling
	if((leftRMSBuffer[0]>leftRMSBuffer[1])&&(leftRMSBuffer[1]>leftRMSBuffer[2]))
	{
		if(leftRMSBuffer[0]>rightRMSBuffer[0]) _leftIsLeading = true;
		else if(leftRMSBuffer[0]<rightRMSBuffer[0]) _leftIsLeading = false;
	}
	//case 2: averageVrms is rising
	else if((leftRMSBuffer[0]<leftRMSBuffer[1])&&(leftRMSBuffer[1]<leftRMSBuffer[2]))
	{
		if(leftRMSBuffer[0]<rightRMSBuffer[0]) _leftIsLeading = true;
		else if(leftRMSBuffer[0]>rightRMSBuffer[0]) _leftIsLeading = false;

	}
	//case 3: averageVrms is at a peak
	else if((leftRMSBuffer[0]<leftRMSBuffer[1])&&(leftRMSBuffer[2]<leftRMSBuffer[1]))
	{
		if(leftRMSBuffer[0]>rightRMSBuffer[0]) _leftIsLeading = true;
		else if(leftRMSBuffer[0]<rightRMSBuffer[0]) _leftIsLeading = false;
	}
	//case 4: averageVrms is at a trough
	else
	{
		if(leftRMSBuffer[0]<rightRMSBuffer[0]) _leftIsLeading = true;
		else if(leftRMSBuffer[0]>rightRMSBuffer[0]) _leftIsLeading = false;

	}
	return _leftIsLeading;
}

int findK()
{
	int k = 0;
	bool initialLeader = leftIsLeading;
	
	//turn 90 degrees ccw and resample
	generatePWM(-90);
	for(int k = 0; k < memRMS; k++)
		{
			for(int i = 0; i< memRMSWindow; i++)
			{
				recordData();
			}
			//we now have our sampled window of memRMSWindow/fs seconds
			determineRMS(k);
		}
	
	//get new leader position and apply theory
	bool newLeader = isLeftLeading();
	if(initialLeader)
	{
		if(initialLeader==newLeader) k = 1;
	}
	else
	{
		if(initialLeader==!newLeader) k = 1;
	}
	return k;
}

double calculateAsmuthWithK()
{
	int k = findK();
	double asmuth = 0;
	asmuth = pow(-1,leftAmpIsHigher);
	double ratio = speedOfSound * delay / (distanceSeparatingSensors / 1000);
	asmuth = (asmuth*asin(ratio)+k*PI)*180/PI;
	return asmuth;
}




