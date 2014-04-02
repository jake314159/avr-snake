/* Skeleton Code for COMP2215 Task 3

    REGULAR WORK

    /////////////// SCHEDULER CODE HEADER START //////////////

   Template for preemptive version of RIOS implemented on an AVR
   available from: http://www.cs.ucr.edu/~vahid/rios/rios_avr.htm
   
   Slightly modified by Klaus-Peter Zauner, Feb 2014:
   - Slowed down to make tasks observable over serial connection
   - Adapted to avr-libc conventions

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

    /////////////// revRotDriver CODE HEADER START //////////////
        Author: Klaus-Peter Zauner            March 2014 
            using components written by
            Steve Gunn and Peter Dannegger

   Licence: This work is licensed under the Creative Commons Attribution License.
            View this license at http:	//creativecommons.org/about/licenses/

    /////////////// revRotDriver CODE HEADER END //////////////

    /////////////// GAME CODE HEADER START //////////////
        Author: Jacob Causon            
                April 2014 

   Licensed under the Apache License, Version 2.0 (the "License"); 
    you may not use this file except in compliance with the License. 
    You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software distributed
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.

    /////////////// GAME CODE HEADER END //////////////

*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <math.h>
#include <util/delay.h>

#include "debug.h"
#include "lcdlib/lcd.h"
#include "RIOS.h"
#include "revRotDriver.h"
#include "avrSnake.h"

#define TRUE 1
#define FALSE 0

#define LED_ON PORTB |= _BV(PINB7)
#define LED_OFF PORTB &= ~_BV(PINB7)


//Scheduler data
#define TASK_NUMBER 2
task tasks[TASK_NUMBER];
const unsigned char tasksNum = TASK_NUMBER;

uint8_t activeProcesses = 0;

const double tick_ms = 1.0;  /* Real time between ticks in ms */
const unsigned long tasksPeriodGCD = 1;  // Timer tick rate
const unsigned long button_time = 40; //50ms
const unsigned long draw_time = 150; //150ms

void init_LED();

void button_task(void);
void draw_task(void);

unsigned char runningTasks[TASK_NUMBER] = {255}; // Track running tasks, [0] always idleTask
const unsigned long idleTask = 255; // 0 highest priority, 255 lowest
unsigned char currentTask = 0; // Index of highest priority task in runningTasks



        /////////////////////////////
        // START OF THE GAME CODE ///
        /////////////////////////////

rectangle rect = {10,20,10,20};

/*
    'N' North
    'S' South
    'E' East
    'W' West
*/
char buttonPressed = 'E';

Point snake[SNAKE_LENGTH_MAX];
Point food = {5,5};
uint8_t snakeHead = 0;
uint8_t snakeLength = 1;
char gameOver = FALSE;
uint8_t score = 0;

// The scheduler
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
       sei();
	 
         tasks[i].TickFct(); // Execute tick
         
       cli();	 
         tasks[i].running = 0; // Mark as not running
         runningTasks[currentTask] = idleTask; // Remove from runningTasks
         currentTask -= 1;
         activeProcesses--; //Count the number of active processes
       sei();
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

// Set up the LED to show on GAME OVER
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

// Set up serial output for debugging 
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

// Set the snake in the init position
// and clear the screen
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

    buttonPressed = 'E';
    food.x = 5;
    food.y = 5;
    score = 0;

    clear_screen();
}

//Actually init things
int main(void)
{
    init_debug_uart1(); //Can be removed when not debugging
    init_LED();

    init_lcd();
    portc  = PORTC;  /* Store display configuration of Port C */
    ddrc   = DDRC;
    
    init_snake();

    // Priority assigned to lower position tasks in array
    unsigned char i = 0;

    tasks[i].period = button_time;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].running = 0;
    tasks[i].TickFct = &button_task;
    ++i;
    tasks[i].period = draw_time;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].running = 0;
    tasks[i].TickFct = &draw_task;

    init_processor();
 
    while(1);   
}

void pushed(char type) {
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
           LED_OFF;
           init_snake(); 
           break;
    }
}

// Task to check for button presses
void button_task(void)
{
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
         if (!(PIND & _BV(PD1))) { 
            pushed('C');
        }
    }

    DDRC    = ddrc;  /* Restore display configuration of Port C */
    PORTC   = portc;
}

// Game over
void set_gameOver()
{
    gameOver = TRUE;
    display_string("SCORE:");

    //4 is 3 digits for the number and 1 for \0
    char str[4];
    snprintf(str, 4, "%d", score);

    display_string(str);
    display_string("\n\n\r\r");
    
}

void updateSnake()
{
    switch(buttonPressed) {
        case 'W':
            if(snake[snakeHead].x == 0) {
                set_gameOver();
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].x = snake[snakeHead].x-1;
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].y = snake[snakeHead].y;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH_MAX;
            break;
        case 'E':
            if(snake[snakeHead].x > GRID_SIZE_X-2) {
                set_gameOver();
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].x = snake[snakeHead].x+1;
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].y = snake[snakeHead].y;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH_MAX;
            break;
        case 'S':
            if(snake[snakeHead].y ==0) {
                set_gameOver();
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].x = snake[snakeHead].x;
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].y = snake[snakeHead].y-1;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH_MAX;
            break;
        case 'N':
            if(snake[snakeHead].y > GRID_SIZE_Y-2) {
                set_gameOver();
                break;
            }
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].x = snake[snakeHead].x;
            snake[(snakeHead+1) % SNAKE_LENGTH_MAX].y = snake[snakeHead].y+1;
            snakeHead = (snakeHead+1) % SNAKE_LENGTH_MAX;
            break;
    }
}

void checkSelfCollide()
{
    uint8_t h = snakeHead-1;
    if(h<0) {
        h = SNAKE_LENGTH_MAX-1;
    }

    uint8_t i = snakeLength-1;
    for(; i; --i) {
        if(snake[h].x == snake[snakeHead].x && snake[h].y == snake[snakeHead].y) {
            set_gameOver();
            return;
        }

        if(h>0) {
            h--;
        } else {
            h = SNAKE_LENGTH_MAX-1;
        }
    }
}

void draw_task(void)
{
    DDRC    = ddrc;  // Restore display configuration of Port C
    PORTC   = portc;
    /* We still need a delay here for the pins to update BUT the other functions */
    /* Make it up so we don't need the extra delay                               */
    //_delay_ms(3);
    if(!gameOver) {
        updateSnake();
        checkSelfCollide();
    }

    if(gameOver) {
        display_string("GAME OVER  \n");
        LED_ON;
        return;
    }

    uint8_t h = snakeHead;

    // Check if we have run into the food and if we haven't
    // Then draw it
    if(snake[h].x == food.x && snake[h].y == food.y) {
        snakeLength++;
        score++;
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
    
    // Draw the snake head
    rect.top = snake[h].y*BLOCK_SIZE;
    rect.bottom = (snake[h].y+1)*BLOCK_SIZE;
    rect.right = (snake[h].x+1)*BLOCK_SIZE;
    rect.left = snake[h].x*BLOCK_SIZE;
    fill_rectangle(rect, SNAKE_HEAD_COLOR);
    if(h>0) {
        h--;
    } else {
        h = SNAKE_LENGTH_MAX-1;
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
            h = SNAKE_LENGTH_MAX-1;
        }
        
    }

    //Cover old snake
    rect.top = snake[h].y*BLOCK_SIZE;
    rect.bottom = (snake[h].y+1)*BLOCK_SIZE;
    rect.right = (snake[h].x+1)*BLOCK_SIZE;
    rect.left = snake[h].x*BLOCK_SIZE;
    fill_rectangle(rect, BLACK);

    //Draw the outside border
    rect.top = 0;
    rect.bottom = (GRID_SIZE_Y)*BLOCK_SIZE;
    rect.right = (GRID_SIZE_X)*BLOCK_SIZE;
    rect.left = 0;
    draw_rectangle(rect, BLUE);

    D1_Z
    D0_H
    D0_R
}

