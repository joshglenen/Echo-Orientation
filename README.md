# Echo orientation through echo location
The objective of this project was initially to orient a robot with two microphones towards a sudden sound with a reasonable SNR. Such a sound could be as wide ranging as person's voice to the engine of a moving vehicle. The robot would use sensors to locate the start of a sound by reading the Vrms values from each microphone over a defined window with a sampling freqency greater than 40 kHz (since human hearing is in the range of 20-20khz). If the Vrms exceeded a certain standard deviation above a tuned threshold, a sound packet was registered. After the Vrms value fell below that threshold, the sound packet was determined to have ended. 

Once a sound packet is registered, the program will try to find the point of origin of the sound by determining the differences in the peak amplitude and delay between both sensors. From here, the real problem of sound localization was encountered. Without a sound distorter/modulator (the human body, mainly the ears, provide this for our brains) there is no way to determine if a sound on a 2D plane is in front or behind (with the current setup). Additionally, in a 3D plane it is difficult to determine if a sound is above or below. Therefore, there are multiple ways to solve this problem but each way requires a substantial change to the initial setup. These ways are defined below.

# 2D orientation using two microphones

## Initial approach
This way is the easiest since it does not require epirical tables or extra components. This method requires using good old fashioned trial and error. One of the methods animals use to determine if an object is above, below, in front, or behind is by simply tilting thier hear and listening from a different orientation. This can be mimiced by turning a robot on its 2D axis in the estimated direction, resampling and correcting. This resampling and correcting process is repeated until either the sound is registered as having ended, a new sound is registered, or the sensors read approximatly the same delay and peak amplitudes. This solution is not ideal since it is slow, cannot react to short sounds, and may focus in the opposite direction! 

### Finding the new orientation
 X = distance between microphone speakers assumeing they are inline with the axis of rotation
 t = time delay
 c = speed of sound in respective medium
 -> this is undetectable   k = 1 if sound is registered from behind and 0 if from ahead
 f = 1 if left amplitude is higher or 0 if right amplitude is higher
 theta = angle needing to clockwise-rotate with origin normal to axis of microphones with left microphone on the left, etc.
 
 theta = ((-1)^f)*(arcsin(c*t/x)+k*180)
 
 Therefore, we estimate that the sound is in front and k = 0; The system will continue to monitor the input and if the sound was originally behind the robot, the robot will be oriented 180 degrees from the signal origin. This difference in angle will be detected onthe next theta calculation and the robot will rotate to the true sound origin. This method calculates k using the assumption and correction method. A commented c file named old_2D_Orientation.c is included. The c file requires implementation of specific microcontroller methods.
 
### Determining k
 Another method not used is to take two initial samples, 1 at initial position and another rotated 90 degrees. With these two values, k can be calculated. This method is less efficient than the assumption method as the motor may spin longer than needed provided k was initially 0. However, this method applies sequential calculation of theta whereas the former only calculates theta at a predefined interval; therefore, this method may be faster overall. An untested c file that estimates k in this way is included with the name old_2D_Orientation_k_addition.
 
## Second approach
Since the first approach does not need to reconstruct the input signals for purposes such as harmonic distortion analysis, comparators could replace the more complex ADCs. This would yield a more responsive system and less demanding system. The milisecond timer function in Arduino language is not sufficiently precise to measure the delay between the signals for a robot with 10 cm between the microphones which is a maximum of 0.029 microseconds!

The next step will be to implement a timer method which uses the full precision of the microcontroller clock. For 10cm, the maximum resolution is 466.5 steps with 0.77 degrees per step for a 16MHz clock. An external timer would not provide additional benifits as the clock speed is the determining factor. It is also possible to increase the distance between the sensors by X% where the resolution will increase by X%. An arduino file for an Arduino Nano is included under the name 2D_Orientation_Arduino_Unfinished.ino which will be updated when the timer is setup. 

### Circuit implementation

///toto///
 
# Other Approaches
## Using two microphones and a directional filter
 This method requires the construction of a filter such as the human ear which causes reflections and distortions in the speaker. These distortions can be detected in the harmonics of the input signal. Using empirpcle data and a proper filter, the relationship between distortion and direction could be estimated. This method also applies for 3D space; however, it's well beyond the scope of this project.
 
## Determining k using three microphones
  Adding more sensors will both increase the sensitivity and easily allow k detection (depending on SNR). Since the goal of this project was to imitate human ears, this method will not be pursued. This method will be more practical and precise but will also demand a significant incease in power and part costs.
