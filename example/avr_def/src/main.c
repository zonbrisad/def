/**
 *---------------------------------------------------------------------------
 * @brief    Test of def_avr.h
 *
 * @file     main.c
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2026-02-28
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

// Include ------------------------------------------------------------------

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "main.h"
#include "def_avr.h"

// Macros -------------------------------------------------------------------

#define UART_BAUD_RATE 57600

// Prototypes ---------------------------------------------------------------

void hw_init(void);

// Datatypes ----------------------------------------------------------------


// Variables ----------------------------------------------------------------

static FILE mystdout = FDEV_SETUP_STREAM((void*)uart_putc, NULL, _FDEV_SETUP_WRITE);

volatile uint16_t timer1Counter = 0;
// Code ---------------------------------------------------------------------

ISR(TIMER1_COMPA_vect) {
  TIMER1_RELOAD(0);
  printf("Timer %u\n", timer1Counter);
  timer1Counter++;
}    

ISR(ADC_vect) {
  
}

void hw_init(void) {
  stdout = &mystdout;
  uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
  sei();  // Enable all interrupts
} 

int main() {
  size_t i;

  hw_init();

  printf("\n\nStarting avr_def.h test\n\n");

  if (IS_POWER_ON_RESET()) {
    printf("Power-on reset detected\n");
  }
  if (IS_BROWN_OUT_RESET()) {
    printf("Brown-out reset detected\n");
  }
  if (IS_WATCH_DOG_RESET()) {
    printf("Watchdog reset detected\n");
  }
  if (IS_EXTERNAL_RESET()) {
    printf("External reset detected\n");
  }
  CLEAR_RESETS();
  printf("\n\n");

  ADC_ENABLE();
  ADC_REF_AVCC();
  ADC_PRESCALER_128();
  ADC_SET_MUX(0); // ADC0
  i = 0;
   while (i < 10) {
    ADC_START();
    ADC_WAIT_COMPLETION();
    uint16_t adcValue = ADC_VALUE();
    printf("ADC value: %u\n", adcValue);
    _delay_ms(100);
    i++;
  }
  ADC_DISSABLE();

  // Timer 1 (16 bit)
  TIMER1_CLK_PRES_1024(); // set prescaler to 1/1024
  TIMER1_OCA_SET(8192);   // 1 second at 16 MHz with 1024 prescaler
  TIMER1_OCA_IE();        // enable output compare A interrupt
  i = 0;
  while (timer1Counter < 10) {
    // Main loop can be empty, all work is done in interrupts
    sleep_mode(); // Put CPU to sleep to save power until next interrupt
  }
  TIMER1_OCA_ID();      // disable output compare A interrupt
  
  i = 0;
  while (1) {
    printf("Hello AVR def %d!\n", i);
    _delay_ms(1000);
    i++;
  }
}
