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

volatile int inProgress[6] = {0};
volatile int debugging = 0;

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
			printf("Feedback waiting timeout\n");
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

	inProgress[CONVEYOR] = RUNNING;
	control_Conveyor();
}

void start_Stars() {
	if(inProgress[STARS]) return;
	inProgress[STARS] = STARTING;
	printf("Starting stars\n");
	control_Stars();
	CHECK(STARS);
	Process_Timeout();
	CHECK(STARS);
	WRITE_SIGNAL("485.rsrs.rm_u2_on7", 1);
	inProgress[STARS] = RUNNING;
	control_Stars();
}

void start_Oil() {
	if(inProgress[OIL]) return;
	printf("Starting oil station\n");
	inProgress[OIL] = STARTING;

	Process_Timeout();
	CHECK(OIL);

	int bki = Get_Signal("wago.bki_k2.M2");
	if(bki) {
		printf("BKI error!\n");
		stop_Oil();
	}
	control_Oil(); // Check temp
	CHECK(OIL);

	WRITE_SIGNAL("wago.oc_mdo1.ka2_1", 1);
	if(!waitForFeedback("wago.oc_mdi1.oc_w_k2", 3, &inProgress[OIL])) {
		printf("Feedback error, stopping oil station\n");
		stop_Oil();
		return;
	}

	inProgress[OIL] = RUNNING;
	control_Oil();
}

void start_Hydratation() {
	if(inProgress[HYDRATATION]) return;
	printf("Starting hydratation\n");
	inProgress[HYDRATATION] = STARTING;

	Process_Timeout();
	CHECK(HYDRATATION);

	int bki = Get_Signal("wago.bki_k5.M5");
	if(bki) {
		printf("BKI error!\n");
		stop_Hydratation();
	}
	control_Hydratation(); // Check temp
	CHECK(HYDRATATION);

	WRITE_SIGNAL("wago.oc_mdo1.ka4_1", 1);
	WRITE_SIGNAL("wago.oc_mdo1.water1", 1);
	if(!waitForFeedback("wago.oc_mdi1.oc_w_k4", 3, &inProgress[HYDRATATION])) {
		printf("Feedback error, stopping hydratation\n");
		stop_Hydratation();
		return;
	}

	inProgress[HYDRATATION] = 1;
	control_Hydratation();
}

void start_Organ() {
	if(inProgress[HYDRATATION] != RUNNING && !debugging) return;
	if(inProgress[ORGAN]) return;
	printf("Starting organ\n");
	inProgress[ORGAN] = STARTING;

	Process_Timeout();
	CHECK(ORGAN);

	int bki = Get_Signal("wago.bki_k1.M1");
	if(bki) {
		printf("BKI error!\n");
		stop_Organ();
	}
	control_Organ(); // Check temp
	CHECK(ORGAN);

	WRITE_SIGNAL("wago.oc_mdo1.ka1_1", 1);
	if(!waitForFeedback("wago.oc_mdi1.oc_w_k1", 3, &inProgress[ORGAN])) {
		printf("Feedback error, stopping organ\n");
		stop_Organ();
		return;
	}

	inProgress[ORGAN] = RUNNING;
	control_Organ();
}


void control_Overloading() {
	if(!inProgress[OVERLOADING]) return;
	int temp = Get_Signal("wago.oc_temp.pt100_m6");
	int tempRelay = Get_Signal("wago.ts_m1.rele_T_m6");

	if(tempRelay) {
		printf("Overloading temp relay error!\n");
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
		printf("Conveyor temp relay error!\n");
		stop_Conveyor();
	}
}

void control_Stars() {
	if(!inProgress[STARS]) return;
	if(inProgress[OIL] != RUNNING && !debugging) {
		printf("Oil station is not running!");
		stop_Stars();
	}
}

void control_Oil() {
	if(!inProgress[OIL]) return;
	int temp = Get_Signal("wago.oc_temp.pt100_m2");
	int tempRelay = Get_Signal("wago.ts_m1.rele_T_m2");

	if(tempRelay) {
		printf("Oil station temp relay error!\n");
		stop_Oil();
	}
}

void control_Hydratation() {
	if(!inProgress[HYDRATATION]) return;
	int temp = Get_Signal("wago.oc_temp.pt100_m5");
	int tempRelay = Get_Signal("wago.ts_m1.rele_T_m5");
	static int lastWaterFlowState = RD;
	int waterFlowState;

	waterFlowState = Get_Signal_Ex(Get_Signal_Idx("485.ad1.adc3.flow"));

	if(waterFlowState != lastWaterFlowState) {
		READ_SIGNAL("485.ad1.adc3.flow");
		waterFlowState = lastWaterFlowState;
	}

	int water = Get_Signal("485.ad1.adc3.flow");

	if(tempRelay) {
		printf("Organ temp relay error!\n");
		stop_Hydratation();
	}
}

void control_Organ() {
	if(!inProgress[ORGAN]) return;
	if(inProgress[HYDRATATION] != RUNNING && !debugging) stop_Organ();
	int temp = Get_Signal("wago.oc_temp.pt100_m1");
	int tempRelay = Get_Signal("wago.ts_m1.rele_T_m1");
	int waterFlow = Get_Signal("485.ad1.adc3.flow");
	READ_SIGNAL("485.ad1.adc3.flow");

	if(waterFlow) {
	}

	if(tempRelay) {
		printf("Organ temp relay error!\n");
		stop_Organ();
	}
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
	printf("Stopping stars\n");
	WRITE_SIGNAL("485.rsrs.rm_u2_on7", 0);
	inProgress[STARS] = 0;
}

void stop_Oil() {
	if(!debugging) stop_Stars();
	if(!inProgress[OIL]) return;
	printf("Stopping oil station\n");
	WRITE_SIGNAL("wago.oc_mdo1.ka2_1", 0);
	inProgress[OIL] = 0;
}

void stop_Hydratation() {
	if(!debugging) stop_Organ();
	if(!inProgress[HYDRATATION]) return;
	WRITE_SIGNAL("wago.oc_mdo1.ka4_1", 0);
	WRITE_SIGNAL("wago.oc_mdo1.water1", 0);
	inProgress[HYDRATATION] = 0;
}

void stop_Organ() {
	if(!inProgress[ORGAN]) return;
	WRITE_SIGNAL("wago.oc_mdo1.ka1_1", 0);
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
