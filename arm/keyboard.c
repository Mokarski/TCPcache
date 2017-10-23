#include "signals.h"
#include "keyboard.h"
#include "ringbuffer.h"
#define __USE_UNIX98
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#define MODE_IDLE			0
#define MODE_PUMPING	1
#define MODE_DIAG			2
#define MODE_NORM			3

#define B_OVERLOAD_START	0
#define B_OVERLOAD_STOP		1
#define B_CONV_START	2
#define B_CONV_STOP		3
#define B_ORGAN_START	4
#define B_ORGAN_STOP	5
#define B_OIL_START		6
#define B_OIL_STOP		7
#define B_HYDRA_START	8
#define B_HYDRA_STOP	9
#define B_STARS_START	10
#define B_STARS_STOP	11
#define B_CHECK_START	12
#define B_CHECK_STOP	13
#define B_SOUND_ALARM	14

#define	J_CONVEYOR		0
#define	J_ORGAN				1
#define	J_LEFT_T			2
#define	J_RIGHT_T			3
#define	J_ACCEL				4
#define	J_TELESCOPE		5
#define	J_RESERVE			6
#define	J_SUPPORT			7
#define	J_SOURCER			8

#define J_BIT_UP			0
#define J_BIT_DOWN		1
#define J_BIT_LEFT		2
#define J_BIT_RIGHT		3
#define JOYVAL_UP			0x1
#define JOYVAL_DOWN		0x2
#define JOYVAL_LEFT		0x4
#define JOYVAL_RIGHT	0x8

#define IS_UP(j)	((joystick[j] & JOYVAL_UP))
#define IS_DOWN(j)	((joystick[j] & JOYVAL_DOWN))
#define IS_LEFT(j)	((joystick[j] & JOYVAL_LEFT))
#define IS_RIGHT(j)	((joystick[j] & JOYVAL_RIGHT))

pthread_mutex_t g_waitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_waitCond = PTHREAD_COND_INITIALIZER;
pthread_t g_worker;
static volatile int g_mode = 0, g_workStarted = 0;
volatile int buttons[32] = {0};
volatile int joystick[32] = {0};

int Wait_For_Feedback(char *name, int expect, int timeout, volatile int *what) {
	int oc  = Get_Signal(name);       
	struct timespec start, now;
	clock_gettime(CLOCK_REALTIME, &start);

	printf("Waiting for feedback %d\n", expect);

	while((oc != expect) && (*what)) {
		int exState;
		oc  = Get_Signal(name);
		if(oc == expect) continue;
		if(exState == RD)
			usleep(10000);
		else if(RB_EMPTY())
			READ_SIGNAL(name);
		clock_gettime(CLOCK_REALTIME, &now);
		if((now.tv_sec > start.tv_sec + timeout) || (now.tv_sec == start.tv_sec + timeout) && (now.tv_nsec >= start.tv_nsec)) {
			printf("Result: %d\n", oc);
			return oc == expect;
		}
	}

	printf("Result: %d\n", oc);
	return oc == expect;
}

int Get_Signal(char *name) {
	struct Signal *s = hash_find(Signal_Name_Hash, Signal_Array, name);

	if(s) {
		return s->Value[1];
	}

	return -1;
}

void Worker_Set_Mode(int mode) {
	if(g_mode == mode) {
		return;
	}
	if(g_mode == MODE_IDLE) {
		while(g_workStarted != 0 && pthread_self() != g_worker)
			pthread_yield();
		pthread_mutex_lock(&g_waitMutex);
		g_mode = mode;
		pthread_mutex_unlock(&g_waitMutex);
		pthread_cond_signal(&g_waitCond);
		return;
	} else {
		if(mode != MODE_IDLE) {
			Worker_Set_Mode(MODE_IDLE);
			Worker_Set_Mode(mode);
			return;
		}
		//pthread_mutex_lock(&g_waitMutex);
		g_mode = mode;
		//pthread_mutex_unlock(&g_waitMutex);
	}
}

void Process_RED_BUTTON() {
	Worker_Set_Mode(MODE_IDLE);
}

void Process_Mode_Change() {
	Worker_Set_Mode(MODE_IDLE);
}

void Process_Local_Kb() {
	buttons[B_OVERLOAD_START] |= Get_Signal("485.kb.key.start_reloader");
	buttons[B_CONV_START] 	  |= Get_Signal("485.kb.key.start_conveyor");
	buttons[B_OVERLOAD_STOP]  |= Get_Signal("485.kb.key.stop_reloader");
	buttons[B_CONV_STOP] 			|= Get_Signal("485.kb.key.stop_conveyor");
	buttons[B_ORGAN_START]		|= Get_Signal("485.kb.key.start_exec_dev");
	buttons[B_ORGAN_STOP] 		|= Get_Signal("485.kb.key.stop_exec_dev");
	buttons[B_OIL_START] 			|= Get_Signal("485.kb.key.start_oil_station");
	buttons[B_OIL_STOP] 			|= Get_Signal("485.kb.key.stop_oil_station");
	buttons[B_HYDRA_START]		|= Get_Signal("485.kb.key.start_hydratation");
	buttons[B_HYDRA_STOP]			|= Get_Signal("485.kb.key.stop_hydratation");
	buttons[B_STARS_START]		|= Get_Signal("485.kb.key.start_stars");
	buttons[B_STARS_STOP]			|= Get_Signal("485.kb.key.stop_stars");
	buttons[B_CHECK_START]		|= Get_Signal("485.kb.key.start_check");
	buttons[B_CHECK_STOP]			|= Get_Signal("485.kb.key.stop_check");
	buttons[B_SOUND_ALARM]			= Get_Signal("485.kb.kei1.sound_alarm");

	joystick[J_CONVEYOR] = (Get_Signal("485.kb.kei1.conveyor_left") << J_BIT_LEFT) | (Get_Signal("485.kb.kei1.conveyor_right") << J_BIT_RIGHT) |
												 (Get_Signal("485.kb.kei1.conveyor_up") << J_BIT_UP) | (Get_Signal("485.kb.kei1.conveyor_down") << J_BIT_DOWN);
	joystick[J_ORGAN] = (Get_Signal("485.kb.kei1.exec_dev_left") << J_BIT_LEFT) | (Get_Signal("485.kb.kei1.exec_dev_down") << J_BIT_DOWN) | 
											(Get_Signal("485.kb.kei1.exec_dev_right") << J_BIT_RIGHT) | (Get_Signal("485.kb.kei1.exec_dev_up") << J_BIT_UP);
	joystick[J_LEFT_T] = (Get_Signal("485.kb.kei1.left_truck_back") << J_BIT_DOWN) | (Get_Signal("485.kb.kei1.left_truck_forward") << J_BIT_UP);
	joystick[J_RIGHT_T] = (Get_Signal("485.kb.kei1.right_truck_back") << J_BIT_DOWN) | (Get_Signal("485.kb.kei1.right_truck_forward") << J_BIT_UP);
	joystick[J_ACCEL] = (Get_Signal("485.kb.kei1.acceleration") << J_BIT_UP);
	joystick[J_TELESCOPE] = (Get_Signal("485.kb.kei1.telescope_up") << J_BIT_UP) | (Get_Signal("485.kb.kei1.telescope_down") << J_BIT_DOWN);
	joystick[J_RESERVE] = (Get_Signal("485.kb.kei1.reserve_down") << J_BIT_DOWN) | (Get_Signal("485.kb.kei1.reserve_up") << J_BIT_UP);
	joystick[J_SUPPORT] = (Get_Signal("485.kb.kei1.combain_support_down") << J_BIT_DOWN) | (Get_Signal("485.kb.kei1.combain_support_up") << J_BIT_UP);
	joystick[J_SOURCER] = (Get_Signal("485.kb.kei1.sourcer_down") << J_BIT_DOWN) | (Get_Signal("485.kb.kei1.sourcer_up") << J_BIT_UP);
}

void Process_Cable_Kb() {
}

void Process_Radio_Kb() {
	joystick[J_SUPPORT] = (Get_Signal("485.rpdu485.kei.support_down") << J_BIT_DOWN) | (Get_Signal("485.rpdu485.kei.support_up") << J_BIT_UP);
	joystick[J_SOURCER] = (Get_Signal("485.kb.kei1.sourcer_down") << J_BIT_DOWN) | (Get_Signal("485.rpdu485.kei.sourcer_up") << J_BIT_UP);
	joystick[J_ACCEL] = (Get_Signal("485.rpdu485.kei.acceleration_up") << J_BIT_UP);
	joystick[J_TELESCOPE] = (Get_Signal("485.rpdu485.kei.telescope_up") << J_BIT_UP) | (Get_Signal("485.rpdu485.kei.telescope_down") << J_BIT_DOWN);
	joystick[J_ORGAN] = (Get_Signal("485.rpdu485.kei.joy_exec_dev_left") << J_BIT_LEFT) | (Get_Signal("485.rpdu485.kei.joy_exec_dev_down") << J_BIT_DOWN) | 
											(Get_Signal("485.rpdu485.kei.joy_exec_dev_right") << J_BIT_RIGHT) | (Get_Signal("485.rpdu485.kei.joy_exec_dev_up") << J_BIT_UP);
	int jleft, jright, jup, jdown;
	joystick[J_LEFT_T] = 0;
	joystick[J_RIGHT_T] = 0;

	buttons[B_OVERLOAD_START] |= Get_Signal("485.rpdu485.kei.reloader_up");
	buttons[B_OVERLOAD_STOP]  |= Get_Signal("485.rpdu485.kei.reloader_down");
	buttons[B_CONV_START] 	  |= Get_Signal("485.rpdu485.kei.conveyor_up");
	buttons[B_CONV_STOP] 			|= Get_Signal("485.rpdu485.kei.conveyor_down");
	buttons[B_HYDRA_START]		 = buttons[B_ORGAN_START]		|= Get_Signal("485.rpdu485.kei.exec_dev_up");
	buttons[B_HYDRA_STOP]			 = buttons[B_ORGAN_STOP] 		|= Get_Signal("485.rpdu485.kei.exec_dev_down");
	buttons[B_OIL_START] 			|= Get_Signal("485.rpdu485.kei.oil_station_up");
	buttons[B_OIL_STOP] 			|= Get_Signal("485.rpdu485.kei.oil_station_down");
	
	buttons[B_SOUND_ALARM]			= Get_Signal("485.rpdu485.sound_beepl");
	//buttons[B_STARS_START]		|= Get_Signal("");
	//buttons[B_STARS_STOP]			|= Get_Signal("");
	//buttons[B_CHECK_START]		|= Get_Signal("");
	//buttons[B_CHECK_STOP]			|= Get_Signal("");
}

void Process_Normal() {
	Worker_Set_Mode(MODE_NORM);
}

void Process_Pumping() {
#define PUMPING_START		"485.kb.key.start_oil_pump"
#define PUMPING_STOP		"485.kb.key.stop_oil_pump"
	int start = Get_Signal(PUMPING_START), stop = Get_Signal(PUMPING_STOP);

	if(stop > 0) {
		printf("Stop button pressed\n");
		//ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka7_1"), 0, WR);		
		printf("Setting idle mode\n");
		Worker_Set_Mode(MODE_IDLE);
	} else if(start > 0) {
		printf("Start button pressed\n");
		if(g_mode != MODE_PUMPING) {
			printf("Starting\n");
			printf("Lighting the button contrast\n");
			Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.led_contrast"), WR, 50);
			Set_Signal_Ex_Val(Get_Signal_Idx("panel10.system_state_code"),WR,1);
			printf("Lighting the start oil pump button\n");
			Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.start_oil_pump"), WR, 1);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_bki.M7"), RD);
			//Set_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m7"), RD);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"), RD);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka7_1"), RD);
			//Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.start_oil_pump"), WR, 1);
			printf("Setting mode\n");
			Worker_Set_Mode(MODE_PUMPING);
		}
	}
}

void Process_Diag() {
}

///////////////////////////////////////////////////////////////////////////////////////////

void Process_Timeout() {
	struct timespec start, now;
	printf("Processing timeout for mode %d\n", g_mode);

	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_on"), 1, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_led"), 1, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_on"), 1, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_led"), 1, WR);

	printf("Getting time\n", g_mode);
	clock_gettime(CLOCK_REALTIME, &start);
	printf("Got time\n");
	while(g_mode != 0) {
		clock_gettime(CLOCK_REALTIME, &now);
		if((now.tv_sec > (start.tv_sec + 5)) || (now.tv_sec == (start.tv_sec + 5)) && (now.tv_nsec >= start.tv_nsec)) {
			printf("Exiting by timeout\n");
			break;
		}
		usleep(1000);
	}

	if(g_mode == 0) {
		printf("Mode changed!!!!!!!!!!!!!!!!!!!!!!\n");
	}

	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_on"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_led"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_on"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_led"), 0, WR);
}

///////////////////////////////////////////////////////////////////////////////////////////
#define LEFT_TRACK_FW	0
#define LEFT_TRACK_BW	1
#define RIGHT_TRACK_FW	2
#define RIGHT_TRACK_BW	3
#define	ASSOCIATE_CONTROL(what, value, signal)	\
if(joystick[what] & value) { \
		if(!(controls[what] & value)) { \
			controls[what] |= value; \
			WRITE_SIGNAL(signal, 1); \
		} \
} else { \
		if(controls[what] & value) { \
			controls[what] &= ~value; \
			WRITE_SIGNAL(signal, 0); \
		} \
}
struct timespec last_moving;

void Process_Joysticks() {
	static int controls[16] = {0};

	if(joystick[J_LEFT_T] && !controls[J_LEFT_T] && !controls[J_RIGHT_T] || 
		 joystick[J_RIGHT_T] && !controls[J_RIGHT_T] && !controls[J_LEFT_T]) {
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		if(now.tv_sec - last_moving.tv_sec > 6) {
			Process_Timeout();
		}
	}

	ASSOCIATE_CONTROL(J_LEFT_T, JOYVAL_UP, "485.rsrs.rm_u2_on10");
	ASSOCIATE_CONTROL(J_LEFT_T, JOYVAL_DOWN, "485.rsrs.rm_u2_on11");
	ASSOCIATE_CONTROL(J_RIGHT_T, JOYVAL_UP, "485.rsrs.rm_u2_on0");
	ASSOCIATE_CONTROL(J_RIGHT_T, JOYVAL_DOWN, "485.rsrs.rm_u2_on1");

	ASSOCIATE_CONTROL(J_ORGAN, JOYVAL_UP, "485.rsrs.rm_u1_on6");
	ASSOCIATE_CONTROL(J_ORGAN, JOYVAL_DOWN, "485.rsrs.rm_u1_on7");
	ASSOCIATE_CONTROL(J_ORGAN, JOYVAL_LEFT, "485.rsrs.rm_u1_on3");
	ASSOCIATE_CONTROL(J_ORGAN, JOYVAL_RIGHT, "485.rsrs.rm_u1_on2");

	ASSOCIATE_CONTROL(J_ACCEL, JOYVAL_UP, "485.rsrs.rm_u1_on0");

	ASSOCIATE_CONTROL(J_TELESCOPE, JOYVAL_UP, "485.rsrs.rm_u1_on4");
	ASSOCIATE_CONTROL(J_TELESCOPE, JOYVAL_DOWN, "485.rsrs.rm_u1_on5");

	ASSOCIATE_CONTROL(J_SUPPORT, JOYVAL_UP, "485.rsrs.rm_u2_on8");
	ASSOCIATE_CONTROL(J_SUPPORT, JOYVAL_DOWN, "485.rsrs.rm_u2_on9");

	ASSOCIATE_CONTROL(J_SOURCER, JOYVAL_UP, "485.rsrs.rm_u1_on8");
	ASSOCIATE_CONTROL(J_SOURCER, JOYVAL_DOWN, "485.rsrs.rm_u1_on9");

	ASSOCIATE_CONTROL(J_CONVEYOR, JOYVAL_UP, "485.rsrs.rm_u2_on2");
	ASSOCIATE_CONTROL(J_CONVEYOR, JOYVAL_DOWN, "485.rsrs.rm_u2_on3");
	ASSOCIATE_CONTROL(J_CONVEYOR, JOYVAL_LEFT, "485.rsrs.rm_u2_on5");
	ASSOCIATE_CONTROL(J_CONVEYOR, JOYVAL_RIGHT, "485.rsrs.rm_u2_on4");

	if(controls[J_LEFT_T] || controls[J_RIGHT_T]) {
		clock_gettime(CLOCK_REALTIME, &last_moving);
	}
}

void Work_Norm(){
	int alarm_enabled = 0;
	printf("Working in normal mode\n");
	while(g_mode == MODE_NORM) {
		if(buttons[B_SOUND_ALARM] && !alarm_enabled) {
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_on"), 1, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_led"), 1, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_on"), 1, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_led"), 1, WR);
			alarm_enabled = 1;
		} else if(!buttons[B_SOUND_ALARM] && alarm_enabled) {
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_on"), 0, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_led"), 0, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_on"), 0, WR);
			ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_led"), 0, WR);
			alarm_enabled = 0;
		}
		if(buttons[B_OVERLOAD_STOP]) {
			stop_Overloading();
			buttons[B_OVERLOAD_START] = buttons[B_OVERLOAD_STOP] = 0;
		} else if(buttons[B_OVERLOAD_START]) {
			start_Overloading();
			buttons[B_OVERLOAD_START] = 0;
		}
		if(buttons[B_CONV_STOP]) {
			stop_Conveyor();
			buttons[B_CONV_START] = buttons[B_CONV_STOP] = 0;
		} else if(buttons[B_CONV_START]) {
			start_Conveyor();
			buttons[B_CONV_START] = 0;
		}
		if(buttons[B_ORGAN_STOP]) {
			stop_Organ();
			buttons[B_ORGAN_STOP] = 0;
			buttons[B_ORGAN_START] = 0;
		} else if(buttons[B_ORGAN_START]) {
			start_Organ();
			buttons[B_ORGAN_START] = 0;
		}
		if(buttons[B_OIL_STOP]) {
			stop_Oil();
			buttons[B_OIL_STOP] = 0;
			buttons[B_OIL_START] = 0;
		} else if(buttons[B_OIL_START]) {
			start_Oil();
			buttons[B_OIL_START] = 0;
		}
		if(buttons[B_HYDRA_STOP]) {
			stop_Hydratation();
			buttons[B_HYDRA_STOP] = 0;
			buttons[B_HYDRA_START] = 0;
		} else if(buttons[B_HYDRA_START]) {
			start_Hydratation();
			buttons[B_HYDRA_START] = 0;
		}
		if(buttons[B_STARS_STOP]) {
			stop_Stars();
			buttons[B_STARS_STOP] = 0;
			buttons[B_STARS_START] = 0;
		} else if(buttons[B_STARS_START]) {
			start_Stars();
			buttons[B_STARS_START] = 0;
		}
		Process_Joysticks();
		control_all();
		usleep(10000);
	}

	printf("Exiting normal mode\n");
	stop_all();
}

void Work_Pumping() {
#define CHECK_MODE_PUMPING() if(g_mode != MODE_PUMPING) { return; }
	int exState;

	Process_Timeout();
	CHECK_MODE_PUMPING();

	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.bki_R_k7_M7"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while((g_mode == MODE_PUMPING) && exState != RD && !RB_EMPTY());
	do {    
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m7"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while((g_mode == MODE_PUMPING) && exState == RD && !RB_EMPTY());
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while((g_mode == MODE_PUMPING) && exState == RD && !RB_EMPTY());

	int tMotor = Get_Signal("wago.oc_temp.pt100_m7");
	int bki = Get_Signal("wago.bki_k7.M7");

	if(bki != 0) { // || tMotor >= 70) {
		printf("tMotor: %d; bki: %d\n", tMotor, bki);
		Worker_Set_Mode(MODE_IDLE);
		return;
	}
	printf("Waiting feedback for 3 sec\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka6_1"), 1, WR);
	RB_FLUSH();

	int oc = Wait_For_Feedback("wago.oc_mdi1.oc_w_k6", 1, 3, &g_mode);
	if(!oc) {
		printf("No Feedback!\n");
		printf("Feedback signal: %d\n", Get_Signal("wago.oc_mdi1.oc_w_k6"));
		printf("Feedback register: %d\n", Get_Signal("wago.oc_mdi1.oc"));
		Worker_Set_Mode(MODE_IDLE);
	}

	printf("Pumping started\n");
	while(g_mode == MODE_PUMPING) {
		int m7a = Get_Signal("wago.oc_mui8.current_m7a");
		int m7b = Get_Signal("wago.oc_mui8.current_m7b");
		int m7c = Get_Signal("wago.oc_mui8.current_m7c");
		// Check currents
	}
	int forever = 1;
	printf("Pumping stopped\n");
	while(!Wait_For_Feedback("wago.oc_mdi1.oc_w_k6", 0, 3, &forever)) {
		printf("Turning off wago\n");
		WRITE_SIGNAL("wago.oc_mdo1.ka6_1", 0);
	}
	printf("Dimming the button contrast\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
	printf("Dimming the start oil pump button\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_oil_pump"), 0, WR);
}

void *Worker(void* arg) {
#define MODE_CHANGED (oldMode != g_mode)
#define CHECK_MODE()	if(MODE_CHANGED) {  }
	int oldMode = g_mode;
	while(1) {
		if(!g_mode) {
			oldMode = 0;
			pthread_mutex_lock(&g_waitMutex);
			g_workStarted = 0;
			pthread_cond_wait(&g_waitCond, &g_waitMutex);
			pthread_mutex_unlock(&g_waitMutex);
		}

		g_workStarted = 1;
		printf("Mode switched to %d\n", g_mode);

		switch(g_mode) {
			case MODE_PUMPING:
				Work_Pumping();
				break;

			case MODE_NORM:
				Work_Norm();
				break;
		}
	}
}

void Init_Worker() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&g_waitMutex, &attr);
	pthread_create(&g_worker, NULL, &Worker, NULL);
}
