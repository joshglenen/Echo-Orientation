

//------------------------------------Project Codename: RobotEars-----------------------------------------------------------//
//------------------------------------Author: Josh_Glenen_2018--------------------------------------------------------------//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//---------------------------------------------------global dynamic and static variables------------------------------------//

//--do not change below-------//
#define PI 3.14159265
typedef enum { false = 0, true = 1 } bool;

bool utteranceRegistered = false;
bool leftAmpIsHigher = false;
const double speedOfSound = 343;
bool leftIsLeading = false;
double delay = 0;
//---do not change above-------//

//for this example we are assuming low noise, high snr and a noise floor of 0.1 volts in a 0-5 Vrms signal 
// thus the signal will never drop below 1 and the signal will be clipped when 5 volts
//change based on experimental data in the environment of operation
const float uThreshold = 0.5;
const float lThreshold = 0.3;

//measure the distance btn sensors in mm
const double distanceSeparatingSensors = 100;

//setup 2 ADC inputs for 2 continuous analog non-periodic signals of range 0 to 5 volts
float leftInput = 1;// setPin(A1, input);
float rightInput = 1; // setPin(A2, input);

//setup digital output for optional LED indicator; indicates active listening
float indicator = 1; // setPin(D1, output);

//setup PWM output for motor driver
float motorControl = 1; // setPin(D2, output);

//indicate the size of the buffers which determine window size but require physical memory
//for our system, 20 to 20khz are accepted thus the longest wave will be 50 ms. For a sampling rate of 
// for a sampling rate of 80 khz, there will be 4000 samples required to fit the largest non-nill signal 
//this is demanding for a system and if 1khz is set as the new lowest audible signal, only 80 samples are required which would have a faster response time
// for this example, we are going to use 4000 and require at least 8 kb of dynamic memory and provide a window of 50ms
const int memRMSWindow = 4000;
const int memRMS = 3;
float leftBuffer[4000];
float rightBuffer[4000];
float leftRMSBuffer[3];
float rightRMSBuffer[3];

//indicate sampling rate fs where fs < fclk and fs > 40khz otherwise aliasing will corrupt data
int fs = 80000;

//defined by the clock speed of MCU
int fclk = 8000000;

//------------------------------------------Functions---------------------------------------------------------------//

//this records the inputs directly as floats between 0 and 5 and stores them in buffers
void recordData()
{
	//wait sampling interval 1/fs
	//since this interval is 12.5 micro seconds, need to use custom function
	//tdelay is the amount of clock cycles per sample (100 in this example)
	int tdelay = 0;
	double tdelayPrime = 0;
	tdelayPrime = fclk / fs;
	tdelay = floor(tdelayPrime);
	
	//using a loop will create a delay but will come with an overhead
	//note that each command will take >=1 cycle provided your MCU contains only one central processor or is single threaded.
	//A proper overhead estimate will involve converting to assembly and manually counting the process delays.
	/*
			mov     eax, DWORD PTR i[rip]
			movss   xmm0, DWORD PTR rightInput[rip]
			cdqe
			movss   DWORD PTR rightBuffer[0+rax*4], xmm0
			mov     DWORD PTR [rbp-4], 0
		.L3:
			cmp     DWORD PTR [rbp-4], 22
			jg      .L2
			add     DWORD PTR [rbp-4], 1
			jmp     .L3
		.L2:
	*/
	//I estimated this to have 5 cycles of overhead with a loop cycle of 4per cycle with an exception o2 for the last cycle.
	//Caution: flk must be greater than fs. Additionally, observed fs will deviate from programmed fs. Thus this section may require optimization
	//to find the fs observed, use this eqn: fso = fclk/(overhead+loopLast + floor(loop*tdelay)) = 80808 hz which is an error of 1.01%.
	int overhead = 5;
	int loop = 4;
	int loopLast = 2;
	tdelay = tdelay - overhead - loopLast;
	tdelay = floor(tdelay/loop);
	if (tdelay < 0) tdelay = 0;
	
	
	//Data is ready to record
	for (int i = 0; i < memRMSWindow; i++)
	{
		leftInput = leftBuffer[i];
		rightInput = rightBuffer[i];
		for (int i = 0; i < tdelay;)
		{
			i++;
		}
	}
}

//takes an input float from -5 to 5 Vpeak @20-20khz with memRMSWindow samples, converts to 0 to 5 Vrms in an array of predefined size
void determineRMS(int index)
{
	float L = 0;
	float R = 0;

	for (int i = 0; i < memRMSWindow; i++)
	{
		L += leftBuffer[i] * leftBuffer[i];
		R += rightBuffer[i] * rightBuffer[i];
	}
	L = sqrt(L / memRMSWindow);
	R = sqrt(R / memRMSWindow);

	leftRMSBuffer[index] = L;
	rightRMSBuffer[index] = R;
}

//here we define an utterance as a period where the Vrms signal has surpassed uThreshold when no utterance was previously registered
//the utterance ends when it drops below lThreshold with an utterance registered
//when an utterance is registered, the robot will try to focus on the source as long as the utterance continues to register
//the robot will not react to low noises provided the thresholds are tuned for the environment of operation
void checkThresholds()
{
	if (!utteranceRegistered)
	{
		if (leftRMSBuffer[memRMS - 1] > uThreshold) utteranceRegistered = true;
	}
	else
	{
		if (leftRMSBuffer[memRMS - 1] < lThreshold) utteranceRegistered = false;
	}
}

//a subfunction of compareLR()5
bool isRisingEdge()
{
	return ((leftRMSBuffer[0] < leftRMSBuffer[1]) && (leftRMSBuffer[1] < leftRMSBuffer[2]));
}

//a subfunction of compareLR()5
bool isFallingEdge()
{
	return ((leftRMSBuffer[0] > leftRMSBuffer[1]) && (leftRMSBuffer[1] > leftRMSBuffer[2]));
}

//a subfunction of compareLR()5
bool isPeak()
{
	return ((leftRMSBuffer[0] < leftRMSBuffer[1]) && (leftRMSBuffer[1] > leftRMSBuffer[2]));
}

//a subfunction of compareLR()5
bool isTrough()
{
	return ((leftRMSBuffer[0] > leftRMSBuffer[1]) && (leftRMSBuffer[1] < leftRMSBuffer[2]));
}

//finds (i) if robot needs to turn (ii) which sensor Vrms amplitude is higher (iii) which sensor is leading (iv) what the delay btn them is!
bool compareLR()
{
	//determine the position in time of the signal using Vrms
	bool _isPeak = isPeak();
	bool _isTrough = isTrough();
	bool _isFallingEdge = isFallingEdge();
	bool _isRisingEdge = isRisingEdge();
	
	//assume signal is above the noise floor with a reasonable SNR
	//I am using a fos of 5% to prevent the motor from constantly running. More precise systems can reduce this down to an ideal 100%.
	float fos = 1.05;

	//Which amplitude is higher?
	//a simple way to determine if amplitudes are close enough not to need to change position. 
	//Alternate way is to use sig figs and a simple == check; it would be faster but require testing.
	if (((leftRMSBuffer[0] < rightRMSBuffer[0] * fos) && (leftRMSBuffer[0] > rightRMSBuffer[0] / fos)) || ((rightRMSBuffer[0] < leftRMSBuffer[0] * fos) && (rightRMSBuffer[0] > leftRMSBuffer[0] / fos)))
	{
		//amplitudes are close enough and we dont need to turn yet
		return false;
	}
	
	//which signal is leading? 
	//this is a bit more difficult as there are four cases without using large samples and derivation to determine rise or fall
	// case 1: averageVrms is falling 
	if (_isFallingEdge)
	{
		if (leftRMSBuffer[0] > rightRMSBuffer[0]) leftIsLeading = true;
		else if (leftRMSBuffer[0] < rightRMSBuffer[0]) leftIsLeading = false;
		else
		{
			delay = 0;
			return true; //no delay detected
		}
	}
	
	//case 2: averageVrms is rising 
	else if (_isRisingEdge)
	{
		if (leftRMSBuffer[0] < rightRMSBuffer[0]) leftIsLeading = true;
		else if (leftRMSBuffer[0] > rightRMSBuffer[0]) leftIsLeading = false;
		else
		{
			delay = 0;
			return true; //no delay detected
		}

	}
	
	//case 3: averageVrms is at a peak
	else if (_isPeak)
	{
		if (leftRMSBuffer[0] > rightRMSBuffer[0]) leftIsLeading = true;
		else if (leftRMSBuffer[0] < rightRMSBuffer[0]) leftIsLeading = false;
		else
		{
			delay = 0;
			return true; //no delay detected
		}

	}
	
	//case 4: averageVrms is at a trough
	else if (_isTrough)
	{
		if (leftRMSBuffer[0] < rightRMSBuffer[0]) leftIsLeading = true;
		else if (leftRMSBuffer[0] > rightRMSBuffer[0]) leftIsLeading = false;
		else
		{
			delay = 0;
			return true; //no delay detected
		}

	}
	
	//case 5: flat line
	else return false;

	//So, what is our delay?
	//our smallest delay that we can find is 1/fs
	double smallestDelayMS = 1000 / fs; //0.0125ms
	double largestDelayMS = distanceSeparatingSensors / speedOfSound; //this is about 0.29 ms which is
	//this gives us a precision of:
	int precision = floor(largestDelayMS / smallestDelayMS);
	//this is 23 which means that there are only 23 measurable of non-zero delays!
	//you can increase this precision by using a larger separation or a larger sampling frequency
	//We must assume super small non-zero delays are smallestDelayMS and super large delays are Errors!
	//we must look to the samples before Vrms calculation and wait for the slowpoke to catchup
	//then, we find the approximate precision, and determine the delay based on that precision
	//take note of which signal is leading and if the signals are rising or falling
	int k = 1;
	float buffer = 0;
	if (leftIsLeading)
	{
		if (rightBuffer[memRMSWindow - k] < leftBuffer[memRMSWindow - k])
		{
			buffer = leftBuffer[memRMSWindow - k];
			while ((rightBuffer[memRMSWindow - k] < buffer) && (k <= precision + 1))
			{
				k++;
			}
		}
		else
		{
			buffer = leftBuffer[memRMSWindow - k];
			while (rightBuffer[memRMSWindow - k] > buffer)
			{
				k++;
			}
		}
	}
	else
	{
		if (rightBuffer[memRMSWindow - k] > leftBuffer[memRMSWindow - k])
		{
			buffer = leftBuffer[memRMSWindow - k];
			while (leftBuffer[memRMSWindow - k] < buffer)
			{
				k++;
			}
		}
		else
		{
			buffer = leftBuffer[memRMSWindow - k];
			while (leftBuffer[memRMSWindow - k] > buffer)
			{
				k++;
			}
		}
	}

	k--;
	if (k > precision + floor(precision*1.1)) return false; //error found
	if (k > precision) delay = largestDelayMS; //sound is coming from 90 or -90 degree angle from (+) normal axis
	else delay = k * smallestDelayMS;
	return true;
}

// now we need to move
// we know 
// which sensor Vrms amplitude is higher 
// which sensor is leading 
// what the delay btn them is
// length of sensor axis 
/*
X = distance between microphone speakers assumeing they are
inline with the axis of rotation t = time delay c = speed of sound in
respective medium -> this is undetectable k = 1 if sound is registered
from behind and 0 if from ahead f = 1 if left amplitude is higher or 0
if right amplitude is higher theta = angle needing to clockwise-rotate
with origin normal to axis of microphones with left microphone on the left, etc.
theta = ((-1)^f)(arcsin(ct/x)+k*180)
*/
//outputs angle in degrees where (+) is clockwise-rotation from (+) normal axis
//HOWEVER, this function assumes k is zero :(
double calculateAsmuth()
{
	double asmuth = 0;
	asmuth = pow(-1, leftIsLeading);
	double ratio = speedOfSound * delay / (distanceSeparatingSensors / 1000);
	asmuth = asmuth * asin(ratio) * 180 / PI;
	return asmuth;
}


//This greatly depends on the specifics of the device in question. 
//I will leave this up to the user
//Take the angle, find the length of time you need to turn motor at set speed with a certain controller (PID or lesser)
double generatePWM(double angle)
{
	//TODO
}

//--------------------------------------------------------Interrupts------------------------------------------------------//

//None so far

//------------------------------------------------------------Main--------------------------------------------------------//
int main(int argc, char **argv)
{
	while (1)
	{
		//sample Vrms values for each sensor 
		for (int k = 0; k < memRMS; k++)
		{
			for (int i = 0; i < memRMSWindow; i++)
			{
				recordData();
			}
			//we now have our sampled window of memRMSWindow/fs seconds
			determineRMS(k);
		}

		//we need to know if a new sound was heard based on whether or not a sound is currently being observed
		checkThresholds();
		if (utteranceRegistered)
		{
			//now we need to get our variables to estimate the 2D origin of the sound
			bool turn = compareLR();
			if (turn)
			{
				double asmuth = calculateAsmuth();
				//provide the signal to rotate the device for a period depending on the asmuth
				generatePWM(asmuth);
			}
		}
	}
}

