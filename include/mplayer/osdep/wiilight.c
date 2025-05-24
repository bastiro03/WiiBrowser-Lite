#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>

struct timespec _wiilight_timeOn;
struct timespec _wiilight_timeOff;

int WIILIGHT_IsOn;
int WIILIGHT_Level;

void _wiilight_turn(int enable);
static void * _wiilight_loop(void *arg);
static vu32 *_wiilight_reg = (u32*)0xCD0000C0;
lwp_t _wiilight_thread;


void WIILIGHT_Init();
void WIILIGHT_TurnOn();
int WIILIGHT_GetLevel();
int WIILIGHT_SetLevel(int level);

void WIILIGHT_Toggle();
void WIILIGHT_TurnOff();

void WIILIGHT_Init(){
	_wiilight_timeOn.tv_sec=(time_t)0;
	_wiilight_timeOff.tv_sec=(time_t)0;
	WIILIGHT_IsOn=0;
	WIILIGHT_SetLevel(0);
}


int WIILIGHT_GetLevel(){
	return WIILIGHT_Level;
}

//
// here is where we set the light intensity
// it turns on the light and 
// waits level*40000 nanoseconds
// (so, if it is 255 it will wait for 10200000 nanoseconds) 
// (that means ~ 1/100secs)
// 
// for the time turned off it is 10200000-level_on
// (so, if level is 255, it will wait for 0 seconds)
// (that means full light intensity)
//

int WIILIGHT_SetLevel(int level){//0 to 255
	long level_on;
	long level_off;
	if(level>255) level=255;
	if(level<0) level=0;
	level_on=level*40000;
	level_off=10200000-level_on;
	_wiilight_timeOn.tv_nsec=level_on;
	_wiilight_timeOff.tv_nsec=level_off;
	WIILIGHT_Level=level;
	return level;
}

//
// just turn one bit
//

void _wiilight_turn(int enable){
	u32 val = (*_wiilight_reg&~0x20);
	if(enable) val |= 0x20;
	*_wiilight_reg=val;
}

void WIILIGHT_TurnOff(){
	
	WIILIGHT_IsOn=false;
	
}

//
// Set the var on, and start the looping thread
//

void WIILIGHT_TurnOn(){
	
	WIILIGHT_IsOn=true;
	LWP_CreateThread(&_wiilight_thread, _wiilight_loop,NULL,NULL,0,80);
	
}

void WIILIGHT_Toggle(){
	
	if(WIILIGHT_IsOn){
		WIILIGHT_TurnOff();
	}else{
		WIILIGHT_TurnOn();
	}
	
}

// 
// This is the looping thread.. it just loops
// turning on and off the light depending on the timing
// specified by SetLevel()
//

static void * _wiilight_loop(void *arg){
	
	struct timespec timeOn;
	struct timespec timeOff;
	while(WIILIGHT_IsOn){
		timeOn=_wiilight_timeOn;
		timeOff=_wiilight_timeOff;
		_wiilight_turn(1);
		nanosleep(&timeOn);
		if(timeOff.tv_nsec>0) _wiilight_turn(0);
		nanosleep(&timeOff);
	}
	
	return NULL;
	
}
