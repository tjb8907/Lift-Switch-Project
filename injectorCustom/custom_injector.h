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

#ifndef CUSTOM_INJECTOR_H
#define CUSTOM_INJECTOR_H

#include "avr_util.h"
#include "custom_config.h"
#include "custom_defs.h"
#include "lin_frame.h"
#include "injector_actions.h"

// Controls signals injected into linbus framed passed between the master and the
// slave (set/reset selected data bits and adjust the checksum byte). 
//
// Like all the other custom_* files, this file should be adapted to the specific application. 
// The example provided is for a Sport Mode button press injector for 981/Cayman.
namespace custom_injector {
  
  // Private state of the injector. Do not use from other files.
  namespace private_ {
    // Target injection bit for 981CS Sport Mode button.
    static const uint8 kTargetedFrameId = 0xf0;
    static const uint8 kTargetedFrameDataBytes = 8;
    static const uint8 kTargetedByteIndexRight = 2;
    static const uint8 kTargetedByteIndexRight2 = 3;
    static const uint8 kTargetedByteIndexLeft = 0;
    static const uint8 kTargetedByteIndexLeft2 = 1;
    static const uint8 kTargetedCrcByteIndex = 7;
    
     extern boolean injections_enabled;
    
    // True if the current linbus frame is transformed by the injector. Othrwise, the 
    // frame is passed as is.
    extern boolean frame_id_matches;
    
    // Used to calculate the modified frame checksum.
    extern uint16 sum;
    
    // The modified frame checksum byte. 
    extern uint8 checksum;

   extern byte frameCRC[7];
  }
  
  // ====== These functions should be called from main thread only ================
  
  inline void setInjectionsEnabled(boolean enabled) {
    private_::injections_enabled = enabled;

  }
    
  // ====== These function should be called from lib_processor ISR only =============

  // Called when the id byte is recieved.
  // Called from lin_processor's ISR.
  inline void onIsrFrameIdRecieved(uint8 id) {
    private_::frame_id_matches = 
        private_::injections_enabled && (id == private_::kTargetedFrameId);
    // Linbus checksum V2 includes also the ID byte. 
    private_::sum = custom_defs::kUseLinChecksumVersion2 ? id : 0;
    private_::checksum = 0x00;
    for (int i = 0; i<8; i++) {
      private_::frameCRC[i] = 0x00;
    }
  }

  // Called when a data or checksum byte is sent (but not the sync or id bytes). 
  // The injector uses it to compute the modified frame checksum.
  // Called from lin_processor's ISR.
  inline void onIsrByteSent(uint8 byte_index, uint8 b) {
    // If this is not a frame we modify then do nothing.
    if (!private_::frame_id_matches) {
      return;
    }
    private_::frameCRC[byte_index] = b;
    // Collect the sum. Used later to compute the checksum byte.
    private_::sum += b;
    //CRC Calculator TODO
    if (byte_index == (private_::kTargetedFrameDataBytes - 2)) {
    
    }
    // If we just recieved the last data byte, compute the modified frame checksum.
    if (byte_index == (private_::kTargetedFrameDataBytes - 1)) {
      // Keep adding the high and low bytes until no carry.
      for (;;) {
        const uint8 highByte = (uint8)(private_::sum >> 8);
        if (!highByte) {
          break;  
        }
        // NOTE: this can add additional carry.  
        private_::sum = (private_::sum & 0xff) + highByte; 
      }
      private_::checksum = (uint8)(~private_::sum);
    }
  }

  // Called before sending a data bit of the the data or checksum bytes to get the 
  // transfer function for it.
  // byte_index = 0 for first data byte, 1 for second data byte, ...
  // bit_index = 0 for LSB, 7 for MSB.
  // Called from lin_processor's ISR.
  inline byte onIsrNextBitAction(uint8 byte_index, uint8 bit_index) {
    if (!private_::frame_id_matches) {
      return injector_actions::COPY_BIT;
    }
//    boolean atByte7High = false;
//    boolean atByte7Low = false;
    int SCVinj = custom_config::whichSCV();  
    // Handle a bit of one of the data bytes.
    if (byte_index < private_::kTargetedFrameDataBytes) {
      const boolean atSCVBit = (((SCVinj == 0 || SCVinj == 2) && ((byte_index == private_::kTargetedByteIndexLeft) || (byte_index == private_::kTargetedByteIndexLeft2))) || ((SCVinj == 1 || SCVinj == 2) && ((byte_index == private_::kTargetedByteIndexRight) || (byte_index == private_::kTargetedByteIndexRight2))));
      const boolean atByte7 = (byte_index == private_::kTargetedCrcByteIndex);
      byte crc = custom_config::crcCalc(private_::frameCRC);
 // Pre crcCalc Code to determine crc Bits
//      switch (SCVinj) {
//        case 0:
//            atByte7High = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 1 || bit_index == 3 || bit_index == 4 || bit_index == 7));
//            atByte7Low = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 2));
//            break;
//        case 1:
//            atByte7High = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 1 || bit_index == 4 || bit_index == 7));
//            atByte7Low = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 2 || bit_index == 5 || bit_index == 6));
//            break;
//
//        case 2:
//            atByte7High = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 3));
//            atByte7Low = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == 5 || bit_index == 6));
//            break;
//
//        default:
//         break;
//      }

    //    const boolean atByte7High = ((byte_index == private_::kTargetedByteIndex3) && (bit_index == private_::kTargetedBitIndex0));
    //    const boolean atByte7Low = (byte_index == private_::kTargetedByteIndex3);
          if (atSCVBit){
            return atSCVBit ? injector_actions::FORCE_BIT_0 : injector_actions::COPY_BIT;
          }
//          else if (atByte7) {
//            return injector_actions::FORCE_BIT_1;
//          }
//          else if (atByte7Low) {
//            return injector_actions::FORCE_BIT_0;
//          }
          else if (atByte7 && bit_index > 0) {
            return bitRead(crc, bit_index) ? injector_actions::FORCE_BIT_1 : injector_actions::FORCE_BIT_0;
          }
          else{
            return injector_actions::COPY_BIT;
          }
    }
    
    // Handle a checksum bit.
    const boolean checksumBit = private_::checksum & bitMask(bit_index);
    return checksumBit ? injector_actions::FORCE_BIT_1 : injector_actions::FORCE_BIT_0;
    
    // TODO: handle the unexpected case of more than 6 + 1 bytes in the frame. For
    // now we will repeat the checksum byte blindly.
  }  
inline byte crcCalc(byte b) {
  return b;
}

}  // namepsace custom_injector

#endif


