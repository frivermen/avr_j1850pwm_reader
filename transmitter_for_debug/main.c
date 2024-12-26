#include <stdint.h>
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define send(a,b) PORTB = 0xFF;_delay_us(a);PORTB = 0x00;_delay_us(b);

void u(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    if (b & 0b10000000) {
      send(8,16);
    }
    else {
      send(16,8);
    }
    b <<= 1;
  }
}

int main() {
  DDRB |= (1 << PB5); // set PB5 as OUTPUT
  uint8_t tp = 0;
  uint8_t tm = 0;
  uint8_t sp = 0;
  while (1) {
    send(32,16); // start of frame
    // 81 1B 10 25 0C 84 00 CB 00 95 // RPM, SFT, TP
    u(0x81);
    u(0x1B);
    u(0x10);
    u(0x25);
    u(0x0C);
    u(0xCD);
    u(0x0F);
    u(0xDF);
    u(tp++);
    u(0x7B);
    _delay_us(72); // just pause between packets
    send(32,16); // start of frame
    // 81 49 10 10 78 D1 // TEMP
    u(0x81);
    u(0x49);
    u(0x10);
    u(0x10);
    u(tm++);
    u(0x7B);
    _delay_us(72); // just pause between packets
    send(32,16); // start of frame
    // A1 29 10 02 00 00 C2 // VSS
    u(0xA1);
    u(0x29);
    u(0x10);
    u(0x02);
    u(sp >> 4);
    u(sp & 15);
    u(0x7B);
    sp += 4;
    _delay_ms(1000);
  }
}
