#include "frisquet_boiler.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#define BOILERMODE_2_STR(mode) (mode == 0 ? "eco" : (mode == 3 ? "comfort" : (mode == 4 ? "frost protection" : "INVALID")))
#define LOG_TAG "boiler_command"

extern esphome::template_::TemplateNumber *max_setpoint;
extern esphome::template_::TemplateSwitch *control_state;

namespace esphome
{
  namespace frisquet_boiler
  {
    const int DELAY_BETWEEN_FRAMES_MS = 33;
    const int LONG_PULSE = 825;

    void FrisquetBoiler::set_boiler_id(const char *str)
    {
      esphome::parse_hex(str, this->boilerID, 2);
    }

    // dump_config() writes all configuration to "config" logs
    // => config logs are shown each time console connection is established
    //    (OTA or through USB direct connection)
    void FrisquetBoiler::dump_config()
    {
      ESP_LOGCONFIG(LOG_TAG, "Frisquet Boiler:");
      ESP_LOGCONFIG(LOG_TAG, "  Boiler id : 0x%02X%02X", this->boilerID[0], this->boilerID[1]);
      ESP_LOGCONFIG(LOG_TAG, "  Wait time before sending first message to boiler : %d sec", this->initDelai_ms / 1000);
      ESP_LOGCONFIG(LOG_TAG, "  Max period between two messages sent to boiler : %d sec", this->repeatDelai_ms / 1000);
      ESP_LOGCONFIG(LOG_TAG, "  Wait time before a new received setpoint is sent to boiler : %d sec", this->newSetpointDelai_ms / 1000);
      ESP_LOGCONFIG(LOG_TAG, "  In case setpoints are not longer received, delai before stopping the boiler (safety): %d sec", this->timeToDeath_ms / 1000);
      ESP_LOGCONFIG(LOG_TAG, "  Maximum setpoint value: %d", this->maxSetpoint);
    }

    void FrisquetBoiler::setup()
    {
      // Setup GPIO pin
      this->boilerOutPIN->setup();
      this->boilerOutPIN->digital_write(0);
      this->previousBitSent = 0;
      this->set_boiler_id_in_message();
      this->lastSetpointReceived = 0;
      this->bReceivedNewSetpoint = false;
      this->boilerMode = 3;
      this->enabledState = true;

      this->bReceivedNewSetpoint = false;
      this->lastTimeReceivedSetpoint = millis();
      this->isMqttClientDead = false;

      this->lastTimeSent = millis();
      this->firstMsgSent = false;
      this->bitstuff_counter = 0;

      // Restoring data read from flash
      this->set_max_setpoint(max_setpoint->state);
      //this->set_control_state(control_state->state);
    }

    void FrisquetBoiler::set_control_state(bool value)
    {
      this->enabledState = value;
      this->lastTimeReceivedSetpoint = 0;
      this->bReceivedNewSetpoint = true;
    }

    void FrisquetBoiler::set_boiler_id_in_message()
    {
      this->boilerFrame[3] = this->boilerID[0];
      this->boilerFrame[4] = this->boilerID[1];
    }

    void FrisquetBoiler::write_state(float state)
    {
      if (state >= 0. && state <= 1.)
      {
        int oldSetpoint = this->lastSetpointReceived;
        int newSetpoint = int(state * 100);
        this->lastSetpointReceived = __min(newSetpoint, this->maxSetpoint);
        bool bChanged = (oldSetpoint != this->lastSetpointReceived);
        if (newSetpoint != this->lastSetpointReceived)
          ESP_LOGD(LOG_TAG, "Received setpoint : %d %% => %d %% (%s)", newSetpoint, this->lastSetpointReceived, bChanged?"new":"no change");
        else
          ESP_LOGD(LOG_TAG, "Received setpoint : %d %% (%s)", this->lastSetpointReceived, bChanged?"new":"no change");
        this->lastTimeReceivedSetpoint = millis();
        if (this->isMqttClientDead)
        {
          ESP_LOGI(LOG_TAG, "Exiting safety state");
          this->isMqttClientDead = false;
        }
        this->bReceivedNewSetpoint = bChanged;
      }
      else
      {
        ESP_LOGE(LOG_TAG, "INVALID SETPOINT RECEIVED : %.1f", state);
      }
    }

    void FrisquetBoiler::loop()
    {
      bool bSend = false;
      if (this->firstMsgSent == false)
        // For the first message, we wait initDelaiMs ms
        bSend = millis() - lastTimeSent >= this->initDelai_ms;
      else
      {
        // We send a message to boiler every MAX_MSG_PERIOD_MS ms if nothing new
        // or after DELAI_BEFORE_SETPOINT_MSG_MS ms if a new setpoint has arrived
        // or when nothing came from MQTT for long time (overheating protection)
        if (!this->isMqttClientDead && (millis() - this->lastTimeReceivedSetpoint >= this->timeToDeath_ms))
        {
          // Overheating protection
          ESP_LOGW(LOG_TAG, "Client has not sent any setpoint for too long; Stopping the boiler");
          bSend = true;
          this->isMqttClientDead = true;
          this->lastSetpointReceived = 0;
          // mqttPublishState();
        }
        else
          bSend = (millis() - this->lastTimeSent >= this->repeatDelai_ms) || (this->bReceivedNewSetpoint && (millis() - this->lastTimeReceivedSetpoint >= this->newSetpointDelai_ms));
      }

      if (bSend)
      {
        this->sendMessagetoBoiler();
        this->firstMsgSent = true;
        this->bReceivedNewSetpoint = false;
        this->lastTimeSent = millis();
      }
    }

    void FrisquetBoiler::sendMessagetoBoiler()
    {
      int setpoint = this->enabledState ? this->lastSetpointReceived : 0;
      if (this->enabledState)
        ESP_LOGI(LOG_TAG, "Sending frames to boiler for mode<%s> and setpoint<%d>", BOILERMODE_2_STR(boilerMode), setpoint);
      else
        ESP_LOGI(LOG_TAG, "[enabledState=false] Sending frames to boiler for mode<%s> and setpoint<%d>", BOILERMODE_2_STR(boilerMode), setpoint);
      for (uint8_t f_idx = 0; f_idx < 3; f_idx++)
      {
        previousBitSent = 0;
        boilerFrame[8] = f_idx;
        boilerFrame[9] = (f_idx == 2) ? boilerMode : boilerMode + 0x80;
        boilerFrame[10] = setpoint;

        int checksum = 0;
        for (uint8_t i = 3; i < 12; i++)
          checksum -= boilerFrame[i];
        boilerFrame[12] = (uint8_t)((checksum) >> 8); // highbyte
        boilerFrame[13] = (uint8_t)((checksum)&0xff); // lowbyte

        // Now we send the frame to the boiler
        for (int i = 0; i < 16; i++)
          sendByteToBoiler(boilerFrame[i], i);
        this->boilerOutPIN->digital_write(0);
        delay(DELAY_BETWEEN_FRAMES_MS);
      }
      this->boilerOutPIN->digital_write(0);
    }

    void FrisquetBoiler::sendByteToBoiler(uint8_t byteValue, uint8_t byteIndex)
    {
      /**
       * @brief Send given byte to the boiler
       * @param byteValue byte value to send
       * @param byteIndex Index of the byte in the message, used for bit stuffing
       */

      for (uint8_t bit = 0; bit < 8; bit++)
      {
        int bitValue = ((byteValue >> bit) & 0x01);
        sendBitToBoiler(bitValue);

        // bit stuffing only applicable to the data part of the message (bits 3 to 13)
        // increment bitstuffing counter if bitValue == 1
        if (byteIndex >= 3 && byteIndex <= 13 && bitValue == 1)
          bitstuff_counter++;

        // reset bitstuffing counter
        if (bitValue == 0)
          bitstuff_counter = 0;

        if (bitstuff_counter >= 5)
        {
          // After 5 consecutive '1', insert a '0' bit (bitstuffing) and reset counter
          sendBitToBoiler(0);
          bitstuff_counter = 0;
        }
      }
    }

    void FrisquetBoiler::sendBitToBoiler(bool bit)
    {
      previousBitSent = !previousBitSent;
      this->boilerOutPIN->digital_write(previousBitSent);
      delayMicroseconds(LONG_PULSE);

      // if bit == 1, put transition
      if (bit)
      {
        previousBitSent = !previousBitSent;
        this->boilerOutPIN->digital_write(previousBitSent);
      }

      delayMicroseconds(LONG_PULSE);
    }

  } // namespace frisquet_boiler
} // namespace esphome
