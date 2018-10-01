/* Uses ATmega328P on "Arduino Nano" development board
 * PinIO:
 * D2 left sensor input
 * D3 right sensor input
 * D4 motor direction
 * D5 pwm generator
 * Note: most vars stored globally to reserve required memory
 */
 
  double speedOfSound = 343; /*m/s*/
  double distanceSeparatingSensors = 0.01; /*m*/
  double minDelayMS = 0.0000625; /*minimum resolution of timer for 16MHz clk*/
  double maxDelayMS = 0.0437; 
  /*(f.o.s. = 1.5)*distanceSeparatingSensors/speedOfSound. 
  for 10cm, resolution is 466.5 steps with 0.77 degrees per step
  */
  int maxMotorSpeedFullLoad = 18000; /*rps*/
  int _duty = 255; /*max speed for now (0 to 255). Can adjust to be slower for small changes in position.*/
  double motorFactor = 0; 

  bool leftIsLeading = false;
  bool delayTimerIsRunningLeft = false;  
  bool delayTimerIsRunningRight = false;
  bool waitForMotorToFinish = false;
  bool waitForPeakSamples = false;
  double _delayMS = 0;
  double _angle = 0;

void setup() 
{
  motorFactor = 1000000/(maxMotorSpeedFullLoad*(_duty/255)*360);
  setTimer();
  /*pulldown resistors configured externally*/
  pinMode(2, INPUT); 
  pinMode(3, INPUT);
  pinMode(4, OUTPUT); /*ccw if high*/
  pinMode(A0, OUTPUT); /*5v,40mA maximum*/
  digitalWrite(4, LOW);
  analogWrite(A0, 0);
  attachInterrupt(digitalPinToInterrupt(2), ISR_D2_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(3), ISR_D3_rising, RISING);
}

void loop() {}

void setTimer()
{
  
}
void startTimer()
{
  
}

void stopTimer()
{
  _delayMS = 0;
}

void ISR_TIMER1_OVF_vect()
{
  /*when time exceeds timer size or no sound observed
   * if delay >> delayMax, need to reset bools and stop timer
   */
}

double calculateAsumth()
{
  double asmuth = 0;

  asmuth = pow(-1, leftIsLeading);

  double ratio = speedOfSound * (_delayMS/1000) / (distanceSeparatingSensors);

  asmuth = asmuth * asin(ratio) * 180 / PI;

  return asmuth;
}

/*Delivers to motor driver not motor itself. 
Would recommend motor position encoder or stepper motor for better precision.*/

void generatePWM()
{
  if(_angle>0) digitalWrite(4, HIGH);
  
  else digitalWrite(4, LOW);
  
  int _motorTimeUS = (int) abs(_angle)*motorFactor;;

  analogWrite(A0, _duty);
  
  delayMicroseconds(_motorTimeUS);

  analogWrite(A0, 0);
}

void reorient()
{
  waitForMotorToFinish = true;
  
  if ((_delayMS > minDelayMS)&&(_delayMS < maxDelayMS))
    {
      _angle = calculateAsumth();
      
      generatePWM();
    }
    
  waitForMotorToFinish = false;
  
}

void ISR_D2_rising()
{
  if ((waitForMotorToFinish)||(delayTimerIsRunningLeft)) return;
    
  if (delayTimerIsRunningRight)
  {
    stopTimer();
    
    delayTimerIsRunningRight = false;
    
    reorient();
  }

  else
  {
    //start timer with left leading
    leftIsLeading = true;
    
    delayTimerIsRunningLeft = true;
    
    startTimer();
  }
}


void ISR_D3_rising()
{
  if ((waitForMotorToFinish)||(delayTimerIsRunningRight)) return;
    
  if (delayTimerIsRunningLeft)
  {
    stopTimer();
    
    delayTimerIsRunningLeft = false;
    
    reorient();
  }

  else
  {
    //start timer with right leading
    leftIsLeading = false;
    
    delayTimerIsRunningRight = true;
    
    startTimer();
  }
}
