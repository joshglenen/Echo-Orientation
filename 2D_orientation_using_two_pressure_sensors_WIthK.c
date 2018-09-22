#include<stdio.h>
#include<stdlib.h>
#include <math.h>


//Here is the extra code to determine k!
//here is my untested theory, diagram not present
//theory: if k = 1, _leftIsLeading will not switch if _leftIsLeading is true initially and robot is rotating ccw
//theoryL if k = 1, _leftIsLeading will switch if _leftIsLeading is false initially and robot is rotating ccw

int findK()
{
	int k = 0;
	bool initialLeader = leftIsLeading;

	float theta = calculateAsmuth();

	//turn 90 degrees ccw
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

bool isLeftLeading()
{

	bool _leftIsLeading = false;

	//assume signal is above the noise floor with a reasonable SNR
	//I am using a fos of 105% to prevent the motor from constantly running. More precise systems can reduce this down to an ideal 100%.
	float fos = 1.05;

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




