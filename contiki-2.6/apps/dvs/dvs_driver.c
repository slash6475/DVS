#include <math.h>

#include "contiki.h"
#include "dvs.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

PROCESS(dvs_mpu6050, "DVS dvs_mpu6050");
PROCESS_THREAD(dvs_mpu6050, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();
  etimer_set(&periodic_timer, 1);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    // MPU6050 Arduino loop function
    if(mpu6050())
      printf("ypr %f %f %f \n", ypr[0]* 180/M_PI, ypr[1]* 180/M_PI, ypr[2]* 180/M_PI);
    //printf("Euler angle %f %f %f \n", euler[0]* 180/M_PI, euler[1]* 180/M_PI, euler[2]* 180/M_PI);
    }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** Process to start a set of sequences to test the leds
 * Each led successively turn on from red, green and blue
 */

PROCESS(neopixel_test_process, "Neopixel test");
PROCESS_THREAD(neopixel_test_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned char r,g,b;
  static short int count;

  PROCESS_BEGIN();
  r = 255;
  g = b = 0;
  count = 0;

  printf("Neopixel Test Initialized\n");
  etimer_set(&periodic_timer, CLOCK_SECOND/8);
  while(1) {
    if (count >= NUM_LEDS*4)
      PROCESS_EXIT();

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    setPixel(count % NUM_LEDS, r, g, b);
    count ++;

    if (count % NUM_LEDS == 0) {
      if (r == 255){
        r = b = 0;
        g = 255;
      }
      else if (g == 255){
        g = r = 0;
        b = 255;
      }
      else r = g = b = 0;
    }
  }
    PROCESS_END();
}



void neopixel_init(){
  int i;
  for (i=0; i < NUM_LEDS; i++)
    setPixel(i, 0,0,0);
}
