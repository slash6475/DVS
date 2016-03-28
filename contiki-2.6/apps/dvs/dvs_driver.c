#include <math.h>

#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "cfs-coffee-arch.h"

#include "dvs.h"

char current_prog, selected_prog;
int fd = -1;
int speed = 1000;

/*---------------------------------------------------------------------------*/
void dvs_init(){
  DDRD  |= 1 << PIN4;
  PORTD ^= (1 << PIN4);
  // Format the File System if first boot
  int fa = cfs_open("/init", CFS_WRITE);
  cfs_write(fa, "init\0", 6);
  cfs_close(fa);


  neopixel_init();
  current_prog = selected_prog = 0;

  // Configuration of button interruption on PB4
  DDRB &= ~(1 << DDB4);
  PORTB |= (1 << PORTD4);
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT4);
  sei();
}
/*---------------------------------------------------------------------------*/
/* Button management:
1 released click: play next program
5 seconds of pressure: formatting eeprom memory
*/
/*---------------------------------------------------------------------------*/
char btn_state = 0;
extern char mpuInterrupt;
volatile uint8_t portbhistory = 0x00;

PROCESS(dvs_format, "DVS cfs format process");
PROCESS_THREAD(dvs_format, ev, data)
{
  static struct etimer periodic_timer;
  char i;

  PROCESS_BEGIN();
  etimer_set(&periodic_timer, CLOCK_SECOND*20);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

  if (btn_state == 1){
    printf("Formatting\n");
    for (i = 0; i < NUM_LEDS; i++)
      setPixel(i, 255,0,0);

    cfs_coffee_format();

    for (i = 0; i < NUM_LEDS; i++)
      setPixel(i, 0,255,0);
  }
  PROCESS_END();
}

ISR (PCINT0_vect)
{
    uint8_t changedbits;

    changedbits = PINB ^ portbhistory;
    portbhistory = PINB;

   // MPU 6050 Interruption
   if (changedbits & (1 << PB5))
    mpuInterrupt = 1;

    // External button
    else if (changedbits & (1 << PB4)) {
     // Start process to count long pression

     if (btn_state == 0)
      process_start(&dvs_format, NULL);

     if(++btn_state % 2) return;
     btn_state = 0;

     if(!(++selected_prog % (MAX_PROGRAMS + 1 ) )) selected_prog = 0;
     printf("DVS plays program %d\n", selected_prog);

     if (selected_prog == 4){
       if (process_is_running (&dvs_player))  process_exit(&dvs_player);
     	 process_start(&dvs_mpu6050, NULL );
     }
     else {
       if (process_is_running (&dvs_mpu6050))  process_exit (&dvs_mpu6050);
       process_start(&dvs_player, NULL);
       speed = 1000;
     }
    }
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
char prev = 0;
char mpu6050_show = 0;
PROCESS(dvs_mpu6050, "DVS dvs_mpu6050");
PROCESS_THREAD(dvs_mpu6050, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_EXITHANDLER(mpu6050_show = 0;)

  PROCESS_BEGIN();

  if (strncmp(data,"show", 4) == 0)
    mpu6050_show = 1;

  etimer_set(&periodic_timer, 0);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    // MPU6050 loop function
    if(mpu6050()){
        if (mpu6050_show == 1){
            printf("ypr %f %f %f \n", ypr[0]* 180/M_PI, ypr[1]* 180/M_PI, ypr[2]* 180/M_PI);
            printf("acc %f %f %f \n", acc[0], acc[1], acc[2]);
            printf("Euler angle %f %f %f \n", euler[0]* 180/M_PI, euler[1]* 180/M_PI, euler[2]* 180/M_PI);
          }

        if (ypr[1]* 180/M_PI < -75){
          if(prev != 0) {
            neopixel_init();
            setPixel(3, 255, 0, 0);
            setPixel(4, 255, 0, 0);
          }
          prev = 0;
        }
        else if (ypr[1]* 180/M_PI < -45){
          if(prev != 1) {
            neopixel_init();
            setPixel(2, 255, 0, 0);
            setPixel(5, 255, 0, 0);
          }
          prev = 1;
        }
        else if (ypr[1]* 180/M_PI < -25){
          if(prev != 2) {
            neopixel_init();
            setPixel(1, 255, 0, 0);
            setPixel(6, 255, 0, 0);
          }
          prev = 2;
        }
        else if (ypr[1]* 180/M_PI < 0){
          if(prev != 3) {
            neopixel_init();
            setPixel(0, 255, 0, 0);
            setPixel(7, 255, 0, 0);
          }
          prev = 3;
        }

        else  if (ypr[1]* 180/M_PI > 75){
          if(prev != 4) {
            neopixel_init();
            setPixel(11, 255, 0, 0);
            setPixel(12, 255, 0, 0);
          }
          prev = 4;
        }
        else  if (ypr[1]* 180/M_PI > 45){
          if(prev != 5) {
            neopixel_init();
            setPixel(10, 255, 0, 0);
            setPixel(13, 255, 0, 0);
          }
          prev = 5;
        }
        else  if (ypr[1]* 180/M_PI > 25){
          if(prev != 6) {
            neopixel_init();
            setPixel(9, 255, 0, 0);
            setPixel(14, 255, 0, 0);
          }
          prev = 6;
        }
        else if (ypr[1]* 180/M_PI > 0){
          if(prev != 6) {
            neopixel_init();
            setPixel(8, 255, 0, 0);
            setPixel(15, 255, 0, 0);
          }
          prev = 6;
        }
      }
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
