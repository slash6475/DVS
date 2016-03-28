#include "contiki.h"
#include "shell.h"
#include "dvs.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_neopixel_process, "neopixel");
SHELL_COMMAND(neopixel_command,
	      "neopixel",
	      "neopixel [id] [r] [g] [b]",
	      &shell_neopixel_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_neopixel_process, ev, data)
{
  int id;
  char r,g,b;
  const char *nextptr;

  PROCESS_BEGIN();
  nextptr = data;

  if (!strncmp(nextptr, "test", 4)){
    process_start(&neopixel_test_process, NULL);
  }
  else {
    id = shell_strtolong(data, &nextptr);
    r = shell_strtolong(nextptr, &nextptr);
    g = shell_strtolong(nextptr, &nextptr);
    b = shell_strtolong(nextptr, &nextptr);
    setPixel(id, r, g, b);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(shell_mpu6050_process, "mpu6050");
SHELL_COMMAND(mpu6050_command,
	      "mpu6050",
	      "mpu6050 start | stop",
	      &shell_mpu6050_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_mpu6050_process, ev, data)
{
  const char *nextptr;
	const char show[] = "show";

  PROCESS_BEGIN();
  nextptr = data;

  if (!strncmp(nextptr, "start", 5))
  	process_start(&dvs_mpu6050, show);

  else if (!strncmp(nextptr, "stop", 4))
		process_exit (&dvs_mpu6050);

  PROCESS_END();
}



/*---------------------------------------------------------------------------*/
void
shell_dvs_init(void)
{
  shell_register_command(&neopixel_command);
	shell_register_command(&mpu6050_command);
}
/*---------------------------------------------------------------------------*/
