/** ***************************************************************************
 * Description       : I2C MRG keypad for RaspberryPi
 * Compiler          : GCC
 *
 * Copyright (C) 2013, MRG Systems Ltd.
 *
 *****************************************************************************/
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdint.h>

#define true 1
#define false 0

#define MAXKEYPAD 7

uint32_t keypad_init(void);
uint8_t write_keypad(uint8_t address, uint8_t data);
uint8_t read_keychar(uint8_t addr);