
#ifndef _NEOPIXEL_H
#define _NEOPIXEL_H

#define NUM_LEDS 16
#define MAX_PROGRAMS 4
#define SIZE_SEQUENCE 96

void setup();
void loop();

void neopixel_init();
void setPixel(int led, int r, int g, int b);

void shell_dvs_init(void);
void shell_neopixel_init();

void dvs_read_program();
void process_sequence(char* sequence);

extern struct process dvs_mpu6050;
extern struct process dvs_player;
extern struct process shell_neopixel_process;
extern struct process neopixel_test_process;

extern float ypr[3];
extern float acc[3];
extern float euler[3];

#endif
