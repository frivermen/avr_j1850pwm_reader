#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "j1850pwm.h"
#include "tm1637.h"
#include "m328_eeprom.h"

#define MAX_BRIGHTNESS 7
#define MIN_BRIGHTNESS 2

volatile uint32_t millis = 0;
void delay(uint32_t ms) {
  uint32_t now = millis;
  while ((millis - now) < ms);
}

volatile const uint8_t spar[4] = {0xA1, 0x29, 0x10, 0x02};
volatile const uint8_t tmar[4] = {0x81, 0x49, 0x10, 0x10};
volatile const uint8_t tpar[4] = {0x81, 0x1B, 0x10, 0x25};
volatile uint8_t speed = 0;
volatile int8_t temp = -40;
volatile uint8_t tp = 0;

volatile uint32_t msec = 9999UL * 3600UL + 3599;
volatile uint16_t msec_address = 0;

int main() {
  // setup timer0 to 40us
  TCCR0A |= (1 << WGM01); // set tim0 to CTC mode
  TCCR0B |= (1 << CS01); // set div/8
  TIMSK0 |= (1 << OCIE0A);    // Set the ISR COMPA vect
  OCR0A = EOD; // when 40us, interrupt catched

  // setup timer2 to 1ms
  TCCR2A |= (1 << WGM21); // set tim0 to CTC mode
  TCCR2B |= (1 << CS22); // set div/64
  TIMSK2 |= (1 << OCIE2A);    // Set the ISR COMPA vect
  OCR2A = 250; // when 1ms, interrupt catched

  // setup int0 to change (j1850)
  EICRA |= (1 << ISC00); // logic change on int0
  EIMSK |= (1 << INT0); // turn on int0

  // setup int0 to falling (ignition)
  EICRA |= (1 << ISC11); // logic falling on int1
  EIMSK |= (1 << INT1); // turn on int1

  // setup pins
  DDRD &= ~(1 << PD2); // set PD2 as INPUT  // j1850+
  DDRD &= ~(1 << PD3); // set PD3 as INPUT  // ingnition on monitoring
  DDRD &= ~(1 << PD4); // set PD4 as INPUT  // backlight monitoring
  DDRD &= ~(1 << PD5); // set PD5 as INPUT  // oil lamp monitoring
  DDRD |=  (1 << PD0); // set PD0 as OUTPUT // tm1637 dio
  DDRD |=  (1 << PD1); // set PD1 as OUTPUT // tm1637 clk
  DDRB |=  (1 << PB3); // set PB3 as OUTPUT // debug

  sei(); // enable interrupts

  // read variable for count moto-hourse
  for (uint16_t i = 0; i < 1024; i++) { // search msec in eeprom
    if (eeprom_read(i) == 0x00) { // if find
      msec_address = i;
      for (uint8_t a = 1; a <= 4; a++) { // get msec
        msec |= (uint32_t) eeprom_read(msec_address + a) << 8 * (4 - a); 
      }
      break;
    }  
  }

  uint8_t backlight = (PIND & (1 << PD4));
  if (backlight) tm1637_init(MIN_BRIGHTNESS);
  else tm1637_init(MAX_BRIGHTNESS);

  tm1637_print(msec / 3600, 0, 4); // print moto-hours 
  delay(2000);
  tm1637_print(msec % 3600, 0, 4); 
  delay(1000);

  uint32_t disp_upd_counter = 0;
  uint32_t mhrs_counter = 0;

  while (1) {
  // display update
    if (millis - disp_upd_counter > 50) {
      disp_upd_counter = millis;
      if (speed < 20) {
        tm1637_print(temp, 0, 3);
        tm1637_char(0b00111001, 3);
      }
      else {
        tm1637_print(speed, 0, 4);
      }
    }
  // set brightness
    if ((PIND & (1 << PD4)) && !backlight) { // backlight turn on
      backlight = 1;
      tm1637_init(MIN_BRIGHTNESS);
    }
    if (!(PIND & (1 << PD4)) && backlight) { // backlight turn off
      backlight = 0;
      tm1637_init(MAX_BRIGHTNESS);
    }
  // count mhrs
    if ((PIND & (1 << PD5)) && (millis - mhrs_counter > 1000)) {
      mhrs_counter = millis;
      if (++msec > (19999UL * 3600)) msec = (10000UL * 3600);
    }
  }
}

uint8_t compare_arrays(uint8_t* a, uint8_t* b) {
  for (uint8_t i = 0; i < 4; i++) {
    if (a[i] != b[i]) return 0;
  }
  return 1;
}

ISR (TIMER0_COMPA_vect) { // timer0 overflow interrupt
  if (data_pointer) {
    if (compare_arrays((uint8_t*) data, (uint8_t*) spar)) {
      speed = ((data[4] << 8) | data[5]) >> 7;
    }
    if (compare_arrays((uint8_t*) data, (uint8_t*) tmar)) {
      temp = data[4] - 40;
    }
    if (compare_arrays((uint8_t*) data, (uint8_t*) tpar)) {
      tp = data[8];
    }
    data_pointer = 0;
  }
}

ISR (TIMER2_COMPA_vect) { // timer2 overflow interrupt
  millis++;
}

ISR(INT1_vect) { // if int1 falling
  tm1637_init(MAX_BRIGHTNESS);
  eeprom_write(msec_address++, 0xFF);
  eeprom_write(msec_address, 0x00);
  for (uint8_t i = 1; i <= 4; i++) {
    eeprom_write(msec_address + i, (msec >> 8 * (4 - i)) & 0xFF);
  }
  tm1637_print(msec/3600, 0, 4); // print moto-hours 
  while (!(PIND & (1 << PD3))); // while ingition off
  if (PIND & (1 << PD4)) tm1637_init(MIN_BRIGHTNESS);
}
