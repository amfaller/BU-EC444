/*	EC444 Quest04 Skill27 
*	State Models
*	October 30, 2020
*	Author: Tony Faller		*/

/* Based on http://whizzer.bu.edu/briefs/design-patterns/dp-state-machine */

/* Diagram and state tabled provided in report.md */


typedef enum {				// Set of states
	STATE_CLICK_TO_PLAY,
	STATE_MOLE_0,
	STATE_MOLE_1,
	STATE_MOLE_2,
	STATE_MOLE_3,
	STATE_MOLE_4,
	STATE_GAME_OVER
} state_e;

typedef enum{				// Set of events
	EVENT_MOLE_CLICK,
	EVENT_MISCLICK,
	EVENT_POPUP_TIMER,
	EVENT_GAME_TIMER,
	EVENT_RESET_TIMER
} event_e;

state_e currState = STATE_CLICK_TO_PLAY;
state_e nextState;
event_e event;

while(1){

	// Read event
	event = read_event();

	// Modify next state based on event
	switch(currState){
		case STATE_CLICK_TO_PLAY:
			if(event == EVENT_MOLE_CLICK || EVENT_MISCLICK) {nextState = STATE_MOLE_0;}
			break;
		case STATE_MOLE_0:
			if(event == EVENT_MISCLICK)	{nextState = STATE_MOLE_0;}
			else if(event == EVENT_POPUP_TIMER)	{nextState = STATE_MOLE_1;}
			else if(event == EVENT_GAME_TIMER)	{nextState = STATE_GAME_OVER;}
			break;
		case STATE_MOLE_1:
			if(event == EVENT_MOLE_CLICK) {nextState = STATE_MOLE_0;}
			else if(event == EVENT_MISCLICK)	{nextState = STATE_MOLE_1;}
			else if(event == EVENT_POPUP_TIMER)	{nextState = STATE_MOLE_2;}
			else if(event == EVENT_MOLE_TIMER)	{nextState = STATE_MOLE_0;}
			else if(event == EVENT_GAME_TIMER)	{nextState = STATE_GAME_OVER;}
			break;
		case STATE_MOLE_2:
			if(event == EVENT_MOLE_CLICK) {nextState = STATE_MOLE_1;}
			else if(event == EVENT_MISCLICK)	{nextState = STATE_MOLE_2;}
			else if(event == EVENT_POPUP_TIMER)	{nextState = STATE_MOLE_3;}
			else if(event == EVENT_MOLE_TIMER)	{nextState = STATE_MOLE_1;}
			else if(event == EVENT_GAME_TIMER)	{nextState = STATE_GAME_OVER;}
			break;
		case STATE_MOLE_3:
			if(event == EVENT_MOLE_CLICK) {nextState = STATE_MOLE_2;}
			else if(event == EVENT_MISCLICK)	{nextState = STATE_MOLE_3;}
			else if(event == EVENT_POPUP_TIMER)	{nextState = STATE_MOLE_4;}
			else if(event == EVENT_MOLE_TIMER)	{nextState = STATE_MOLE_2;}
			else if(event == EVENT_GAME_TIMER)	{nextState = STATE_GAME_OVER;}
			break;
		case STATE_MOLE_4:
			if(event == EVENT_MOLE_CLICK) {nextState = STATE_MOLE_3;}
			else if(event == EVENT_MISCLICK)	{nextState = STATE_MOLE_4;}
			else if(event == EVENT_POPUP_TIMER)	{nextState = STATE_MOLE_4;}
			else if(event == EVENT_MOLE_TIMER)	{nextState = STATE_MOLE_3;}
			else if(event == EVENT_GAME_TIMER)	{nextState = STATE_GAME_OVER;}
			break;
		case STATE_GAME_OVER:
			if(event == EVENT_RESET_TIMER) {nextState = STATE_CLICK_TO_PLAY;}
			break;
		default:
			nextState = STATE_CLICK_TO_PLAY;
	}

	// Update current state
	currState = nextState;
}