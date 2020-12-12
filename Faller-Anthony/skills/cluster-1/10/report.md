#  RTOS TASKs - Free RTOS

Author: Anthony Faller

Date: 2020-09-15
-----

## Summary
I have created three tasks which 
	a) Counts up in binary every second using LEDs
	b) Toggles between up and down counting on a button push
	c) Reads the current directions and displays either "UP" or "DOWN" on the alphanumeric display

The biggest issue I came across was a stuttering button - rather than implementing a full-fledged debouncer, I was advised to just wait after a button press for a few milliseconds before the next input. I found a delay of about 50ms between inputs to be appropriate.

## Sketches and Photos


## Modules, Tools, Source Used Including Attribution
Much of this code is reused from Skill07 and Skill08.

## Supporting Artifacts
Demo: Video: https://drive.google.com/file/d/1c65NB3BCdGawvedqWl6jgKMG72aFL45K/view?usp=sharing

-----
