#include "signals.h"
#include "keyboard.h"
#include "ringbuffer.h"
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#define MODE_IDLE			0
#define MODE_PUMPING			1
#define MODE_DIAG			2
#define MODE_NORM			3
#define MODE_RELOADER			4
#define MODE_CONV			5

extern int Get_Signal_Ex(int ID);
extern int Get_Signal_Idx(char *name);
extern int Set_Signal_Ex_Val(int idx, int Ex, int Val);
extern int Set_Signal_Ex(int idx, int Ex);

pthread_mutex_t g_waitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_waitCond = PTHREAD_COND_INITIALIZER;
pthread_t worker;
static int g_mode = 0, g_workStarted = 0;

int Get_Signal(char *name) {
	struct Signal *s = hash_find(Signal_Name_Hash, Signal_Array, name);

	if(s) {
		return s->Value[1];
	}

	return -1;
}

static void Worker_Set_Mode(int mode) {
	if(g_mode == mode) {
		return;
	}
	if(g_mode == MODE_IDLE) {
		while(g_workStarted != 0)
			pthread_yield();
		pthread_mutex_lock(&g_waitMutex);
		g_mode = mode;
		pthread_mutex_unlock(&g_waitMutex);
		pthread_cond_signal(&g_waitCond);
		while(g_workStarted == 0)
			pthread_yield();
		return;
	} else {
		if(mode != MODE_IDLE) {
			Worker_Set_Mode(MODE_IDLE);
			Worker_Set_Mode(mode);
			return;
		}
		pthread_mutex_lock(&g_waitMutex);
		g_mode = mode;
		pthread_mutex_unlock(&g_waitMutex);
		while(g_workStarted != 0)
			pthread_yield();
	}
}

void Process_RED_BUTTON() {
	Worker_Set_Mode(MODE_IDLE);
}

void Process_Local_Kb() {
/*
485.kb.kbl.start_reloader
485.kb.kbl.start_conveyor
485.kb.kbl.start_stars
485.kb.kbl.start_oil_station
485.kb.kbl.start_hydratation
485.kb.kbl.start_exec_dev

*/
#define CRIT_STOP		"485.kb.kei1.stop_alarm"
#define START_RELOADER		"485.kb.key.start_reloader"
#define START_CONV		"485.kb.key.start_conveyor"
#define START_STARS		"485.kb.key.start_stars"
#define START_OIL_ST		"485.kb.key.start_oil_station"
#define START_HYDRO		"485.kb.key.start_hydratation"
#define START_EXECDEV		"485.kb.key.start_exec_dev"
#define START_ALL		"485.kb.kei1.start_all"

#define CONV_STOP		"485.kb.key.stop_conveyor"
#define RELOADER_STOP		"485.kb.key.stop_reloader"
#define STARS_STOP		"485.kb.key.stop_stars"
#define OIL_ST_STOP		"485.kb.key.stop_oil_station"
#define HYDRO_STOP		"485.kb.key.stop_hydratation"
#define EXECDEV_STOP		"485.kb.key.stop_exec_dev"
#define ALL_STOP		"485.kb.kei1.stop_all"


//int start_stars = Get_Signal(START_STARS), stop_stars = Get_Signal(STARS_STOP);
int start_reloader = Get_Signal(START_RELOADER), stop_reloader = Get_Signal(RELOADER_STOP);
	if(stop_reloader > 0) {
		printf("Stop_stars button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_reloader"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka6_1"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_reloader > 0) {
		printf("Start_stars button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_reloader"), 1, WR); //led is on
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka6_1"), 1, WR); // rm U2  bit 7
		Worker_Set_Mode(MODE_IDLE);
	}

/*
int start_reloader = Get_Signal(START_RELOADER), stop_reloader = Get_Signal(RELOADER_STOP);
	if(stop_reloader > 0) {
		printf("Stop_reloader button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_reloader"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka6_1"), 0, WR);
	        
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_reloader > 0) {
		printf("Start_reloader button pressed\n");
		//if(g_mode != MODE_NORM) {
			printf("Starting\n");
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_reloader"), 1, WR);
			Set_Signal_Ex(Get_Signal_Idx("wago.bki_k6.M6"), RD); //????
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m6"), RD); //???
			
			/*
			wago.bki_k6.M6	  // if =1 -> 
			wago.bki_R_k6_M6
			wago.oc_mdi1.oc_w_k5 	// if =0 ->
			wago.oc_mui7.current_m6a
			wago.oc_mui7.current_m6b
			wago.oc_mui7.current_m6c
			*/
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"), RD); 
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka5_1"), RD);
			//Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.start_oil_pump"), WR, 1);
	                ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka6_1"), 1, WR);
			Worker_Set_Mode(MODE_IDLE);
		//}
	}
*/
int start_conv = Get_Signal(START_CONV), stop_conv = Get_Signal(CONV_STOP);
	if(stop_conv > 0) {
		printf("Stop_conv button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_conveyor"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka3_1"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_conv > 0) {
		printf("Start_conv button pressed\n");
		                ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_conveyor"), 1, WR);
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka3_1"), 1, WR);
		Worker_Set_Mode(MODE_IDLE);
		/*
		if(g_mode != MODE_CONV) {
			printf("Starting\n");
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_conveyor"), 1, WR);
			Set_Signal_Ex(Get_Signal_Idx("wago.bki_k3_k4.M3_M4"), RD); //????
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m6"), RD); //???
			
			
			//wago.bki_k6.M6	  // if =1 -> 
			//wago.bki_R_k6_M6
			//wago.oc_mdi1.oc_w_k5 	// if =0 ->
			//wago.oc_mui7.current_m6a
			//wago.oc_mui7.current_m6b
			//wago.oc_mui7.current_m6c
			
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"), RD); 
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka3_1"), RD);
			//Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.start_oil_pump"), WR, 1);
			Worker_Set_Mode(MODE_CONV);
		}
			*/
		
	}

int start_stars = Get_Signal(START_STARS), stop_stars = Get_Signal(STARS_STOP);
	if(stop_stars > 0) {
		printf("Stop_stars button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_stars"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs.rm_u2_on7"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_stars > 0) {
		printf("Start_stars button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_stars"), 1, WR); //led is on
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs.rm_u2_on7"), 1, WR); // rm U2  bit 7
		Worker_Set_Mode(MODE_IDLE);
	}


int start_oil_st = Get_Signal(START_OIL_ST), stop_oil_st = Get_Signal(OIL_ST_STOP);
	if(stop_oil_st > 0) {
		printf("Stop_Oil_station button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_oil_station"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka2_1"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_oil_st > 0) {
		printf("Start_Oil_station button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_oil_station"), 1, WR); //led is on
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka2_1"), 1, WR); // rm U2  bit 7
		Worker_Set_Mode(MODE_IDLE);
	}


int start_hydro = Get_Signal(START_HYDRO), stop_hydro = Get_Signal(HYDRO_STOP);
	if(stop_hydro > 0) {
		printf("Stop_Hydro button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_hydratation"), 0, WR); //led is off
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka4_1"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_hydro > 0) {
		printf("Start_Hydro button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_hydratation"), 1, WR); //led is on
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka4_1"), 1, WR); // rm U2  bit 7
		Worker_Set_Mode(MODE_IDLE);
	}



int start_execdev = Get_Signal(START_EXECDEV), stop_execdev = Get_Signal(EXECDEV_STOP);
	if(stop_execdev > 0) {
		printf("Stop_ExecDev button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_exec_dev"), 0, WR); //led is off		
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka1_1"), 0, WR);
		Worker_Set_Mode(MODE_IDLE);
	} else if(start_execdev > 0) {
		printf("Start_ExecDev button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("A485.kb.kbl.start_exec_dev"), 1, WR); //led is on
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka1_1"), 1, WR); // rm U2  bit 7
		Worker_Set_Mode(MODE_IDLE);
	}



}

void Process_Cable_Kb() {
}

void Process_Radio_Kb() {
}

void Process_Pumping() {
#define PUMPING_START		"485.kb.key.start_oil_pump"
#define PUMPING_STOP		"485.kb.key.stop_oil_pump"
	int start = Get_Signal(PUMPING_START), stop = Get_Signal(PUMPING_STOP);

	if(stop > 0) {
		printf("Stop button pressed\n");
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 1, WR);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_oil_pump"), 0, WR);
	        ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka7_1"), 0, WR);		
		Worker_Set_Mode(MODE_IDLE);
	} else if(start > 0) {
		printf("Start button pressed\n");
		if(g_mode != MODE_PUMPING) {
			printf("Starting\n");
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.led_contrast"), 50, WR);
				ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.kb.kbl.start_oil_pump"), 1, WR);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_bki.M7"), RD);
			//Set_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m7"), RD);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"), RD);
			Set_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka7_1"), RD);
			//Set_Signal_Ex_Val(Get_Signal_Idx("485.kb.kbl.start_oil_pump"), WR, 1);
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
	printf("Got time.\n");
	while(g_mode != 0) {
		clock_gettime(CLOCK_REALTIME, &now);
		if((now.tv_sec > start.tv_sec + 5) || (now.tv_sec == start.tv_sec + 5) && (now.tv_nsec >= start.tv_nsec)) {
			break;
		}
		usleep(1000);
	}

	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_on"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound1_led"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_on"), 0, WR);
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("485.rsrs2.state_sound2_led"), 0, WR);
}

///////////////////////////////////////////////////////////////////////////////////////////
void Work_Conv(){
#define CHECK_MODE_CONV() if(g_mode != MODE_CONV) { return; }
	int exState;
	printf("Checkpoint1\n");
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.bki_k3_k4.M3_M4"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_CONV();
	} while(exState != RD);
	do {    
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m3"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_CONV();
	} while(exState == RD);
	do {    
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m4"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_CONV();
	} while(exState == RD);
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka3_1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_CONV();
	} while(exState == RD);
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_CONV();
	} while(exState == RD);
	printf("Checkpoint2\n");
	int tMotorM3 = Get_Signal("wago.oc_temp.pt100_m3");
	int tMotorM4 = Get_Signal("wago.oc_temp.pt100_m4");
	int bki = Get_Signal("wago.bki_R_k3_k4.M3_M4");
	int eugenAllowance = !Get_Signal("wago.oc_mdo1.ka3_1");
        printf("bki: %d\n",bki);
	printf("Eugen: %d; bki: %d\n", eugenAllowance,  bki);
	printf("Checkpoint3\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka3_1"), 1, WR);
	        /*
	do {  	
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka7_1"));
			if(exState != 3) usleep(10000);
		} while(bki != 0);
		*/
		
	while(g_mode == MODE_CONV) {
	/*
	wago.oc_mui4.current_m3a
	wago.oc_mui4.current_m3b
	wago.oc_mui4.current_m3c
	wago.oc_mui5.current_m4a
	wago.oc_mui5.current_m4b
	wago.oc_mui5.current_m4c
	*/
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui4.current_m3a"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui4.current_m3b"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui4.current_m3c"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui5.current_m4a"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui5.current_m4b"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui5.current_m4c"), 0, RD);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui4.current_m3a"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui4.current_m3b"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui4.current_m3c"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);

		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui5.current_m4a"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui5.current_m4b"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui5.current_m4c"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);

		int m3a = Get_Signal("wago.oc_mui4.current_m3a");
		int m3b = Get_Signal("wago.oc_mui4.current_m3b");
		int m3c = Get_Signal("wago.oc_mui4.current_m3c");
		int m4a = Get_Signal("wago.oc_mui5.current_m4a");
		int m4b = Get_Signal("wago.oc_mui5.current_m4b");
		int m4c = Get_Signal("wago.oc_mui5.current_m4c");
		printf("m3a: %d; m3b: %d; m3c: %d m4a: %d; m4b: %d; m4c: %d\n", m3a,m3b,m3c, m4a,m4b,m4c);
	}
	printf("Checkpoint end\n");		
}


void Work_Norm(){
#define CHECK_MODE_NORM() if(g_mode != MODE_NORM) { return; }
	int exState;
	printf("Checkpoint1\n");
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.bki_R_k6_M6"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_NORM();
	} while(exState != RD);
	do {    
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m6"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_NORM();
	} while(exState == RD);
	
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka5_1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_NORM();
	} while(exState == RD);
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_NORM();
	} while(exState == RD);
	printf("Checkpoint2\n");
	int tMotor = Get_Signal("wago.oc_temp.pt100_m6");
	int bki = Get_Signal("wago.bki_R_k6_M6");
	int eugenAllowance = !Get_Signal("wago.oc_mdo1.ka5_1");
        printf("bki: %d\n",bki);
	printf("Eugen: %d; bki: %d\n", eugenAllowance,  bki);
	printf("Checkpoint3\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka5_1"), 1, WR);
	        /*
	do {  	
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka7_1"));
			if(exState != 3) usleep(10000);
		} while(bki != 0);
		*/
		
	while(g_mode == MODE_NORM) {
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui7.current_m6a"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui7.current_m6b"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui7.current_m6c"), 0, RD);

		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui7.current_m6a"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui7.current_m6b"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui7.current_m6c"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);

		int m6a = Get_Signal("wago.oc_mui7.current_m6a");
		int m6b = Get_Signal("wago.oc_mui7.current_m6b");
		int m6c = Get_Signal("wago.oc_mui7.current_m6c");
		printf("m7a: %d; m7b: %d; m7c: %d\n", m6a,m6b,m6c);
	}
	printf("Checkpoint end\n");		
}

void Work_Pumping() {
#define CHECK_MODE_PUMPING() if(g_mode != MODE_PUMPING) { return; }
	int exState;

	printf("Checkpoint1\n");
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.bki_R_k7_M7"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while(exState != RD);
	do {    
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_temp.pt100_m7"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while(exState == RD);
	
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdo1.ka7_1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while(exState == RD);
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_qf1"));
		if(exState == RD) pthread_yield();
		CHECK_MODE_PUMPING();
	} while(exState == RD);
	printf("Checkpoint2\n");

	int tMotor = Get_Signal("wago.oc_temp.pt100_m7");
	int bki = Get_Signal("wago.bki_R_k7_M7");
	int eugenAllowance = !Get_Signal("wago.oc_mdo1.ka7_1");
        printf("bki: %d\n",bki);
        /*
        do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.bki_R_k7_M7"));
			if(exState != 3) usleep(10000);
		} while(bki != 0);
		
	if(bki != 0) { //!eugenAllowance || tMotor >= 70 || bki != 0) {
		printf("Eugen: %d; bki: %d\n", eugenAllowance,  bki);
		printf("Eugen: %d; tMotor: %d; bki: %d\n", eugenAllowance, tMotor, bki);
		Worker_Set_Mode(MODE_IDLE);
		return;
	}
	*/
        printf("Eugen: %d; bki: %d\n", eugenAllowance,  bki);
	printf("Checkpoint3\n");
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka7_1"), 1, WR);
      /*
	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdi1.oc_w_k6"), 0, RD);
	do {
		exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi1.oc_w_k6"));
		if(exState == RD) usleep(10000);
	} while(exState == RD && (g_mode == MODE_PUMPING));

	int oc  = Get_Signal("wago.oc_mdi1.oc_w_k6");       
	if(oc != 1) {
		//Worker_Set_Mode(MODE_IDLE);
	}
      */
	printf("Checkpoint4\n");
	/*
	while(g_mode == MODE_PUMPING) {
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui8.current_m7a"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui8.current_m7b"), 0, RD);
		ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mui8.current_m7c"), 0, RD);

		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui8.current_m7a"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui8.current_m7b"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);
		do {
			exState = Get_Signal_Ex(Get_Signal_Idx("wago.oc_mui8.current_m7c"));
			if(exState != 3) usleep(10000);
		} while(exState != 3);

		int m7a = Get_Signal("wago.oc_mui8.current_m7a");
		int m7b = Get_Signal("wago.oc_mui8.current_m7b");
		int m7c = Get_Signal("wago.oc_mui8.current_m7c");
		//printf("m7a: %d; m7b: %d; m7c: %d\n", m7a,m7b,m7c);
	}
	*/
	//ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx("wago.oc_mdo1.ka7_1"), 0, WR);
	
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
			g_workStarted = 1;
			pthread_mutex_unlock(&g_waitMutex);
		}

		printf("Mode switched to %d\n", g_mode);

		if(oldMode != g_mode && oldMode == 0) {
			printf("Processing timeout\n");
			oldMode = g_mode;
			Process_Timeout();
		}

		if(MODE_CHANGED) {
			continue;
		}

		switch(g_mode) {
		case MODE_PUMPING:
			Work_Pumping();
			break;
		}
	}
}

void Init_Worker() {
	pthread_create(&worker, NULL, &Worker, NULL);
}
