#ifndef	__TM1637_H__
#define	__TM1637_H__

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define dio_set()   PORTD |=  (1 << PD0)
#define dio_reset() PORTD &= ~(1 << PD0)
#define clk_set()   PORTD |=  (1 << PD1)
#define clk_reset() PORTD &= ~(1 << PD1)

// #define TM1637_SLOW
#ifdef TM1637_SLOW
#define delay_clk()  _delay_us(5)
#else
#define delay_clk()  
#endif // TM1637_SLOW

uint8_t tm1637_buffer[4] = {0};

void tm1637_start() {
  clk_set();
  dio_set();
  delay_clk();
  dio_reset();
}
  
void tm1637_stop() {
  clk_set();
  dio_reset();
  delay_clk();
  dio_set();
}
  
void tm1637_send_byte(uint8_t byte) {
  // byte
  for (uint8_t i = 0; i < 8; i++) {
    clk_reset();
    if (byte & (1 << i)) {
      dio_set();
    }
    else {
      dio_reset();
    }
    delay_clk();
    clk_set();
    delay_clk();
  }
  // ignore acknowlege
  clk_reset();
  delay_clk();
  dio_reset();
  clk_set();
  delay_clk();
  clk_reset();
  delay_clk();
}

void tm1637_init(uint8_t brightness) {
  // init byte
    // bit0 - always 1
    // bit1 - always 0
    // bit2 - 0 or nevermind
    // bit3 - 0 or nevermind
    // bit4 - 1 - enable / 0 - disable
    // bit5,6,7 - brightness
  // conf byte
    // bit0 - always 0
    // bit1 - always 1
    // bit2 - 0 or nevermind
    // bit3 - 0 or nevermind
    // bit4 - 1 - test mode / 0 - normal mode
    // bit5 - 1 - fix address / 0 - autoincrement address (from left to right)
    // bit6 - 1 - read keyboard / 0 - write display
    // bit7 - always 0
  tm1637_start();
  tm1637_send_byte(0b10001000 | brightness);
  tm1637_stop();
  tm1637_start();
  tm1637_send_byte(0b01000000);
  tm1637_stop();
}

void tm1637_char(uint8_t c, uint8_t pos) {
  tm1637_start();
  tm1637_send_byte(0xC0 + pos);
  tm1637_buffer[pos] &= 0b10000000;
  tm1637_buffer[pos] |= c;
  tm1637_send_byte(tm1637_buffer[pos]);
  tm1637_stop();
}

void tm1637_print(int16_t value, uint8_t pos) {
  const uint8_t segments[] = { // 0bDGFEDCBA
  	0x3F, // 0
  	0x06, // 1
  	0x5B, // 2
  	0x4F, // 3
  	0x66, // 4
  	0x6D, // 5
  	0x7D, // 6
  	0x07, // 7
  	0x7F, // 8
  	0x6F  // 9
  };
  for (uint8_t i = 0; i < 4; i++) tm1637_buffer[i] &= 0b10000000;

  uint8_t sign_flag = 1; // 1 positive, 0 negative
  if (value < 0) {
    sign_flag = 0;
    value = -value;
  }

  for (int8_t i = pos; i >= 0; i--) {
    if (value) {
      tm1637_buffer[i] |= segments[value % 10];
      value /= 10;
    }
    else {
      if (i == pos) {
        tm1637_buffer[i] |= segments[0]; // '0'
      }
      if (sign_flag == 0) {
        tm1637_buffer[i] |= 0b01000000; // '-'
      }
      break;
    }
  }
   
  tm1637_start();
  tm1637_send_byte(0xC0); // left digit address
  for (uint8_t l = 0; l < 4; l++) {
    tm1637_send_byte(tm1637_buffer[l]);
  }
  tm1637_stop();
}

#endif	// __TM1637_H__
