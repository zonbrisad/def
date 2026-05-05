/**
 *---------------------------------------------------------------------------
 * @brief    Test program for HD44780 LCD's
 *
 * @file     main.c
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2026-03-02
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

/*
 * Hardware description:
 * - AVR Atmega 128
 
 *
 * HD44780 LCD:
 * - RS = PA5
 * - RW = PA6
 * - E  = PA7
 * - D4 = PF4
 * - D5 = PF5
 * - D6 = PF6
 * - D7 = PF7
 * 
 * Pot = PF0 (ADC0) Backlight controll
 * Bl  = PE3 (OC3A) PWM on timer 3 (LCD backlight)
 * 
 * I2C bus:
 *   SCL = PD0
 *   SDA = PD1
 */



#define LED_TEST B,5

// Include ------------------------------------------------------------------

#include "main.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "LEF/LEF.h"
#include "def.h"
#include "def_avr.h"
#include "uart.h"
#include "lcd.h"
#include "i2c_sw.h"
#include "light_ws2812.h"


// Macros -------------------------------------------------------------------

#define LCD_BACKLIGHT_PIN E, 3
#define POT_ADC 0

static const uint32_t UART_BAUD_RATE = 57600;

typedef enum {
    EVENT_Timer1 = 0,
    EVENT_Timer2,
    EVENT_TimerA,
    EVENT_Button1,
    EVENT_Button2,
    EVENT_Button3,
    EVENT_BL_POT
} Events;


LEF_Timer timer1;
LEF_Timer timer2;
LEF_Timer timer_a;
LEF_Button button1;
LEF_Button button2;
LEF_Led led_test;
LEF_Pot bl_pot;

bool evOn = false;

static FILE mystdout =
    FDEV_SETUP_STREAM((void*)uart1_putc, NULL, _FDEV_SETUP_WRITE);

#define LCD_RS_PIN A, 5    /**< pin for RS line         */
#define LCD_RW_PIN A, 6    /**< pin for Read/Write line */
#define LCD_E_PIN A, 7     /**< pin for Enable line     */
#define LCD_DATA4_PIN F, 4 /**< pin for 4bit data bit 0  */
#define LCD_DATA5_PIN F, 5 /**< pin for 4bit data bit 1  */
#define LCD_DATA6_PIN F, 6 /**< pin for 4bit data bit 2  */
#define LCD_DATA7_PIN F, 7 /**< pin for 4bit data bit 3  */

static uint16_t lcd_gpio_callback(HD44780_MSG msg, uint16_t data_arg) {
    uint16_t result = 0;
    switch (msg) {
        case HD44780_MSG_INIT:
            gpio_init(LCD_E_PIN, true, false);
            gpio_init(LCD_RW_PIN, true, false);
            gpio_init(LCD_RS_PIN, true, false);
            break;
        case HD44780_MSG_GPIO_DATA_DIRECTION:
            gpio_direction(LCD_DATA4_PIN, data_arg);
            gpio_direction(LCD_DATA5_PIN, data_arg);
            gpio_direction(LCD_DATA6_PIN, data_arg);
            gpio_direction(LCD_DATA7_PIN, data_arg);
            break;
        case HD44780_MSG_GPIO_DATA_READ:
            if (gpio_read(LCD_DATA4_PIN)) result |= 0x01;
            if (gpio_read(LCD_DATA5_PIN)) result |= 0x02;
            if (gpio_read(LCD_DATA6_PIN)) result |= 0x04;
            if (gpio_read(LCD_DATA7_PIN)) result |= 0x08;
            break;
        case HD44780_MSG_GPIO_DATA_WRITE:
            gpio_write(LCD_DATA7_PIN, data_arg & 0x80);
            gpio_write(LCD_DATA6_PIN, data_arg & 0x40);
            gpio_write(LCD_DATA5_PIN, data_arg & 0x20);
            gpio_write(LCD_DATA4_PIN, data_arg & 0x10);
            break;
        case HD44780_MSG_GPIO_E:
            gpio_write(LCD_E_PIN, data_arg);
            break;
        case HD44780_MSG_GPIO_E_TOGGLE:
            gpio_write(LCD_E_PIN, 1);
            _delay_us(HD44780_DELAY_ENABLE_PULSE);
            gpio_write(LCD_E_PIN, 0);
            break;
        case HD44780_MSG_GPIO_RW:
            gpio_write(LCD_RW_PIN, data_arg);
            break;
        case HD44780_MSG_GPIO_RS:
            gpio_write(LCD_RS_PIN, data_arg);
            break;
        case HD44780_MSG_DELAY_E:
            _delay_us(HD44780_DELAY_ENABLE_PULSE);
            break;
        case HD44780_MSG_DELAY_US:
            data_arg = data_arg / 10;
            while (data_arg--) _delay_us(10);
            break;
        case HD44780_MSG_BACKLIGHT:
            TIMER_OCA(3, data_arg);  // set PWM on LCD backlight
            break;

        default:
            break;
    }
    return result;
}

#define PCF_RS 0
#define PCF_RW 1
#define PCF_E 2
#define PCF_BL 3
#define PCF_D4 4
#define PCF_D5 5
#define PCF_D6 6
#define PCF_D7 7
#define PCF_ADDR 0x27

static uint16_t LCD_Handler_i2c(HD44780_MSG msg, uint16_t data_arg) {
    uint16_t result = 0;
    static uint8_t pins;
    uint8_t data;

    switch (msg) {
        case HD44780_MSG_INIT:
            i2c_init();
            pins = 0;
            break;
        case HD44780_MSG_GPIO_DATA_DIRECTION:
            break;
        case HD44780_MSG_GPIO_DATA_READ:
            data = pcf_read(PCF_ADDR, (1 << PCF_D4) | (1 << PCF_D5) |
                                          (1 << PCF_D6) | (1 << PCF_D7));

            if (data & (1 << PCF_D4)) result |= 0x01;
            if (data & (1 << PCF_D5)) result |= 0x02;
            if (data & (1 << PCF_D6)) result |= 0x04;
            if (data & (1 << PCF_D7)) result |= 0x08;
            break;
        case HD44780_MSG_GPIO_DATA_WRITE:
            (data_arg & 0x10) ? BitSet(pins, PCF_D4) : BitClear(pins, PCF_D4);
            (data_arg & 0x20) ? BitSet(pins, PCF_D5) : BitClear(pins, PCF_D5);
            (data_arg & 0x40) ? BitSet(pins, PCF_D6) : BitClear(pins, PCF_D6);
            (data_arg & 0x80) ? BitSet(pins, PCF_D7) : BitClear(pins, PCF_D7);
            break;
        case HD44780_MSG_GPIO_E:
            data_arg ? BitSet(pins, PCF_E) : BitClear(pins, PCF_E);
            break;
        case HD44780_MSG_GPIO_E_TOGGLE:
            BitSet(pins, PCF_E);
            pcf_write(PCF_ADDR, pins);
            _delay_us(HD44780_DELAY_ENABLE_PULSE);
            BitClear(pins, PCF_E);
            break;
        case HD44780_MSG_GPIO_RW:
            data_arg ? BitSet(pins, PCF_RW) : BitClear(pins, PCF_RW);
            break;
        case HD44780_MSG_GPIO_RS:
            data_arg ? BitSet(pins, PCF_RS) : BitClear(pins, PCF_RS);
            break;
        case HD44780_MSG_DELAY_E:
            _delay_us(HD44780_DELAY_ENABLE_PULSE);
            break;
        case HD44780_MSG_BACKLIGHT:
            data_arg ? BitSet(pins, PCF_BL) : BitClear(pins, PCF_BL);
            break;
        case HD44780_MSG_DELAY_US:
            data_arg = data_arg / 10;
            while (data_arg--) _delay_us(10);
            break;
        default:
            break;
    }
    pcf_write(PCF_ADDR, pins);
    return result;
}

static void cmd_help(char* args) {
    UNUSED(args);
    LEF_Cli_print();
}

static void cmd_lcd_init(char* args) {
    UNUSED(args);
    lcd_init(lcd_gpio_callback, HD44780_MODE_ON);
    lcd_puts("HD44780 LCD gpio");
}

static void cmd_lcd_i2c(char* args) {
    UNUSED(args);
    lcd_init(LCD_Handler_i2c, HD44780_MODE_ON);
    lcd_clear();
    lcd_puts("HD44780 LCD i2c");
}

static void cmd_lcd_on(char* args) {
    UNUSED(args);
    lcd_on();
}

static void cmd_lcd_off(char* args) {
    UNUSED(args);
    lcd_off();
}

static void cmd_lcd_cursor_on(char* args) {
    UNUSED(args);
    lcd_on_cursor();
}

static void cmd_lcd_cursor_on_blink(char* args) {
    UNUSED(args);
    lcd_on_cursor_blink();
    lcd_mode(HD44780_MODE_ON_CURSOR_BLINK);
}

static void cmd_lcd_clear(char* args) {
    UNUSED(args);
    lcd_clear();
}

static void cmd_lcd_home(char* args) {
    UNUSED(args);
    lcd_home();
}

static void cmd_lcd_move_right(char* args) {
    UNUSED(args);
    lcd_move_right();
}

static void cmd_lcd_move_left(char* args) {
    UNUSED(args);
    lcd_move_left();
}

static void cmd_lcd_test2(char* args) {
    UNUSED(args);
    lcd_clear();
    lcd_gotoxy(3, 0);
    lcd_puts_P("Testing gotoxy");
    lcd_gotoxy(6, 3);
    lcd_puts_P("Line 4");
    lcd_gotoxy(1, 1);
    lcd_puts_P("Line 2");
    lcd_gotoxy(14, 2);
    lcd_puts_P("Line 3");
}

static void cmd_lcd_test3(char* args) {
    UNUSED(args);
    lcd_clear();
    for (int i = 0; i < 80; i++) {
        lcd_putc('X');
        _delay_ms(20);
    }
}
/*
 * constant definitions
 */
static const PROGMEM unsigned char copyRightChar[] = {
    0x07, 0x08, 0x13, 0x14, 0x14, 0x13, 0x08, 0x07,
    0x00, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x00};
    
static void cmd_lcd_test_cc(char* args) {
    UNUSED(args);
    lcd_clear();
    lcd_puts_P("Custom character");
    
    lcd_send_command(HD44780_INST_CGRAM); /* set CG RAM start address 0 */
    for (int i = 0; i < 16; i++) {
        lcd_send_data(pgm_read_byte_near(&copyRightChar[i]));
    }
    lcd_gotoxy(0, 1);
    lcd_putc(0);
    lcd_putc(1);
}

static const PROGMEM unsigned char custom_characters[] = {
    0xFF, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff
};

static void cmd_lcd_test_cc2(char* args) {
    UNUSED(args);
    lcd_clear();
    lcd_puts_P("Custom character 2");
    
    lcd_send_command(HD44780_INST_CGRAM); /* set CG RAM start address 0 */
    for (int i = 0; i < 16; i++) {
        lcd_send_data(pgm_read_byte_near(&custom_characters[i]));
    }

    lcd_gotoxy(0, 1);
    lcd_putc(0);
    lcd_putc(1);
    lcd_putc(2);
    lcd_putc(3);
    lcd_putc(4);
    lcd_putc(5);
    lcd_putc(6);
    lcd_putc(7);
}

static void cmd_lcd_test_move(char* args) {
    UNUSED(args);
    lcd_clear();
    lcd_puts_P("Some text to move");
    _delay_ms(300);
    lcd_home();
    lcd_send_command(HD44780_MOVE_DISPLAY_RIGHT);
}

static void cmd_lcd_backlight_on(char* args) {
    UNUSED(args);
    lcd_backlight(200);
}

static void cmd_lcd_backlight_off(char* args) {
    UNUSED(args);
    lcd_backlight(0);
}

static void lcd_demo_generic(uint8_t lines, uint8_t cols) {
    char buf[41];
    lcd_clear();
    lcd_backlight(200);
    for (int i=0;i<lines;i++) {
        lcd_gotoxy(i*2, i);
        sprintf(buf, "Line %d", i);
        lcd_puts(buf);
        _delay_ms(300);
    }
    for (int i=0;i<lines;i++) {
        lcd_gotoxy(0, i);
        for (int j=0; j<cols;j++ ) {
            lcd_putc((char)(65+i));
            _delay_ms(40);
        }
    }

    for (int j=0; j<(cols/2);j++ ) {
        for (int i=0;i<lines;i++) {
            lcd_gotoxy(j, i);
            lcd_putc(' ');
            lcd_gotoxy(cols-j, i);
            lcd_putc(' ');
            _delay_ms(30);
        }
    }

    lcd_clear();
}

static void cmd_lcd_demo4x20(char* args) {
    UNUSED(args);
    lcd_demo_generic(4,20);
}
static void cmd_lcd_demo2x40(char* args) {
    UNUSED(args);
    lcd_demo_generic(2,20);
}
static void cmd_lcd_demo2x20(char* args) {
    UNUSED(args);
    lcd_demo_generic(2,20);
}
static void cmd_lcd_demo2x16(char* args) {
    UNUSED(args);
    lcd_demo_generic(2,16);
}

static void cmd_lcd_cnt(char* args) {
    UNUSED(args);
    char buf[12];
    lcd_clear();
    lcd_puts("Counter");
    for (int i=0; i<2000; i++) {
        lcd_gotoxy(3,1);
        sprintf(buf,"%4d",i);
        lcd_puts(buf);
    }
}

static void cmd_lcd_characters(char* args) {
    UNUSED(args);;
    lcd_clear();
    for (int p=0;p<7;p++) {
        for (int j=0;j<2;j++) {
            lcd_gotoxy(0,j);
            for (int i=0;i<20;i++) {
                lcd_putc((char)(p*40 + 20*j + i)); 
            }
        }
        _delay_ms(2000);
    }
    
}

static void cmd_reset(char* args) {
    UNUSED(args);
    RESET();
}

static void cmd_sysinfo(char* args) {
    UNUSED(args);
    // print_sysinfo();
    LEF_print_sysinfo();
}

struct cRGB led[20];

static void cmd_led_off(char* args) {
    UNUSED(args);
    for (int i=0;i<20;i++) {
        led[i].r = 0;
        led[i].g = 0;
        led[i].b = 0;
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_white(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 255;
        led[i].g = 255;
        led[i].b = 255;  
    }
    ws2812_setleds(led, 10);
}

static void cmd_led_red(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 255;
        led[i].g = 0;
        led[i].b = 0;  
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_green(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 0;
        led[i].g = 255;
        led[i].b = 0;  
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_blue(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 0;
        led[i].g = 0;
        led[i].b = 255;
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_yellow(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 255;
        led[i].g = 255;
        led[i].b = 0;  
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_cyan(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 0;
        led[i].g = 255;
        led[i].b = 255;  
    }
    ws2812_setleds(led, 20);
}

static void cmd_led_magenta(char* args) {
    UNUSED(args);
    for (int i=0;i<10;i++) {
        led[i].r = 255;
        led[i].g = 0;
        led[i].b = 255;  
    }
    ws2812_setleds(led, 20);
}



static void cmd_led_test(char* args) {
    UNUSED(args);
    for (int i=0;i<256;i++) {
        led[0].r = i;
        led[0].g = 0;
        led[0].b = 0;  
        ws2812_setleds(led, 1);
        LEF_Delay(1);
    }
    for (int i=0;i<256;i++) {
        led[0].r = 255;
        led[0].g = i;
        led[0].b = 0;  
        ws2812_setleds(led, 1);
        LEF_Delay(1);
    }
    for (int i=0;i<256;i++) {
        led[0].r = 255;
        led[0].g = 255;
        led[0].b = i;  
        ws2812_setleds(led, 1);
        LEF_Delay(1);
    }   
}

static void cmd_led_demo(char* args) {
    UNUSED(args);

    for (int i=0;i<10;i++) {
        for (int j=0;j<8;j++) {
            for (int k=0;k<8;k++) {
                led[k].r = 0;
                led[k].g = 0;
                led[k].b = 0;  
            }
            led[j].r = 255;
            led[j].g = 0;
            led[j].b = 0;  
            ws2812_setleds(led, 10);
            LEF_Delay(10);
        }
        for (int j=6;j>0;j--) {
            for (int k=0;k<8;k++) {
                led[k].r = 0;
                led[k].g = 0;
                led[k].b = 0;  
            }
            led[j].r = 255;
            led[j].g = 0;
            led[j].b = 0;  
            ws2812_setleds(led, 10);
            LEF_Delay(10);
        }
        
    }
}

static uint8_t fade(uint8_t value) {
    switch (value) {
        case 250:   return 60;
        case 60:    return 30;
        case 30:    return 15;
        case 15:    return 7;
        case 7:     return 3;
        case 3:     return 1;
        default:
            return 0;
    }
}

static void cmd_led_demo2(char* args) {
    UNUSED(args);
    for (int j=0;j<8;j++) {
        led[j].r = 0;
        led[j].g = 0;
        led[j].b = 0;
    }

    for (int i=0;i<10;i++) {
        for (int j=0;j<8;j++) {
            for (int k=0;k<8;k++) {
                if (led[k].r > 0) {
                    led[k].r = fade(led[k].r);
                }
            }
            led[j].r = 250;
            ws2812_setleds(led, 10);
            LEF_Delay(10);
        }
        for (int j=6;j>0;j--) {
            for (int k=0;k<8;k++) {
                if (led[k].r > 0) {
                    led[k].r = fade(led[k].r);
                }
            }
            led[j].r = 250;
            ws2812_setleds(led, 10);
            LEF_Delay(10);
        }
    }
}

static void cmd_led_ringdemo(char* args) { 
    UNUSED(args);
    for (int j = 0; j < 12; j++) {
        led[j].r = 0;
        led[j].g = 0;
        led[j].b = 0;
    }
    for (int i=0;i<10;i++) {
        for (int j=0;j<12;j++) {
            for (int k = 0; k < 12; k++) {
                if (led[k].r > 0) {
                    led[k].r = fade(led[k].r);
                }
            }
            // led[j].r = 50;
            // ws2812_setleds(led, 12);
            // LEF_Delay(2);

            // led[j].r = 100;
            // ws2812_setleds(led, 12);
            // LEF_Delay(2);
            
            // led[j].r = 150;
            // ws2812_setleds(led, 12);
            // LEF_Delay(2);
            
            // led[j].r = 200;
            // ws2812_setleds(led, 12);
            // LEF_Delay(2);
            
            led[j].r = 250;
            ws2812_setleds(led, 12);
            LEF_Delay(10);
        }
    }
}

const PROGMEM LEF_CliCmd cmdTable[] = {
    LEF_CLI_LABEL("LCD"),
    {cmd_lcd_init, "initgpio", "Initiate HD44780 LCD"},
    {cmd_lcd_i2c, "initi2c", "Initiate LCD on PCF8574"},
    {cmd_lcd_on, "on", "Turn LCD on"},
    {cmd_lcd_off, "off", "Turn LCD off"},
    {cmd_lcd_backlight_on, "blon", "LCD backlight off"},
    {cmd_lcd_backlight_off, "bloff", "LCD backlight off"},
    {cmd_lcd_clear, "clr", "Clear LCD"},
    {cmd_lcd_home, "lcdh", "Move cursor to home pos"},
    {cmd_lcd_move_right, "lcdr", "Move text right"},
    {cmd_lcd_move_left, "lcdl", "Move text left"},
    {cmd_lcd_test_cc, "cc", "Testing custom character"},
    {cmd_lcd_test_cc2, "cc2", "Testing custom character 2"},
    {cmd_lcd_test_move, "lcdtm", "Run LCD move test"},
    {cmd_lcd_cursor_on, "curon", "Cursor on"},
    {cmd_lcd_cursor_on_blink, "curbl", "Cursor blinking"},
    {cmd_lcd_test2, "lcdt2", "Testing gotoxy"},
    {cmd_lcd_test3, "lcdt3", "Testing wrap"},
    {cmd_lcd_cnt, "cnt", "Counter"},
    {cmd_lcd_demo4x20, "demo204", "Demo 20x4"},
    {cmd_lcd_demo2x40, "demo402", "Demo 40x2"},
    {cmd_lcd_demo2x20, "demo202", "Demo 20x2"},
    {cmd_lcd_demo2x16, "demo162", "Demo 16x2"},
    {cmd_lcd_characters, "chars", "Demo all characters"},
    LEF_CLI_LABEL("NeoPixel"),
    {cmd_led_test, "np1", "NeoPixel test 1"},
    {cmd_led_off, "npoff", "NeoPixel off"},
    {cmd_led_white, "npwhite", "NeoPixel white"},
    {cmd_led_red, "npred", "NeoPixel red"},
    {cmd_led_green, "npgreen", "NeoPixel green"},
    {cmd_led_blue, "npblue", "NeoPixel blue"},
    {cmd_led_yellow, "npyellow", "NeoPixel yellow"},
    {cmd_led_cyan, "npcyan", "NeoPixel cyan"},
    {cmd_led_magenta, "npmagenta", "NeoPixel magenta"},
    {cmd_led_demo, "npdemo", "NeoPixel demo"},
    {cmd_led_demo2, "npdemo2", "NeoPixel demo 2"},
    {cmd_led_ringdemo, "npring", "NeoPixel ring demo"},
    LEF_CLI_LABEL("Misc"),
    {cmd_reset, "reset", "Reset the system"},
    {cmd_sysinfo, "info", "Print system info"},
    LEF_CLI_CMD(cmd_help, "help", "Show help")
};

ISR(TIMER1_COMPA_vect) {

    LEF_Timer_update(&timer1);
    LEF_Timer_update(&timer2);
    LEF_Timer_update(&timer_a);
    LEF_systick();

    if (LEF_Pot_is_enabled(&bl_pot))
        ADC_START();
}

ISR(ADC_vect) {
    LEF_Pot_update(&bl_pot, ADC_VALUE());
}

static void hw_init(void) {
    stdout = &mystdout;
    uart1_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));

    // Timer 1 (16 bit) 10ms intervall on OCA1 interrupt 
    // (used for LEF system timer)
    TIMER1_CLK_PRES_64(); // alternative for 10ms that gives good accuracy
    TIMER1_OCA(2499);
    TIMER1_WGM_CTC();       // Clear timer on compare (on OC1A)
    TIMER1_OCA_INT(true);   // enable output compare A interrupt

    // Timer 3 (16-bit) setup for PWM on OCA3 for backlight controll
    TIMER_CLK_DIV_64(3);
    TIMER_MODE_FAST_PWM_8BIT(3);
    TIMER_COM_OC_CLEAR(3,A);
    TIMER_OCA(3, 200);
    gpio_init(LCD_BACKLIGHT_PIN, GPIO_OUTPUT, GPIO_NO_PULLUP);

    ADC_ENABLE();
    ADC_REF_AVCC();
    ADC_PRESCALER_128();
    ADC_MUX(POT_ADC);
    ADC_IE();

    sei();
}


static bool main_event_handler(LEF_Event* event) {
    uint16_t ch;
    uint16_t val;

    if (evOn) LEF_Print_event(event);

    switch (event->id) {
        case EVENT_Button1:  // Handle button press event
            break;
        case EVENT_Button2:  // Handle button press event
            break;
        case EVENT_Button3:  // Handle button press event
            break;
        case EVENT_Timer2:  // Handle data from uart to Cli
            ch = uart1_getc();
            while ((ch & 0xff00) != UART_NO_DATA) {
                LEF_Cli_putc(ch);
                ch = uart1_getc();
            }
            break;
        case EVENT_BL_POT:  // Controll backlight
            val = LEF_Pot_state(&bl_pot);
            // printf("Pot: %d\n", val);
            if (val < 20)
                lcd_backlight(0);
            else
                lcd_backlight(val / 4);
            break;

        case LEF_EVENT_CLI:
            LEF_Cli_exec(event);
            break;
    }
    return true;
}

int main(void) {

    LEF_init();

    LEF_Timer_init(&timer1, EVENT_Timer1);
    LEF_Timer_start_repeat(&timer1, 100);
    LEF_Timer_init(&timer2, EVENT_Timer2);
    LEF_Timer_start_repeat(&timer2, 10);
    LEF_Timer_init(&timer_a, EVENT_TimerA);
    LEF_Pot_init(&bl_pot, EVENT_BL_POT);
    bl_pot.threshold=5;    
    LEF_CLI_INIT(cmdTable);
    
    hw_init();

    printf_P(PSTR("\n\nHD44780 LCD testprogram (ET-AVR-STAMP)\n\n"));

    LEF_Run(main_event_handler, NULL);

    return 0;
}
