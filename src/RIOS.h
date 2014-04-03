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

typedef struct task {
   unsigned char running;       // 1 indicates task is running
   unsigned long period;        // Rate at which the task should tick
   unsigned long elapsedTime;   // Time since task's previous tick
   void (*TickFct)(void);       // Function to call for task's tick
} task;

void init_processor(double tick_ms);
void addTask(task t);

