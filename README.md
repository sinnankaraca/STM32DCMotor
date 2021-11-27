# Description
The code designed to control encoder integrated DC motor. <br />
Run with makefile <br />
L293D was used for DC motor driver, STM32F411 was used for controlling. <br />
As a motor acceleration model, sinosidal formula was used. <br />

# Usage
### Mode : 0 - 1  <br />
0, run motor without encoder data, run motor for specific period of time. <br />
1, run motor with encoder data, run motor for specific rotating degree. <br />

### Time :   <br />
Time in seconds. Motor will run in this period of time. <br />

### Speed : 0 - 1000   <br />
Percentage of PWM output. <br />

### Direction : 0 - 1   <br />
Direction of motor turn. CCW or CW <br />

# Example Usage
pwmGlow 0 50 200 1<br />
Run motor in 0 mode<br />
Run motor for 50 seconds<br />
Run motor with 20 percent power<br />
Run motor in the CW direction<br />

pwmGlow 1 7200 <br />
Run motor in 1 mode (Encoder activated mode)<br />
Run motor for 7200 degree. (Turn motor for 20 turns and stop)<br />



