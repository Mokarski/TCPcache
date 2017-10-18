#pragma once

#define RB_FLUSH() while(!RB_EMPTY()) pthread_yield()

extern int Get_Signal_Ex(int ID);
extern int Get_Signal_Idx(char *name);
extern int Set_Signal_Ex_Val(int idx, int Ex, int Val);
extern int Set_Signal_Ex(int idx, int Ex);

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
