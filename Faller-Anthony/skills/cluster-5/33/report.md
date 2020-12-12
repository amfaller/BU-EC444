#  PID

Author: Anthony Faller

Date: 2020-11-22
-----

## Summary
Using a simple feedback loop, I have created code that powers a certain color LED based on distance. A setpoint of 30cm was used since it is about the length of a standard ruler. A red LED is triggered if the sensor is <30cm from a target, a green LED is triggered if the sensor is exactly 30cm from a target, and a blue LED is triggered if the sensor is >30cm away from a target. Due to the sensitivity of the ultrasonic sensor, it is difficult to trigger the green LED; however, it is believed to be coded correctly.

## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
[Sensor Code](https://github.com/petemadsen/esp32/blob/master/hc-sr04/main/hcsr04.c)

## Supporting Artifacts
[Skill Page](http://whizzer.bu.edu/skills/pid)

[Demo video](https://drive.google.com/file/d/1yDt0eftHMszMNjfNd75_tQm_kjZo3w6z/view?usp=sharing)

-----
