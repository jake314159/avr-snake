/* Skeleton Code for COMP2215 Task 3

    REGULAR WORK

    /////////////// SCHEDULER CODE HEADER START //////////////

   Template for preemptive version of RIOS implemented on an AVR
   available from: http://www.cs.ucr.edu/~vahid/rios/rios_avr.htm
   
   Slightly modified by Klaus-Peter Zauner, Feb 2014:
   - Slowed down to make tasks observable over serial connection
   - Adapted to avr-libc conventions

   Modified by Jacob Causon, Feb 2014
   - Added Idle sleep
   - Idle LED indicator added


   Copyright (c) 2012 UC Regents. All rights reserved.

      Developed by: Frank Vahid, Bailey Miller, and Tony Givargis
      University of California, Riverside; University of California, Irvine
      <http://www.riosscheduler.org>http://www.riosscheduler.org

      Permission is hereby granted, free of charge, to any person
      obtaining a copy of this software and associated documentation
      files (the "Software"), to deal with the Software without
      restriction, including without limitation the rights to use,
      copy, modify, merge, publish, distribute, sublicense, and/or sell
      copies of the Software, and to permit persons to whom the
      Software is furnished to do so, subject to the following conditions:

	  * Redistributions of source code must retain the above
	    copyright notice, this list of conditions and the following
	    disclaimers.
	  * Redistributions in binary form must reproduce the above
	    copyright notice, this list of conditions and the following
	    disclaimers in the documentation and/or other materials
	    provided with the distribution.
	  * Neither the names of any of the developers or universities nor 
	    the names of its contributors may be used to endorse or
	    promote products derived from this Software without 
	    specific prior written permission.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
      EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
      OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
      NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
      HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
      FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
      OTHER DEALINGS WITH THE SOFTWARE.
   (http://opensource.org/licenses/NCSA)	

    /////////////// SCHEDULER CODE HEADER END  //////////////

*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>
#include "debug.h"

#include "lcdlib/lcd.h"

#define BLINK_TIME 50

//Sleep mode = idle
#define SLEEP_MODE 0

#define TRUE 1
#define FALSE 0
#define true 1
#define false 0

#define LED_ON PORTB |= _BV(PINB7)
#define LED_OFF PORTB &= ~_BV(PINB7)

#define STDIO_BAUD  9600
#define PRNTBUF 200

#define D0_L  DDRD |=  _BV(PD0); PORTD &= ~_BV(PD0);
#define D0_H  DDRD |=  _BV(PD0); PORTD |=  _BV(PD0);
#define D0_R  DDRD &= ~_BV(PD0); PORTD |=  _BV(PD0);
#define D0_Z  DDRD &= ~_BV(PD0); PORTD &= ~_BV(PD0);

#define D1_L  DDRD |=  _BV(PD1); PORTD &= ~_BV(PD1);
#define D1_H  DDRD |=  _BV(PD1); PORTD |=  _BV(PD1);
#define D1_R  DDRD &= ~_BV(PD1); PORTD |=  _BV(PD1);
#define D1_Z  DDRD &= ~_BV(PD1); PORTD &= ~_BV(PD1);


#define C2_L  DDRC |=  _BV(PC2); PORTC &= ~_BV(PC2);
#define C2_H  DDRC |=  _BV(PC2); PORTC |=  _BV(PC2);
#define C2_R  DDRC &= ~_BV(PC2); PORTC |=  _BV(PC2);
#define C2_Z  DDRC &= ~_BV(PC2); PORTC &= ~_BV(PC2);

#define C3_L  DDRC |=  _BV(PC3); PORTC &= ~_BV(PC3);
#define C3_H  DDRC |=  _BV(PC3); PORTC |=  _BV(PC3);
#define C3_R  DDRC &= ~_BV(PC3); PORTC |=  _BV(PC3);
#define C3_Z  DDRC &= ~_BV(PC3); PORTC &= ~_BV(PC3);

#define C4_L  DDRC |=  _BV(PC4); PORTC &= ~_BV(PC4);
#define C4_H  DDRC |=  _BV(PC4); PORTC |=  _BV(PC4);
#define C4_R  DDRC &= ~_BV(PC4); PORTC |=  _BV(PC4);
#define C4_Z  DDRC &= ~_BV(PC4); PORTC &= ~_BV(PC4);

#define C5_L  DDRC |=  _BV(PC5); PORTC &= ~_BV(PC5);
#define C5_H  DDRC |=  _BV(PC5); PORTC |=  _BV(PC5);
#define C5_R  DDRC &= ~_BV(PC5); PORTC |=  _BV(PC5);
#define C5_Z  DDRC &= ~_BV(PC5); PORTC &= ~_BV(PC5);

/* #define DZ DDRD=0; PORTD=0; */
#define CZ DDRD=0; PORTD=0;

/*
   Copyright (c) 2013 Frank Vahid, Tony Givargis, and
   Bailey Miller. Univ. of California, Riverside and Irvine.
   RIOS version 1.2
*/
typedef struct task {
   unsigned char running;     // 1 indicates task is running
   int state;                 // Current state of state machine
   unsigned long period;      // Rate at which the task should tick
   unsigned long elapsedTime; // Time since task's previous tick
   int (*TickFct)(int);       // Function to call for task's tick
} task;

task tasks[3];

const unsigned char tasksNum = 3;
uint8_t activeProcesses = 0;

const double tick_ms = 1.0;  /* Real time between ticks in ms */
const unsigned long tasksPeriodGCD = 1;  // Timer tick rate
const unsigned long rotor_time = 1; //Every 1 ms
const unsigned long button_time = 40;
const unsigned long draw_time = 150;
const unsigned long period4 = 200;

// Stores the value of the scroll wheel
volatile int16_t scroll_delta = 0;

void init_LED();
void blink();

int rotor_task(int state);
int button_task(int state);
int draw_task(int state);

unsigned char runningTasks[3] = {255}; // Track running tasks, [0] always idleTask
const unsigned long idleTask = 255; // 0 highest priority, 255 lowest
unsigned char currentTask = 0; // Index of highest priority task in runningTasks

uint8_t ddrc, portc;


//Black
#define BACKGROUND_COLOR 0x0000 

#define STEP_SIZE 4
#define BLOCK_SIZE 14
#define GRID_START_X 14
#define GRID_START_Y 14
#define GRID_SIZE_X 14
#define GRID_SIZE_Y 18
#define SNAKE_LENGTH (GRID_SIZE_X * GRID_SIZE_Y)
// White
#define SNAKE_COLOR 0xffff

typedef struct Point {
    uint8_t x, y;
} Point;

uint8_t numColors = 7;
int8_t selectedColor = 0;
uint16_t colors[7] = {WHITE, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW};

char coverOld = true;
//rectangle rectOld = {10,20,10,20};
rectangle rect = {10,20,10,20};

/*
    'N' North
    'S' South
    'E' East
    'W' West
*/
char buttonPressed = 'E';

Point snake[SNAKE_LENGTH];
Point food = {5,5};
uint8_t snakeHead = 0;
uint8_t snakeLength = 1;
char gameOver = FALSE;

unsigned schedule_time = 0;
ISR(TIMER1_COMPA_vect) {
   sleep_disable();
   unsigned char i;
   for (i=0; i < tasksNum; ++i) { // Heart of scheduler code
      if (  (tasks[i].elapsedTime >= tasks[i].period) // Task ready
          && (runningTasks[currentTask] > i) // Task priority > current task priority
          && (!tasks[i].running) // Task not already running (no self-preemption)
         ) { 
	   cli();	  
         tasks[i].elapsedTime = 0; // Reset time since last tick
         tasks[i].running = 1; // Mark as running
         currentTask += 1;
         runningTasks[currentTask] = i; // Add to runningTasks
         activeProcesses++; //Count the number of active processes
        // LED_ON;
       sei();
	 
         tasks[i].state = tasks[i].TickFct(tasks[i].state); // Execute tick
         
       cli();	 
         tasks[i].running = 0; // Mark as not running
         runningTasks[currentTask] = idleTask; // Remove from runningTasks
         currentTask -= 1;
         activeProcesses--; //Count the number of active processes
         if(activeProcesses == 0){ 
        //    LED_OFF;
         //   sleep_enable();
            sei();
          //  sleep_cpu();       
         } else {
            sei();
         }
      }
      tasks[i].elapsedTime += tasksPeriodGCD;
   }
}

void init_processor() {
    
    /* Configure 16 bit Timer for ISR  */
    TCCR1B = _BV(WGM12)   /* Clear Timer on Compare Match Mode (CTC) */
          | _BV(CS12)
          | _BV(CS10); 	 /* F_CPU / 1024 */

    OCR1A = (uint16_t)(F_CPU * tick_ms / (1024.0 * 1000)- 0.5);

    TIMSK1 = _BV(OCIE1A); //enables compare match interrupt
    TCNT1 = 0;
         
    sei();
}

void init_LED() {
    DDRB |= _BV(PINB7); /* set LED as output */

    /* Free up port C: disable JTAG (protected): */
    MCUCR |= _BV(JTD); /* Requires 2 writes      */
    MCUCR |= _BV(JTD); /* within 4 clock cycles  */
    
    DDRB &= ~_BV(PINC0)   /* inputs */
 	  & ~_BV(PINC1);
 
    PORTC |= _BV(PINC0)   /* enable pull ups */
   	   | _BV(PINC1);
    
}

void init_sleep()
{
    set_sleep_mode(SLEEP_MODE);
}

 void init_stdio_uart1(void)
 {
	 /* Configure UART1 baud rate, one start bit, 8-bit, no parity and one stop bit */
	 UBRR1H = (F_CPU/(STDIO_BAUD*16L)-1) >> 8;
	 UBRR1L = (F_CPU/(STDIO_BAUD*16L)-1);
	 UCSR1B = _BV(RXEN1) | _BV(TXEN1);
	 UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);

	 /* Setup new streams for input and output */
	 static FILE uout = FDEV_SETUP_STREAM(uputchar1, NULL, _FDEV_SETUP_WRITE);
	 static FILE uin = FDEV_SETUP_STREAM(NULL, ugetchar1, _FDEV_SETUP_READ);

	 /* Redirect all standard streams to UART0 */
	 stdout = &uout;
	 stderr = &uout;
	 stdin = &uin;
 }

void init_snake()
{
    DDRC    = ddrc;  // Restore display configuration of Port C
    PORTC   = portc;
    _delay_ms(3);

    uint8_t i;
    for(i=0; i<3; i++) {
        snake[i].x = i+3;
        snake[i].y = 3;
    }
    snakeLength = 3;
    snakeHead = 2;
    gameOver = FALSE;

    char buttonPressed = 'E';
    Point snake[SNAKE_LENGTH];
    Point food = {5,5};

    clear_screen();
}

int main(void)
{

   init_debug_uart1();

   
   init_LED();
//     init_stdio_uart1();
    

   init_lcd();
    portc  = PORTC;  /* Store display configuration of Port C */
     ddrc   = DDRC;

   // Priority assigned to lower position tasks in array
   unsigned char i = 0;

   tasks[i].state = -1;
   tasks[i].period = rotor_time;
   tasks[i].elapsedTime = tasks[i].period;
   tasks[i].running = 0;
   tasks[i].TickFct = &rotor_task;
   ++i;
   tasks[i].state = -1;
   tasks[i].period = button_time;
   tasks[i].elapsedTime = tasks[i].period;
   tasks[i].running = 0;
   tasks[i].TickFct = &button_task;
   ++i;
   tasks[i].state = -1;
   tasks[i].period = draw_time;
   tasks[i].elapsedTime = tasks[i].period;
   tasks[i].running = 0;
   tasks[i].TickFct = &draw_task;

   init_processor();

    
   init_snake();
    
   while(1);   
}

int rotor_task(int state) {
    cli();
    static int8_t last;
    uint8_t ddrc, portc;
    uint8_t ddrd, portd;
    int8_t new, diff;
    uint8_t wheel;

    /* Save state of Port C & D */
    ddrc   = DDRC; portc  = PORTC;
    ddrd   = DDRD; portd  = PORTD;

    CZ C2_H C3_H C2_R C3_R
    D0_Z D1_L
    _delay_us(3);
    wheel = PINC;

    /*
    Scan rotary encoder
    ===================
    This is adapted from Peter Dannegger's code available at:
    http://www.mikrocontroller.net/attachment/40597/ENCODE.C
    */

    new = 0;
    if( wheel  & _BV(PC3) ) {
        new = 3;
    }
    if( wheel  & _BV(PC2) ) {
        new ^= 1;			        // convert gray to binary
    }
    diff = last - new;			// difference last - new
    if( diff & 1 ){			// bit 0 = value (1)
        last = new;		       	// store new as next last
        scroll_delta += ((diff & 2) - 1);	// bit 1 = direction (+/-)
    }

    /* Restore state of Port C */
    DDRC    = ddrc; PORTC   = portc;
    DDRD    = ddrd; PORTD   = portd;

    PORTA &= ~_BV(PA0);  /* ISR Timing End */
    sei();
    return ++state;
}

int16_t iob_delta(){
    int16_t val;

    cli();
    val = scroll_delta;
    scroll_delta &= 1;
    sei();

    return val >> 1;
}

void pushed(char type) {
    printf("Pressed %c\n", type);
    switch(type) {
        case 'W':
           if(buttonPressed != 'S')
                buttonPressed = 'N';
           break; 
        case 'E':
           if(buttonPressed != 'N')
                buttonPressed = 'S';
           break; 
        case 'S':
           if(buttonPressed != 'E')
                buttonPressed = 'W';
           break; 
        case 'N':
           if(buttonPressed != 'W')
                buttonPressed = 'E';
           break; 
        case 'C':
           coverOld = false;
           break;
    }
}

int button_task(int state) {
    //printf( "-> Rotor [% 3d]\n", scroll_delta);
D0_H
         D0_R

         C2_H C3_H C4_H C5_L
         _delay_ms(3);
         if (!(PIND & _BV(PD0))) { pushed('S'); }

             C2_H C3_H C4_L C5_H
         _delay_ms(3);
         if (!(PIND & _BV(PD0))) { pushed('W'); }


             C2_H C3_L C4_H C5_H
         _delay_ms(3);
         if (!(PIND & _BV(PD0))) { pushed('N'); }

             C2_L C3_H C4_H C5_H
         _delay_ms(3);
         if (!(PIND & _BV(PD0))) { pushed('E'); }

         D0_Z

         D1_H 
         D1_R
         
        if(gameOver) {
                 C2_Z C3_Z C4_L C5_Z
             if (!(PIND & _BV(PD1))) { init_snake(); }
        }

       //  D1_L

       //      C2_R C3_R
       /*  if (!(PINC & _BV(PC2))) { pushed("A"); }


         if (!(PINC & _BV(PC3))) { pushed("B "); }*/


         DDRC    = ddrc;  /* Restore display configuration of Port C */
         PORTC   = portc;

        int16_t d = iob_delta();
        selectedColor += d;//scroll_delta;
        scroll_delta = 0;
        if(selectedColor >= numColors) {
            selectedColor = 0;
        } else if(selectedColor < 0) {
            selectedColor = numColors -1;
        }
        //selectedColor = selectedColor % numColors;
        if(d != 0)
            printf("new color %d delta %d\n", selectedColor, d);

    return ++state;
}

void updateSnake()
{
    switch(buttonPressed) {
        case 'W':
            if(snake[snakeHead].x == 0) {
                gameOver = TRUE;
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH].x = snake[snakeHead].x-1;
            snake[(snakeHead+1) % SNAKE_LENGTH].y = snake[snakeHead].y;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH;
            break;
        case 'E':
            if(snake[snakeHead].x > GRID_SIZE_X-2) {
                gameOver = TRUE;
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH].x = snake[snakeHead].x+1;
            snake[(snakeHead+1) % SNAKE_LENGTH].y = snake[snakeHead].y;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH;
            break;
        case 'S':
            if(snake[snakeHead].y ==0) {
                gameOver = TRUE;
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH].x = snake[snakeHead].x;
            snake[(snakeHead+1) % SNAKE_LENGTH].y = snake[snakeHead].y-1;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH;
            break;
        case 'N':
            if(snake[snakeHead].y > GRID_SIZE_Y-2) {
                gameOver = TRUE;
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH].x = snake[snakeHead].x;
            snake[(snakeHead+1) % SNAKE_LENGTH].y = snake[snakeHead].y+1;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH;
            break;
    }
}

int draw_task(int state) {
    DDRC    = ddrc;  // Restore display configuration of Port C
    PORTC   = portc;
    _delay_ms(3);
    updateSnake();

    if(gameOver) {
        display_string("GAME OVER \n");
        return 0;
    }

    uint8_t h = snakeHead;

    if(snake[h].x == food.x && snake[h].y == food.y) {
        snakeLength++;
        //Pick a new 'random' place on the grid
        food.x = (food.y + 43) % (GRID_SIZE_X-1);
        food.y = (food.x + 71) % (GRID_SIZE_Y-1);
    } else {
        rect.top = food.y*BLOCK_SIZE;
        rect.bottom = (food.y+1)*BLOCK_SIZE;
        rect.right = (food.x+1)*BLOCK_SIZE;
        rect.left = food.x*BLOCK_SIZE;
        fill_rectangle(rect, YELLOW);
    }
    
    rect.top = snake[h].y*BLOCK_SIZE;
    rect.bottom = (snake[h].y+1)*BLOCK_SIZE;
    rect.right = (snake[h].x+1)*BLOCK_SIZE;
    rect.left = snake[h].x*BLOCK_SIZE;
    fill_rectangle(rect, RED);
    if(h>0) {
        h--;
    } else {
        h = SNAKE_LENGTH-1;
    }
    //Replace old head with white
    rect.top = snake[h].y*BLOCK_SIZE;
    rect.bottom = (snake[h].y+1)*BLOCK_SIZE;
    rect.right = (snake[h].x+1)*BLOCK_SIZE;
    rect.left = snake[h].x*BLOCK_SIZE;
    fill_rectangle(rect, SNAKE_COLOR);

    // An awful way to reduce h by snakeLength with a loop around
    uint8_t i = snakeLength-1; //-1 as head handled before
    for(; i; --i) {
        if(h>0) {
            h--;
        } else {
            h = SNAKE_LENGTH-1;
        }
        
    }
    //Cover old snake
    rect.top = snake[h].y*BLOCK_SIZE;
    rect.bottom = (snake[h].y+1)*BLOCK_SIZE;
    rect.right = (snake[h].x+1)*BLOCK_SIZE;
    rect.left = snake[h].x*BLOCK_SIZE;
    fill_rectangle(rect, BLACK);

    rect.top = 0;
    rect.bottom = (GRID_SIZE_Y)*BLOCK_SIZE;
    rect.right = (GRID_SIZE_X)*BLOCK_SIZE;
    rect.left = 0;
    draw_rectangle(rect, BLUE);

    coverOld = true;
    D1_Z

    D0_H
    D0_R

    return ++state;
}

