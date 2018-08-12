#ifndef CONVERTORS_H
#define CONVERTORS_H

#include <Arduino.h>

void writeUShortToBuf(byte * buf, unsigned short value);

unsigned short uShortFromBuf(byte * buf);

void writeULongToBuf(byte * buf, unsigned long value);

unsigned long uLongFromBuf(byte * buf);

#endif
