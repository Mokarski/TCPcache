// speed test functions and timer functions
#include <sys/time.h>


struct timeval tv1,tv2,dtv;
struct timezone tz;

void speedtest_start() {
gettimeofday(&tv1,&tz);
}

long speedtest_stop(){
gettimeofday(&tv2,&tz);
dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
   if(dtv.tv_usec<0)
      {
           dtv.tv_sec--; dtv.tv_usec+=1000000;
              }
               return dtv.tv_sec*1000+dtv.tv_usec/1000;
}
               
struct timeval tvr1,tvr2,dtvr;
struct timezone tzr;
               
void timer_start() {
               gettimeofday(&tvr1,&tzr);
               }
               
long timer_stop(){
               gettimeofday(&tvr2,&tzr);
               dtvr.tv_sec= tvr2.tv_sec -tvr1.tv_sec;
               dtvr.tv_usec=tvr2.tv_usec-tvr1.tv_usec;
                  if(dtvr.tv_usec<0)
                     {
                          dtvr.tv_sec--; dtvr.tv_usec+=1000000;
                             }
                              return dtvr.tv_sec*1000+dtvr.tv_usec/1000;
 }
                              
                              