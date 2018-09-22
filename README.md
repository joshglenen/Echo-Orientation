# Echo orientation through echo location
The objective of this project was initially to orient a robot with two microphones towards a sudden sound with a reasonable SNR. Such a sound could be as wide ranging as person's voice to the engine of a moving vehicle. The robot would use sensors to locate the start of a sound by reading the Vrms values from each microphone over a defined window with a sampling freqency greater than 40 kHz (since human hearing is in the range of 20-20khz). If the Vrms exceeded a certain standard deviation above a tuned threshold, a sound packet was registered. After the Vrms value fell below that threshold, the sound packet was determined to have ended. 

Once a sound packet is registered, the program will try to find the point of origin of the sound by determining the differences in the peak amplitude and delay between both sensors. From here, the real problem of sound localization was encountered. Without a sound distorter/modulator (the human body, mainly the ears, provide this for our brains) there is no way to determine if a sound on a 2D plane is in front or behind (with the current setup). Additionally, in a 3D plane it is difficult to determine if a sound is above or below. Therefore, there are multiple ways to solve this problem but each way requires a substantial change to the initial setup. These ways are defined below.

# 2D orientation using two pressure sensors (microphones)
This way is the easiest since it does not require epirical tables or extra components. This method requires using good old fashioned trial and error. One of the methods animals use to determine if an object is above, below, in front, or behind is by simply tilting thier hear and listening from a different orientation. This can be mimiced by turning a robot on its 2D axis in the estimated direction, resampling and correcting. This resampling and correcting process is repeated until either the sound is registered as having ended, a new sound is registered, or the sensors read approximatly the same delay and peak amplitudes. This solution is not ideal since it is slow, cannot react to short sounds, and may focus in the opposite direction! A c file is included above under the same name as the section title which requires setup from a user as commented within.
 
 The math: X = distance between microphone speakers assumeing they are inline with the axis of rotation
 t = time delay
 c = speed of sound in respective medium
 -> this is undetectable   k = 1 if sound is registered from behind and 0 if from ahead
 f = 1 if left amplitude is higher or 0 if right amplitude is higher
 theta = angle needing to clockwise-rotate with origin normal to axis of microphones with left microphone on the left, etc.
 
 theta = ((-1)^f)*(arcsin(c*t/x)+k*180)
 
 Therefore, we estimate that the sound is in front and k = 0; The angle is measured and the robot rotates. Theta is recalculated, if theta is close to 0, the algorythm ends otherwise the robot rotates to that second angle and repeat. The c file requires implementation of your respective microcontroller methods which are indicated within the file. A diagram of the required circuit is illustrated below

//todo//
 
 Another method not used is to take two initial samples, 1 at initial position and another rotated 90 degrees. With these two values, k can be calculated. The robot however will spin more than it needs to at every registered noise which will be distracting. An optional c file is included with the extra methods to apply this method.
 
 # 2D orientation using two pressure sensors (microphones) and a directional filter
 This method requires the construction of a filter such as the human ear which causes reflections and distortions in the speaker. Theses distortions can be detected in the harmonics of the input signal. Using empiracle data and a proper filter, the relationship between distortion and direction could be estimated. This method also applies for 3D space but requires much more data and analysis.
 
  # 2D orientation using three pressure sensors (microphones)
  This method is cheating but is the most practical. Adding more sensors will increase the sensitivity and allow detection of back and front easily. The math changes based on the number of inputs which should be spatially separated equidistantly for the highest sensitivity. This example uses three sensors, though more could be added. No code is currently included for this method.
  
  The math:
  
  //todo//
  
  # 3D orientation using five pressure sensors (microphones)
  This method uses a shperical model with 3 sesnors on the horizontal axis and a sensor on either pole. This is the minimal configuration of sensors for a practical definition. Adding more sensors increases sensitivity. This is an expensive solution for high quality components which is indicates how important directional filters such as the human body are. A proper filter reduces the amount of sensors by at least 60%! This model uses three axis and two rotational angles. The circuit and code are not provided for this method.
 

