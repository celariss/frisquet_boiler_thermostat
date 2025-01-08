#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome.h"

#define __min(a,b) (((a) < (b)) ? (a) : (b))
#define __max(a,b) (((a) > (b)) ? (a) : (b))


namespace esphome
{
    namespace frisquet_boiler
    {
        class FrisquetBoiler : public output::FloatOutput, public Component
        {
        public:
            // Configuration methods
            void set_pin(GPIOPin *pin) { this->boilerOutPIN = pin; }
            void set_boiler_id(const char *str);
            void set_init_delai(int value) { this->initDelai_ms = value*1000;}
            void set_repeat_delai(int value) { this->repeatDelai_ms = value*1000;}
            void set_new_setpoint_delai(int value) { this->newSetpointDelai_ms = value*1000;}
            void set_time_to_death(int value) { this->timeToDeath_ms = value*1000;}
            void set_max_setpoint(int value) { this->maxSetpoint = __max(0,__min(100,value)); }
            void set_control_state(bool value);

            void dump_config() override;
            void setup() override;
            void loop() override;

            // "state" : value this output should be on, from 0.0 to 1.0
            void write_state(float state) override;

            // get values
            bool get_enabled_state() { return this->enabledState; }
            int get_max_setpoint() { return this->maxSetpoint; }
            int get_current_setpoint() {return enabledState?this->lastSetpointReceived:0;}

        protected:
            void set_boiler_id_in_message();
            void sendMessagetoBoiler();
            void sendByteToBoiler(uint8_t byteValue, uint8_t byteIndex);
            void sendBitToBoiler(bool bit);

        protected:
            // Configuration
            ////////////////////////////
            // Board PIN to use to send frames
            GPIOPin *boilerOutPIN;
            // ID of the boiler to be regulated
            uint8_t boilerID[2];
            // Boiler operating mode : 0 = eco / 3 = comfort / 4 = frost protection
            int boilerMode;
            // Wait time (ms) before sending first command to the boiler
            int initDelai_ms;
            // Max duration (ms) between 2 messages sent to the boiler
            int repeatDelai_ms;
            // Wait time (ms) before sending a new setpoint received from client
            int newSetpointDelai_ms;
            // Delai (ms) before stopping the boiler, in case of no client activity (no message received)
            int timeToDeath_ms;
            // Max allowed value for setpoint
            int maxSetpoint;
            // If false, the boiler setpoint is set to 0
            bool enabledState;

            // Variables
            ///////////////////////////////
            // Setpoint for boiler, from 0 to 100 (%)
            int lastSetpointReceived;
            // Indicates that a new setpoint has been received and must be sent to the boiler
            bool bReceivedNewSetpoint;
            // Last time a setpoint has been received
            uint32_t lastTimeReceivedSetpoint;
            // true when the client is considered has dead (no activity)
            bool isMqttClientDead;
            // Last time a frame has been sent to the boiler
            uint32_t lastTimeSent;
            // true if a frame has been sent since last reboot
            bool firstMsgSent;

            int bitstuff_counter; // TBD, still works inside sendByteToBoiler() ?
            int previousBitSent;

            // Boiler frame (16 bytes)
            // Format :
            //   bytes 0-1   : 0x00
            //   byte  2     : 0x7E
            //   bytes 3-4   : boiler id
            //   bytes 5-7   : 0x00, 0x20, 0x00
            //   byte  8     : frame index (0-2)
            //   byte  9     : mode + 0x80 ( 0 = eco / 3 = comfort / 4 = frost protection)
            //   byte  10    : setpoint
            //   byte  11    : 0x00
            //   bytes 12-13 : checksum of bytes 3-11
            //   bytes 14-15 : 0xFF, 0x80
            uint8_t boilerFrame[16] = {0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x80};
        };
    } // namespace frisquet_boiler
} // namespace esphome
