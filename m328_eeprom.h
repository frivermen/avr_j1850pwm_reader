#ifndef __M328_EEPROM_H__
#define __M328_EEPROM_H__

#include <avr/io.h>
#include <stdint.h>

uint8_t eeprom_read(uint16_t address) {
  while(EECR & (1 << EEPE));  // wait eeprom ready
  EEAR = address;
  EECR |= (1 << EERE); // read EEPROM
  return EEDR; // return byte
}

void eeprom_write(uint16_t address, uint8_t byte) {
  if (eeprom_read(address) == byte) return; // not rewrite same data
  while(EECR & (1 << EEPE));  // wait eeprom ready
  EEAR = address;
  EEDR = byte;
  EECR |= (1 << EEMPE); // allow write
  EECR |= (1 << EEPE); // write byte
}

#endif // __M328_EEPROM_H__
