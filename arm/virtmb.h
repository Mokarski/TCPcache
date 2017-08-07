#ifndef _VIRTMB_H
#define _VIRTMB_H

#define VirtDev 13      //numbers of virtual modbus devices
#define VirtDevRegs 10  //number of virtual mb registers
//   for cache read all devices registers //

struct Mb_Device {
 char Name[100];
 int  MB_Id;                //Modbus device ID
 int  MB_reg_counter[VirtDevRegs];   //temporary array for count number of regs if set yo 1 -> then used else unused
 int  MB_Registers[VirtDevRegs];     //Mb register values
 int Off;               //Signal not used if OFF = 1;
 int EventTrigger;          //signal has trigger state
 int For_Remove;        // mar for remove from tasks
       
       } Mb_Device_Cache;

struct Mb_Device Device_Array [VirtDev]; //number of virtual devices
       
       

int virt_mb_registers (int ID);
int virt_mb_filldev( char *sName, int mb_id, int mb_reg);
int virt_mb_devread(int ID, int reg_num);
void virt_mb_devlist (void);

#endif