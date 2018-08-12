#include "convertors.h"

void writeUShortToBuf(byte buf[], unsigned short value) {
  buf[0] = (value >> 8) & 0xFF;
  buf[1] = value & 0xFF;
}

unsigned short uShortFromBuf(byte buf[]) {
  return (((unsigned short) (buf[0] << 8))
      + ((unsigned short) (buf[1])));
}

void writeULongToBuf(byte buf[], unsigned long value) {
  buf[0] = (value >> 24) & 0xFF;
  buf[1] = (value >> 16) & 0xFF;
  buf[2] = (value >> 8) & 0xFF;
  buf[3] = value & 0xFF;
}

unsigned long uLongFromBuf(byte buf[]) {
  return (((unsigned long) buf[0] << 24) 
      + ((unsigned long) buf[1] << 16) 
      + ((unsigned long) buf[2] << 8) 
      + ((unsigned long) buf[3]));
}
