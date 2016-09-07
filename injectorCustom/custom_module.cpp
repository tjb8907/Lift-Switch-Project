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

#include "custom_module.h"

#include "custom_config.h"
#include "custom_injector.h"
#include "custom_signals.h"
#include "io_pins.h"
#include "leds.h"
#include "signal_tracker.h"
#include "sio.h"

// Like all the other custom_* files, this file should be adapted to the specific application. 
// The example provided is for a Sport Mode button press injector for 981/Cayman.
namespace custom_module {
  
// A single byte enum representing the states.
namespace states {
  static const uint8 IGNITION_OFF_IDLE = 0;
  static const uint8 IGNITION_ON_MAYBE_INJECT = 1;
  static const uint8 IGNITION_ON_INJECT = 2;
  static const uint8 IGNITION_ON_IDLE = 3;
  static uint8 REVERSE = 2;
}

// The current state. One of states:: values. 
static uint8 state;
boolean notReverse = 1; 
// Tracks since change to current state.
static PassiveTimer time_in_state;
int revPin1 = 10;
static const uint16_t injectionTime = 500;
static const uint16_t waitToInject = 200;
  
static inline void changeToState(uint8 new_state) {
  state = new_state;
  //sio::printf(F("injection state: %d\n"), state);
  // We assume this is a new state and always reset the time in state.
  time_in_state.restart();
}

void setup() {
  pinMode(revPin1,INPUT);
  custom_signals::setup();
  custom_config::setup();
  changeToState(states::IGNITION_OFF_IDLE);
  //pinMode(12,INPUT_PULLUP);
}
   
// Called periodically from loop() to update the state machine.
static inline void updateState() {
  // Meta rule: inject IFF in INJECT state.
  {
    const boolean injecting = (state == states::IGNITION_ON_INJECT);
    //sio::printf(F("injecting: %d\n"), injecting);
    custom_injector::setInjectionsEnabled(injecting);
    leds::status.set(injecting);
  }
   
  // Meta rule: if ignition is known to be off, reset to IGNITION_OFF_IDLE state.
  if (custom_signals::ignition_state().isOff()) {
    if (state != states::IGNITION_OFF_IDLE) {
      changeToState(states::IGNITION_OFF_IDLE);
    }  
    return;
  }

  if (digitalRead(revPin1) == LOW) {
    //sio::printf(F("11 is high \n"));
    notReverse = 1;
  }
  // Handle the state transitions.
  switch (state) {
    case states::IGNITION_OFF_IDLE:
    //sio::printf(F("injection state: OFF_IDLE (%d)"), state);
     // if (custom_signals::ignition_state().isOnForAtLeastMillis(1000)) {
     //states::REVERSE = digitalRead(11);
     //sio::printf(F("Reverse State is: %d\n"), states::REVERSE);
     if (digitalRead(revPin1)==HIGH && notReverse == 1 && time_in_state.timeMillis() > 500) {
        sio::printf(F("injection state: ON_MAYBE_INJECT (%d)\n"), state);
        notReverse = 0;
        changeToState(states::IGNITION_ON_MAYBE_INJECT);  
      }
      break;
      
    case states::IGNITION_ON_MAYBE_INJECT:
    
    if (time_in_state.timeMillis() > waitToInject) {
      sio::printf(F("injection state: ON_INJECT (%d)\n"), state);
      changeToState(states::IGNITION_ON_INJECT);  
    }
    else if (digitalRead(revPin1) == LOW && time_in_state.timeMillis() > 100) {
      sio::printf(F("injection state: OFF_IDLE (%d)\n"), state);
      changeToState(states::IGNITION_OFF_IDLE);
    }
      return;
    
    case states::IGNITION_ON_INJECT:
      
      // NOTE: the meta rule at the begining of this method enables injection
      // as long as state is INJECT. We don't need to control injection here.
      if (time_in_state.timeMillis() > injectionTime || digitalRead(revPin1)==LOW) {
        sio::printf(F("injection state: OFF_IDLE (%d)\n"), state);
        changeToState(states::IGNITION_OFF_IDLE);
      }
//      if (digitalRead(11)== HIGH) {
//        changeToState(states::IGNITION_OFF_IDLE);
//      }
      break;
    
    case states::IGNITION_ON_IDLE:
      //sio::printf(F("injection state: ON_IDLE (%d)"), state);
      // Nothing to do here. Stay in this state until ignition is
      // turned off.
      break;
    
    // Unknown state, set to initial.
    default:
      sio::printf(F("injection state: unknown (%d)"), state);
      changeToState(states::IGNITION_OFF_IDLE);
      break;
  } 
}
  
void loop() {
  // Update dependents.
  // custom_signals::loop();
  //custom_config::loop();
  
  // Update the state machine
  updateState();
}

void frameArrived(const LinFrame& frame) {
  // Track the signals in this frame.
  custom_signals::frameArrived(frame);
  
  // Report an error if the Sport Mode assembly does not respond as expected
  // to its frame.
  if (frame.get_byte(0) == 0x8e) {
    if (frame.num_bytes() != (1 + 8 + 1)) {
      leds::errors.action();
      sio::println(F("slave error")); 
    }
  }
}

}  // namespace custom_module


