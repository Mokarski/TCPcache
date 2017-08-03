//Virtual Modbus devices
#include <string.h>
#include <stdio.h>
#define VirtDev 30      //numbers of virtual modbus devices
#define VirtDevRegs 50  //number of virtual mb registers

//   for cache read all devices registers //
struct Mb_Device {
 char Name[100];
 int  MB_Id;                           //Modbus device ID
 int  MB_Registers[VirtDevRegs];       //Mb register number
 int Off;                              //Signal not used if OFF = 1;
 int EventTrigger;                     //signal has trigger state
 int For_Remove;                       // mar for remove from tasks
 
} Mb_Device_Cache;
   
struct Mb_Device Device_Array [VirtDev]; //number of virtual devices


int virt_mb_filldev( char sName[100], int mb_id){ //fill unic dev to virt
	int i=0;
	int uniq_test=0;
	for (i=0; i < VirtDev; i++){
		if ( Device_Array[i].MB_Id == mb_id ) {
			uniq_test=0;
			break;
		} else uniq_test++; //increment checked position
	}
	
	if ( uniq_test > 0 ){ // if device is unique
		for (i=0; i< VirtDev; i++){
			if ( strlen (Device_Array[i].Name) < 2){			
		        //strcpy(Device_Array[i].Name,sName); //fill the name
		          strncpy(Device_Array[i].Name,sName,14 ); //fill the name
			  Device_Array[i].MB_Id = mb_id;				
			  break; //if filled Name and ID then break the cycle
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

 for (t=0; t<30; ++t) {
    if(Device_Array[t].Name[0])
       {
         printf("Virt Device Name: %s\n", Device_Array[t].Name);
         printf("Virt MB ID: %i\n", Device_Array[t].MB_Id);
           int z=0;
            for(z=0; z<VirtDevRegs; z++) { //50 numbers of virtual registers in struct Device_Array
                printf ("Virt Register[%i] : [ %i ]\n",z, Device_Array[t].MB_Registers[z]);
               }
                           
        }
 }
 printf("\n\n");
}
                             
                                