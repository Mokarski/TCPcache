#pragma once

#define RB_FLUSH() while(!RB_EMPTY()) pthread_yield()
#define WRITE_SIGNAL(signal, value)	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx(signal), value, WR);
#define READ_SIGNAL(signal)	ring_buffer_push(Signal_Mod_Buffer, Get_Signal_Idx(signal), 0, RD);
#define CHECK(what) 	if(!inProgress[what]) return;

extern int Get_Signal_Ex(int ID);
extern int Get_Signal_Idx(char *name);
extern int Set_Signal_Ex_Val(int idx, int Ex, int Val);
extern int Set_Signal_Ex(int idx, int Ex);

int Wait_For_Feedback(char *name, int expect, int timeout, volatile int *what);
void Process_Timeout();
int  Get_Signal(char *name);
void Init_Worker();
void Worker_Set_Mode(int mode);
void Process_Mode_Change();
void Process_RED_BUTTON();
void Process_Local_Kb();
void Process_Cable_Kb();
void Process_Radio_Kb();
void Process_Pumping();
void Process_Normal();
void Process_Diag();
