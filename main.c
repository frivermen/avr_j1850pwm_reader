#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "j1850pwm.h"
#include "tm1637.h"
// #include "m328_eeprom.h"
// #include "m328_uart.h"

#define MAX_BRIGHTNESS 7
#define MIN_BRIGHTNESS 2

volatile uint32_t millis = 0;
void delay(uint32_t ms) {
  uint32_t now = millis;
  while ((millis - now) < ms);
}

volatile const uint8_t spar[4] = {0xA1, 0x29, 0x10, 0x02};
volatile const uint8_t tmar[4] = {0x81, 0x49, 0x10, 0x10};
volatile uint8_t speed = 0;
volatile int8_t temp = -40;

uint8_t backlight = 0;

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

  backlight = (PIND & (1 << PD4));
  if (backlight) tm1637_init(MIN_BRIGHTNESS);
  else tm1637_init(MAX_BRIGHTNESS);

  uint32_t disp_upd_counter = 0;

  sei(); // enable interrupts

  while (1) {
  // display block
  // print temp and speed
    if (millis - disp_upd_counter > 50) {
      disp_upd_counter = millis;
      if (speed < 20) {
        tm1637_print(temp, 2);
        tm1637_char(0b00111001, 3); // 'C'
      }
      else {
        tm1637_print(speed, 3);
      }
    }
  // GPIO block
  // set brightness
    if ((PIND & (1 << PD4)) && !backlight) { // backlight turn on
      backlight = 1;
      tm1637_init(MIN_BRIGHTNESS);
    }
    if (!(PIND & (1 << PD4)) && backlight) { // backlight turn off
      backlight = 0;
      tm1637_init(MAX_BRIGHTNESS);
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
    data_pointer = 0;
  }
}

ISR (TIMER2_COMPA_vect) { // timer2 overflow interrupt
  millis++;
}

ISR(INT1_vect) { // if int1 falling
  tm1637_init(MAX_BRIGHTNESS);
  tm1637_char(0b01000000, 0); // '-'
  tm1637_char(0b01000000, 1);
  tm1637_char(0b01000000, 2);
  tm1637_char(0b01000000, 3);
  while (!(PIND & (1 << PD3))); // while ingition off
  if (PIND & (1 << PD4)) tm1637_init(MIN_BRIGHTNESS);
}
