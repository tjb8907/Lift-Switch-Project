/*
 Copyright 2011 G. Andrew Stone
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* This example code uses 2 different LIN LEDs of unknown type (acquired as 
   samples).  You will have to modify this code to work with whatever LIN
   devices you have.

   TODO: make a Arduino LIN slave so the code works without modification.
 */

#include "Arduino.h"
#include "lin.h"

Lin lin;

#ifndef SIM
#define simprt p
#undef assert  // I don't want to assert in embedded code
#define assert
#else
#define simprt printf
#include <stdarg.h>
#include <assert.h>
#endif

#ifdef LIGHTUINO
// Printf-style to the USB
void p(char *fmt, ... )
{
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Usb.print(tmp);
}
#else  // We can't use the serial on a normal Arduino because it is being used by LIN
void p(char *fmt, ... )
{
}
#endif

uint8_t frame[] = { 0x7d, 0x7d, 0x00, 0x00, 0xff ,0xff, 0xff, 0x92 };
uint8_t frame2[] = {0x7d , 0x7d, 0x00, 0x00, 0xff ,0xff, 0xff, 0x93 };

void setup(void)
{
  pinMode(10, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  lin.begin(19200);
  //Usb.begin();
}



 void loop(void)

{
if (digitalRead(10)== LOW) {
  digitalWrite(13, HIGH);
  lin.send(0xf0, frame,8,2);
  delay(25);
  digitalWrite(13, LOW);
  lin.send(0xf0, frame2, 8,2);
  delay(25);
  
  }
else {
  digitalWrite(13, LOW);
}
}
