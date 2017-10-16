#ifndef _SIGNALS_H
#define _SIGNALS_H

//#define MAX 100
#define STR_LEN_TXT 100
#define MAX_Signals 400

//char packed_txt_string[40000];

struct Signal {
 int Srv_id_num;         // server number in server list. Assigned by server.
 char Name[150];         // Name signal

//if modbus signals type
    int  MB_Id;          // Modbus device ID
    int  MB_Reg_Num;     // Mb register number
    char Val_Type[10];   // type of value int or bit
    int  Bit_Pos;        // bit position

//if signals tcp
    int   TCP_Mb;         // TCP Modbus or tcp flow
    char  TCP_Type[3];       // TCP flag if set to 1 then tcp if set to 0 modbus rtu
    int   TCP_Addr;       // TCP address of signals 192.168.255.255 max size 18 digits

//values 
 int  Value[2];          // value 2 int
 char Reserv1[10];       // type of value int or bit
 char Reserv2[10];       // type of value int or bit
 char Reserv3[10];       // type of value int or bit
 
 //Arbitrage and on/off
 int Prio;               // Priority of signal 0 - no priority
 int TypeTrigger;        // signal has trigger state on/off 
 int Off;                // Signal not used if OFF = 1;
 
 //debug parametrs
 int  ExState;           // 0-not executed/unknown 1-for execution,2-executed,3-expired,4-dropped  Thread Arbitrage
 char Ex_Hw;             // Execution Hardware mark PcProc =1 Panel 43=2 Panel 10=3 | soft_class
 char Ex_Sf[100];        // Execution Software mark ModBusMaster_RTU =1 Modbus_Master_TCP=2 CoreSignal=100

 };

struct Signal Signal_Array[MAX_Signals];
                          
                     
void init_signals_list (void);
void print_signals_list (void);
void print_by_name (char *sName);

int  signals_txt(char *packed_txt_string);
int  signal_read(char sName[100]);

int sSerial_by_num (int, char *buf);
int sSerial_by_num_short (int, char *buf);

int sDeSerial_by_num (int);
int unpack_signal (char *str,int n);
int  signal_update_ex (char sName[100],int sVal, int Execution_State); 
void block_all_signals(void);
void unblock_all_signasl(void);
int  text_parser(char *str_in, char str_out1[STR_LEN_TXT], char str_out2[STR_LEN_TXT], char str_out3[STR_LEN_TXT], char str_out4[STR_LEN_TXT], char str_out5[STR_LEN_TXT]);
int  signals_file_load(void );
int  signal_update (char sName[100],int sVal);

#endif
