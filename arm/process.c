#include "process.h"
#include "keyboard.h"
#include "ringbuffer.h"
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#define IDLE				0
#define STARTING		1
#define RUNNING			2

#define OVERLOADING	0
#define CONVEYOR		1
#define STARS				2
#define OIL					3
#define HYDRATATION	4
#define ORGAN				5

static volatile int inProgress[6] = {0};

int waitForFeedback(char *name, int timeout, volatile int *what) {
	int oc  = 0;       
	struct timespec start, now;
	clock_gettime(CLOCK_REALTIME, &start);

	while(!oc && (*what)) {
		int exState;
		oc  = Get_Signal(name);
		if(oc) continue;
		if(exState == RD)
			usleep(10000);
		else if(RB_EMPTY())
			READ_SIGNAL(name);
		clock_gettime(CLOCK_REALTIME, &now);
		if((now.tv_sec > start.tv_sec + timeout) || (now.tv_sec == start.tv_sec + timeout) && (now.tv_nsec >= start.tv_nsec)) {
			return oc;
		}
	}

	return oc;
}

void start_Overloading() {
	if(inProgress[OVERLOADING]) return;
	printf("Starting overloading\n");
	inProgress[OVERLOADING] = STARTING;

	Process_Timeout();
	CHECK(OVERLOADING);

	int bki = Get_Signal("wago.bki_k6.M6");
	if(bki) {
		printf("BKI error!\n");
		stop_Overloading();
	}
	control_Overloading(); // Check temp
	CHECK(OVERLOADING);

	WRITE_SIGNAL("wago.oc_mdo1.ka5_1", 1);
	if(!waitForFeedback("wago.oc_mdi1.oc_w_k5", 3, &inProgress[OVERLOADING])) {
		stop_Overloading();
		return;
	}

	CHECK(OVERLOADING);
	inProgress[OVERLOADING] = RUNNING;
	control_Overloading();
}

void start_Conveyor() {
	if(inProgress[CONVEYOR]) return;
	printf("Starting conveyor\n");
	inProgress[CONVEYOR] = STARTING;

	Process_Timeout();
	CHECK(CONVEYOR);

	int bki = Get_Signal("wago.bki_k3_k4.M3_M4");
	if(bki) {
		printf("BKI error!\n");
		stop_Conveyor();
	}
	CHECK(CONVEYOR);

	WRITE_SIGNAL("wago.oc_mdo1.ka3_1", 1);
	if(!waitForFeedback("wago.oc_mdi1.oc_w_k3", 3, &inProgress[CONVEYOR])) {
		stop_Conveyor();
		return;
	}

	CHECK(CONVEYOR);
	inProgress[CONVEYOR] = RUNNING;
	control_Conveyor();
}

void start_Stars() {
	if(inProgress[STARS]) return;
	inProgress[STARS] = 1;
}

void start_Oil() {
	if(inProgress[OIL]) return;
	inProgress[OIL] = 1;
}

void start_Hydratation() {
	if(inProgress[HYDRATATION]) return;
	inProgress[HYDRATATION] = 1;
}

void start_Organ() {
	if(inProgress[ORGAN]) return;
	inProgress[ORGAN] = 1;
}


void control_Overloading() {
	if(!inProgress[OVERLOADING]) return;
	int temp = Get_Signal("wago.oc_temp.pt100_m6");
	int tempRelay = Get_Signal("wago.ts_m1.rele_T_m6");

	if(tempRelay) {
		stop_Overloading();
	}
}

void control_Conveyor() {
	if(!inProgress[CONVEYOR]) return;
	int temp1 = Get_Signal("wago.oc_temp.pt100_m3");
	int temp2 = Get_Signal("wago.oc_temp.pt100_m4");
	int tempRelay1 = Get_Signal("wago.ts_m1.rele_T_m3");
	int tempRelay2 = Get_Signal("wago.ts_m1.rele_T_m4");

	if(tempRelay1 || tempRelay2) {
		stop_Conveyor();
	}
}

void control_Stars() {
	if(!inProgress[STARS]) return;
}

void control_Oil() {
	if(!inProgress[OIL]) return;
}

void control_Hydratation() {
	if(!inProgress[HYDRATATION]) return;
}

void control_Organ() {
	if(!inProgress[ORGAN]) return;
}


void stop_Overloading() {
	if(!inProgress[OVERLOADING]) return;
	printf("Stopping overloading\n");
	WRITE_SIGNAL("wago.oc_mdo1.ka5_1", 0);
	inProgress[OVERLOADING] = 0;
}

void stop_Conveyor() {
	if(!inProgress[CONVEYOR]) return;
	printf("Stopping conveyor\n");
	WRITE_SIGNAL("wago.oc_mdo1.ka3_1", 0);
	inProgress[CONVEYOR] = 0;
}

void stop_Stars() {
	if(!inProgress[STARS]) return;
	inProgress[STARS] = 0;
}

void stop_Oil() {
	if(!inProgress[OIL]) return;
	inProgress[OIL] = 0;
}

void stop_Hydratation() {
	if(!inProgress[HYDRATATION]) return;
	inProgress[HYDRATATION] = 0;
}

void stop_Organ() {
	if(!inProgress[ORGAN]) return;
	inProgress[ORGAN] = 0;
}

void control_all() {
	control_Overloading();
	control_Conveyor();
	control_Stars();
	control_Oil();
	control_Hydratation();
	control_Organ();
}

void stop_all() {
	stop_Overloading();
	stop_Conveyor();
	stop_Stars();
	stop_Oil();
	stop_Hydratation();
	stop_Organ();
}
