/*
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
*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "RIOS.h"


#define MAX_TASK_NUMBER 2

task tasks[MAX_TASK_NUMBER];
unsigned char tasksNum = 0;

uint8_t activeProcesses = 0;

//const double tick_ms = 1.0;  /* Real time between ticks in ms */
const unsigned long tasksPeriodGCD = 1;  // Timer tick rate


void init_LED();

void button_task(void);
void draw_task(void);

unsigned char runningTasks[MAX_TASK_NUMBER] = {255}; // Track running tasks, [0] always idleTask
const unsigned long idleTask = 255; // 0 highest priority, 255 lowest
unsigned char currentTask = 0; // Index of highest priority task in runningTasks


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

void init_processor(double tick_ms) {
    
    /* Configure 16 bit Timer for ISR  */
    TCCR1B = _BV(WGM12)   /* Clear Timer on Compare Match Mode (CTC) */
          | _BV(CS12)
          | _BV(CS10); 	 /* F_CPU / 1024 */

    OCR1A = (uint16_t)(F_CPU * tick_ms / (1024.0 * 1000)- 0.5);

    TIMSK1 = _BV(OCIE1A); //enables compare match interrupt
    TCNT1 = 0;
         
    sei();
}

void addTask(task t)
{
    tasks[tasksNum] = t;
    tasksNum++;
}
