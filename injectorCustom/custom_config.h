// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CUSTOM_CONFIG_H
#define CUSTOM_CONFIG_H

#include "avr_util.h"
#include "lin_frame.h"
#include "sio.h"

// Implements the application specific configuration control. It exports a single 'enable'
// bit that is persisted in eeprom and can be toggled by pressing the button in a specific
// sequence:
// 1. Turn ignition on.
// 2. Click the Sport Mode button 6 times at a rate of about one click per second.
// 3. Turn ignition off.
// The entire sequence must be completed within 20 seconds, otherwise it is ignored.
//
// Like all the other custom_* files, this file should be adapted to the specific application. 
// The example provided is for a Sport Mode button press injector for 981/Cayman.
namespace custom_config {
  namespace private_ {
    // True when sport mode injection is enabled.
    extern boolean is_enabled;
    extern int SCV;
    extern byte crcByte;
  }

  // Called once during initialization.
  extern void setup();

  // Called once on each iteration of the Arduino main loop().
  extern void loop();
  
  inline boolean is_enabled() {
    return private_::is_enabled;
  }

  inline int whichSCV() {
    return private_::SCV;
  }

  inline byte crcCalc(byte frameCrc[]) {
   const byte generator = 0x12;
  //byte data[8];
  //data[1] = 10;
  //Serial.println(data[3],BIN);
 //byte data[] = {0x7d, 0x7d, 0xa4, 0xa4, 0xff,0xff,0xff};
 int len = 7;
    byte crc = 0; /* start with 0 so first byte can be 'xored' in */

    for (int i = 0; i < len; i++)
    {
        crc ^= frameCrc[i]; /* XOR-in the next input byte */
        for (int i = 0; i < 8; i++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (byte)((crc << 1) ^ generator);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
  }

}  // namespace custom_config

#endif

