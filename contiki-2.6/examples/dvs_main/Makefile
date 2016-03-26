all: dvs-main

CONTIKI=../..
COFFEE_FILES=2

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

APPS += serial-shell
APPS += dvs
APPS += rest-http

# Import printf library for float variable
CFLAGS += -DWITH_HTTP -Wl,-u,vfprintf
LDFLAGS +=  -Wl,-u,vfprintf -lprintf_flt -lm

WITH_UIP6=1
UIP_CONF_IPV6=1
UIP_CONF_RPL=0

#CFLAGS+= -DUIP_CONF_IPV6_RPL

#SMALL=1

include $(CONTIKI)/Makefile.include

# sudo avrdude -p atmega128rfa1 -c jtag2isp -P usb -U lfuse:w:0x62:m -U hfuse:w:0x0x99:m -U efuse 0xfe:m
# avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature dvs-main.avr-atmega128rfa1 dvs-main.hex
# sudo avrdude -p atmega128rfa1 -c jtag2isp -P usb -Uflash:w:dvs-main.hex

# avr-size -C --mcu=atmega128rfa1
