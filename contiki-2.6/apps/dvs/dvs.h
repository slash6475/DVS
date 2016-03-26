
#ifndef _NEOPIXEL_H
#define _NEOPIXEL_H

#define NUM_LEDS 16
#define MAX_PROGRAMS 4
#define SIZE_SEQUENCE 96

void dvs_read_program();

extern struct process neopixel_test_process;

void shell_neopixel_init();
void setup();
void process_sequence(char* sequence);

void neopixel_init();
void setPixel(int led, int r, int g, int b);
void shell_dvs_init(void);

void loop();

extern struct process dvs_mpu6050;
extern struct process shell_neopixel_process;

extern float ypr[3];
extern float euler[3];

#endif
