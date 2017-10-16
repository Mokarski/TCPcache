//Virtual Modbus devices
#include <string.h>
#include <stdio.h>
#include "virtmb.h"
//#define VirtDev 30      //numbers of virtual modbus devices
//#define VirtDevRegs 10  //number of virtual mb registers
/*
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

*/

int bit_mask (int val, int bit, int mb_R) { // val = 1 or 0;  bit = bit position to set;  mb_R = previus readed register state

//register 16 bit
int  tab_reg[1];

 if ( val == 0 ) //if set to "0"
       {
         if (bit == 0)    tab_reg[0]=0xfffe & mb_R;
         if (bit == 1)    tab_reg[0]=0xfffd & mb_R;
         if (bit == 2)    tab_reg[0]=0xfffb & mb_R ;
         if (bit == 3)    tab_reg[0]=0xfff7 & mb_R ;
         if (bit == 4)    tab_reg[0]=0xffef & mb_R ;
         if (bit == 5)    tab_reg[0]=0xffdf & mb_R ;
         if (bit == 6)    tab_reg[0]=0xffbf & mb_R ;
         if (bit == 7)    tab_reg[0]=0xff7f & mb_R ;
         if (bit == 8)    tab_reg[0]=0xfeff & mb_R ;
         if (bit == 9)    tab_reg[0]=0xfdff & mb_R ;
         if (bit == 10)    tab_reg[0]=0xfbff & mb_R ;
         if (bit == 11)    tab_reg[0]=0xf7ff & mb_R ;
         if (bit == 12)    tab_reg[0]=0xefff & mb_R ;
         if (bit == 13)    tab_reg[0]=0xdfff & mb_R ;
         if (bit == 14)    tab_reg[0]=0xbfff & mb_R ;
         if (bit == 15)    tab_reg[0]=0x7fff & mb_R ;

         printf(">>Mb_write ZERRRO <<\n");
       }

       if (val) // if set to "1"
       {
         if (bit == 0)    tab_reg[0]=0x1 | mb_R;
         if (bit == 1)    tab_reg[0]=0x2 | mb_R;
         if (bit == 2)    tab_reg[0]=0x4 | mb_R ;
         if (bit == 3)    tab_reg[0]=0x8 | mb_R ;
         if (bit == 4)    tab_reg[0]=0x10| mb_R ;
         if (bit == 5)    tab_reg[0]=0x20 | mb_R ;
         if (bit == 6)    tab_reg[0]=0x40 | mb_R ;
         if (bit == 7)    tab_reg[0]=0x80 | mb_R ;
         if (bit == 8)    tab_reg[0]=0x100 | mb_R ;
         if (bit == 9)    tab_reg[0]=0x200 | mb_R ;
         if (bit == 10)    tab_reg[0]=0x400 | mb_R ;
         if (bit == 11)    tab_reg[0]=0x800 | mb_R ;
         if (bit == 12)    tab_reg[0]=0x1000 | mb_R ;
         if (bit == 13)    tab_reg[0]=0x2000 | mb_R ;
         if (bit == 14)    tab_reg[0]=0x4000 | mb_R ;
         if (bit == 15)    tab_reg[0]=0x8000 | mb_R ;
        }
return tab_reg[0];
}


int virt_mb_filldev( char *sName, int mb_id, int mb_reg, int inExState){ //fill unic dev to virt
	int i=0;
	//int reg_counter[VirtDevRegs]; //temporary array for count number of regs
	 printf("MB_FILL_DEVICES EX[%i] \n\r",inExState);
	
	if (inExState == 2) printf("MB_FILL_DEVICES EX[%i] \n\r",inExState);
	
	for (i=0; i < VirtDev; i++){
		if (Device_Array[i].Name[0] == 0) {
			break;
		}

		if(Device_Array[i].MB_Id == mb_id) {
			//Device_Array[i].ExState = inExState; //very bad idea
			Device_Array[i].MB_reg_counter[mb_reg] = 1; //mark for this DEV_ID register as used
			if (inExState == 1) Device_Array[i].Rd=1;  //mark device to read
			if (inExState == 2) {
				Device_Array[i].Wr=1;  //mark device  to write
				Device_Array[i].WR_MB_reg_counter[mb_reg] = 1; //mark for this DEV_ID register as used
				printf(">>>>>> WR Name[%s] EX=%i\n\r", sName,inExState);
			}
			
			return;
		}
	}
	
	if(i >= VirtDev) {
		return -1;
	}

	strcpy(Device_Array[i].Name, sName); //fill the name
	Device_Array[i].MB_Id = mb_id;
	//Device_Array[i].ExState = inExState; //very bad idea
	printf(">>>>>>Create device [%s] EX=%i\n\r",sName,inExState);
	if (inExState == 1) Device_Array[i].Rd=1;  //mark device to read
	if (inExState == 2) {
		Device_Array[i].Wr=1;  //mark device  to write
		Device_Array[i].WR_MB_reg_counter[mb_reg] = 1; //mark for this DEV_ID register as used
	}
	Device_Array[i].MB_reg_counter[mb_reg] = 1; //mark for this DEV_ID register as used  >>>>>>> fix missed zerro index!

	return 0;
}	
 
//read one virtual device register
int virt_mb_devread(int ID, int reg_num){
int result;
int z=0;
  for (z=0; z < VirtDev; z++)
  {
     if( Device_Array[z].MB_Id == ID ) {   //search Modbus device by ID
        result = Device_Array[z].MB_Registers[reg_num];

      }
  }


return result;
}
 
//WRITE one virtual device register
int virt_mb_devwrite(int ID, int reg_num, int val){
int result;
int z=0;
  for (z=0; z < VirtDev; z++)
  {
     if( Device_Array[z].MB_Id == ID ) {   //search Modbus device by ID
        Device_Array[z].MB_Registers[reg_num]=val;
        result++;

      }
  }


return result;
}
  
int virt_mb_registers (int ID){ //search and count used registers
	 int tmp=0; //counter of found used registers BUG?
         int z=0;
	   //calculate used registers
	  // printf("ID in reg calculate [%i]",ID);
	   for(z=0; z < VirtDevRegs; z++) { // numbers of virtual registers in struct Device_Array
	        //printf ("Used registers[%i] : [ %i ] \n\r",z, Device_Array[t].MB_Registers[z]);
                if ( Device_Array[ID].MB_reg_counter[z] > 0 ) {
                    //printf ("\n\r Count registers:  DEV_ID[%i]  Mark as Used registers num[%i] : Value [ %i ] ",ID,z, Device_Array[ID].MB_reg_counter[z]); //DEBUG
                    tmp++;  //May be bug? count from 2
                    //printf ("[Registers counter %i] \n\r ",tmp); //DEBUG
                    }
           }

return tmp;
}


void virt_mb_devlist (void) //print all virt devices
{
register int t;

 for (t=0; t < VirtDev; t++) {
    if(Device_Array[t].Name[0])
       {
	 int tmp=0; //counter of found used registers
         printf("[Virt_Device Name: %s] ", Device_Array[t].Name);
         printf("[Virt_MB ID: %i] \n\r", Device_Array[t].MB_Id);
         printf("[Ex: %i] \n\r", Device_Array[t].ExState);
         printf("[Rd: %i]  [Wr: %i]\n\r",Device_Array[t].Rd, Device_Array[t].Wr);
           int z=0;
	   //calculate used registers
	   for(z=0; z < VirtDevRegs; z++) { // numbers of virtual registers in struct Device_Array
	        //printf ("Used registers[%i] : [ %i ] \n\r",z, Device_Array[t].MB_Registers[z]);
                if ( Device_Array[t].MB_reg_counter[z] > 0 ) {
                     printf ("Mark as Used registers[%i] : [ %i ] \n\r",z, Device_Array[t].MB_reg_counter[z]);
                    // tmp++;
                    }
           }
		   
            for(z=0; z < VirtDevRegs ; z++) { // numbers of virtual registers in struct Device_Array
                printf ("USED Virt Register[%i] Value: [ %i ] \n\r",z, Device_Array[t].MB_Registers[z]);
               }
			   
			
                           
        }
 }
 
 printf("\n\n");
}
                             
