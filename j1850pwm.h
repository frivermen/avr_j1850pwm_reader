#ifndef __J1850PWM_H__
#define __J1850PWM_H__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// j1850 timings, 0.5us per unit
#define GAP 2*4
#define SOF 2*32
#define SOF_MIN (SOF-GAP)
#define SOF_MAX (SOF+GAP)
#define LOW 2*16
#define HIGH 2*8
#define MID ((LOW+HIGH)/2)
#define EOD 2*40

volatile uint8_t pulse_duration = 0;
volatile uint8_t data[12] = {0};
volatile uint8_t data_pointer = 0;
volatile uint8_t byte_buffer = 0;
volatile uint8_t bit_counter = 0b10000000;

ISR(INT0_vect) { // if int0 changing
  if (PIND & (1 << PD2)) { // if rising
    TCNT0 = 0;
  }
  else { // falling
    pulse_duration = TCNT0;
    TCNT0 = 0;
    if (pulse_duration >= SOF_MIN) {
      if (pulse_duration <= SOF_MAX) {
        data_pointer = 0;
        byte_buffer = 0;
        bit_counter = 0b10000000;
      }
    }
    else {
      if (pulse_duration < MID) {
        byte_buffer |= bit_counter;
      }
      bit_counter >>= 1;
      if (bit_counter == 0) {
        data[data_pointer++] = byte_buffer;
        bit_counter = 0b10000000;
        byte_buffer = 0;
      }
    }
  }
}

#endif // __J1850PWM_H__
