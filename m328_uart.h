#ifndef __UART_H__
#define __UART_H__

#ifndef F_CPU
#define F_CPU 16000000UL
#endif // F_CPU
 
#define UART_BOUDRATE 1000000
#define UART_SPEED ((F_CPU / (8L * UART_BOUDRATE)) - 1)

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

void uart_init() {
  UBRR0H = UART_SPEED >> 4;
  UBRR0L = UART_SPEED & 15;
  UCSR0A = (1 << U2X0);
  UCSR0B = (1 << TXEN0); // enable transmite
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8bit mode
}

void uart_srt(char* str) {
  uint8_t i = 0;
  while (str[i]) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = str[i++];
  }
}

void uart_u32(uint32_t num, uint8_t base) {
  char str[12] = {0};
  itoa(num, str, base);
  uart_srt(str);
}

void uart_hex(uint8_t num) {
  uint8_t hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7','8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = hex[num >> 4];
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = hex[num & 15];
}


#endif // __UART_H__
