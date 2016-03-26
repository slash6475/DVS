
#include <stdlib.h>
#include <avr/interrupt.h>

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

#include "rest.h"
#include "http-common.h"

#include "dev/serial-line.h"
#include "shell.h"
#include "serial-shell.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "cfs-coffee-arch.h"

#include "dvs.h"
#include "dvs.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(dvs_process, "Devil Stick");
AUTOSTART_PROCESSES(&dvs_process);
/*---------------------------------------------------------------------------*/
RESOURCE(leds, METHOD_POST, "leds");
// TODO defining a root ressource / describing the WEB API

/*
 * Sequence are defined by hexadecimal value on 2 digit for each color (r,g,b) for each led
 */
 PROCESS(dvs_coffee_save_process, "Coffee program storing");
 PROCESS_THREAD(dvs_coffee_save_process, ev, data)
 {
  char buf[COFFEE_NAME_LENGTH];

   PROCESS_BEGIN();

    // Extract the name of the file
    char *tmp  = strtok ((char*)data, ":");
    strncpy(buf, tmp, COFFEE_NAME_LENGTH);
    if(atoi(buf) < 0 || atoi(buf) >= MAX_PROGRAMS){
        printf("DVS program number invalid for storage: %d(0,%d)", atoi(buf), MAX_PROGRAMS);
        PROCESS_EXIT();
    }
    printf("DVS store new program: %s:", buf);
    // Open the file and store the sequences
    int fa = cfs_open((const char*) buf, CFS_WRITE + CFS_APPEND);
    tmp = strtok (NULL, ":");
    int r = cfs_write(fa, tmp, strlen(tmp));
    cfs_write(fa, 'X', 1); // File termination for Coffee
    if (r<0) printf("CFS Error !\n");
    cfs_close(fa);
    printf(" done.\n");
    PROCESS_END();
 }

void
leds_handler(REQUEST* request, RESPONSE* response)
{
  char tmp[5];
  sprintf(tmp,"done");


  if (!strncmp((char*)request->payload,"del:",4)){
    printf("METHOD GET %s \n",(char*) request->payload+4);
    cfs_remove ((char*)request->payload+4);
  }

  else if (!strncmp((char*)request->payload,"add:",4)){
    printf("METHOD POST\n");
    process_start(&dvs_coffee_save_process, (char*)request->payload+4);
  }

  http_set_res_header(response, "Access-Control-Allow-Origin", "*", 0);
  http_set_res_header(response, "Access-Control-Allow-Methods", "GET,PUT,POST,DELETE", 0);

  rest_set_header_content_type(response, TEXT_PLAIN);
  rest_set_response_payload(response, tmp, strlen(tmp));

}

/*---------------------------------------------------------------------------*/
/* */
char current_prog, selected_prog;
int fd = -1;
int speed = 1000;
/*---------------------------------------------------------------------------*/

void process_sequence(char* sequence) {
  int i=0;
  unsigned char val[3];
  char tmp[3];
  for (i=0; i<strlen(sequence); i+=2){
    if((i/2)%3 == 0 && i > 5){
      setPixel((i/2)/3-1, val[0], val[1], val[2]);
    }
    tmp[0] = sequence[i];
    tmp[1] = sequence[i+1];
    tmp[2] = '\0';
    val[(i/2)%3] = (unsigned char)strtol(tmp, NULL, 16);
  }
  setPixel((i/2)/3-1, val[0], val[1], val[2]);
}

PROCESS(dvs_player, "DVS Player process");
PROCESS_THREAD(dvs_player, ev, data)
{
  static struct etimer periodic_timer;
  char seq[97];
  int j;
  char k;

  PROCESS_BEGIN();

  printf("Player initiliazed\n");
  etimer_set(&periodic_timer, (CLOCK_SECOND/1000)*speed);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    // No program file loaded, try to load current one
    if (fd < 0 || current_prog != selected_prog) {
      current_prog = selected_prog;
      cfs_close(fd);
      sprintf(seq,"%d", current_prog);
      fd = cfs_open(seq, CFS_READ);
    }
    // Read next sequence of loaded file
    if(fd > -1) {
      k = 0;
      do {
        // Read the sequence speed
        cfs_read(fd, seq, 6);
        seq[6] = '\0';
        speed = atoi(seq);
        if (speed < 100) speed = 100;

        // Read the sequence
        j = cfs_read(fd, seq, SIZE_SEQUENCE);
        seq[j] = '\0';

        // If end of file, try at beginning
        if (strlen(seq) < SIZE_SEQUENCE)
          cfs_seek (fd, 0, CFS_SEEK_SET);
      }
      while (strlen(seq) < SIZE_SEQUENCE && k++ == 0);

      //printf("seq:%s\n", seq);
      process_sequence(seq);
      cfs_read(fd, seq, 1); // Skip end terminatio caracter
    }
  etimer_set(&periodic_timer, (int)(((float)CLOCK_SECOND/(float)1000)*speed));
  }
    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Button management:
1 released click: play next program
5 seconds of pressure: formatting eeprom memory
*/
/*---------------------------------------------------------------------------*/
char btn_state = 0;
PROCESS(dvs_format, "DVS cfs format process");
PROCESS_THREAD(dvs_format, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();
  etimer_set(&periodic_timer, CLOCK_SECOND*5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

  if (btn_state == 1){
    printf("Formatting\n");
    cfs_coffee_format();
  }
  PROCESS_END();
}

extern char mpuInterrupt;
ISR (PCINT0_vect)
{
   // MPU 6050 Interruption
   if (PIND & (1<<PD5))
    mpuInterrupt = 1;

    // External button
    else {
     // Start process to count long pression
     if (btn_state == 0)
      process_start(&dvs_format, NULL);

     if(++btn_state % 2) return;
     btn_state = 0;

     if(!(++selected_prog % MAX_PROGRAMS)) selected_prog = 0;
     printf("DVS plays program %d\n", selected_prog);
     speed = 1000;
    }
}



/*---------------------------------------------------------------------------*/
void dvs_init(){
  // Format the File System if first boot
//  int fa = cfs_open("/init", CFS_WRITE);
//  cfs_write(fa, "init\0", 6);
//  cfs_close(fa);

  neopixel_init();
  current_prog = selected_prog = 0;

  // Configuration of button interruption on PB4
  DDRB &= ~(1 << DDB4);
  PORTB |= (1 << PORTD4);
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT4);
  sei();
}

PROCESS_THREAD(dvs_process, ev, data)
{

  DDRD  |= 1 << PIN4;
  PORTD |= ~(1 << PIN4);

  PROCESS_BEGIN();
  printf("F_CPU: %lu\n", F_CPU);
  setup();

  serial_line_init();
  serial_shell_init();
  shell_ps_init();
  shell_dvs_init();

  rest_init();
  printf("Webservices started on %d\n", HTTP_PORT);
  rest_activate_resource(&resource_leds);

  dvs_init();

  process_start(&neopixel_test_process, NULL);
  process_start(&dvs_player, NULL);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
