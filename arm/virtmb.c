//Virtual Modbus devices
#include <string.h>
#include <stdio.h>

#define VirtDev 30      //numbers of virtual modbus devices
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


int virt_mb_filldev( char *sName, int mb_id, int mb_reg){ //fill unic dev to virt
	int i=0;
	int uniq_test=0;
	//int reg_counter[VirtDevRegs]; //temporary array for count number of regs
	for (i=0; i< VirtDev; i++){
		if ( Device_Array[i].MB_Id == mb_id ) {
			uniq_test=0;
			
			Device_Array[i].MB_reg_counter[mb_reg] = 1; //mark for this DEV_ID register as used
			
			break;
		} else uniq_test++; //increment checked position
	}
	
	if ( uniq_test > 0 ){ // if device is unique
		for (i=0; i< VirtDev; i++){
			
			if ( strlen (Device_Array[i].Name) < 2){			
		                strcpy(Device_Array[i].Name,sName); //fill the name
				Device_Array[i].MB_Id = mb_id;
				
				break; // device copied to array DONE
			}
		}
	}
	
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
  

void virt_mb_devlist (void) //print all virt devices
{
register int t;

 for (t=0; t < VirtDev; t++) {
    if(Device_Array[t].Name[0])
       {
	 int tmp=0; //counter of found used registers
         printf("[Virt_Device Name: %s] ", Device_Array[t].Name);
         printf("[Virt_MB ID: %i] \n\r", Device_Array[t].MB_Id);
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
                             
