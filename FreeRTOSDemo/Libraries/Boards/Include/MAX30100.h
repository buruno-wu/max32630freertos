/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAX30100_H
#define MAX30100_H

#include <stdint.h>
#include "MAX30100_Registers.h"


//#define I2C_BUS_SPEED               400000UL

#define MAX30102_ADDR      0x57


extern uint32_t rawIRValue;
extern uint32_t rawRedValue;


uint8_t readRegister(uint8_t address);
void writeRegister(uint8_t address, uint8_t data);
void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length);
void readFifoData(void);
void get_tempture(void);
uint8_t Write_One_Byte(uint8_t addr,uint8_t data);
uint8_t Read_One_Byte(uint8_t address,uint8_t *result);//¶Á×Ö½Ú
uint8_t Buff_Read(uint8_t address,uint8_t *buf, uint8_t len);

#endif
