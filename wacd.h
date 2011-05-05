/* Williams Analog Clock Daemon constants. */
/* Copyright 2011 Antal Spector-Zabusky and Andrew Lorenzen */
#ifndef WACD_H
#define WACD_H

#define WACD_SET      0x01
#define WACD_GET      0x02
#define WACD_MOMENT   0x03
#define WACD_GO       0x04
#define WACD_STOP     0x05
#define WACD_GOTO     0x06
#define WACD_FINISH   0x07
#define WACD_SHUTDOWN 0x08

#define WACD_PORT 10010

#define WACD_SECONDS_PER_CLOCK (60*60*12)

#define WACD_MIN_MOMENT 75

#define WACD_STATUS_OK  0
#define WACD_STATUS_ERR -1

#define WACD_PHYSICAL_PORT 888

#endif
