/*
 * Copyright (c) 2021 Tomoyuki Sakurai <y@trombik.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#ifdef ARDUINO_ARCH_ESP32
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "ping_sock.h"

#include "esphome/components/sensor/sensor.h"

#define TAG "ping_esp32"

namespace esphome {
namespace ping {
class PingSensorESP32 : public PingSensor {
 public:
  void setup() override;
  void update() override;

 private:
  static void cb_ping_on_ping_success(esp_ping_handle_t hdl, void *context) {
    reinterpret_cast<PingSensorESP32 *>(context)->cmd_ping_on_ping_success(hdl);
  }

  static void cb_cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *context) {
    reinterpret_cast<PingSensorESP32 *>(context)->cmd_ping_on_ping_end(hdl);
  }

  static void cb_cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *context) {
    reinterpret_cast<PingSensorESP32 *>(context)->cmd_ping_on_ping_timeout(hdl);
  }

 protected:
  void cmd_ping_on_ping_success(esp_ping_handle_t hdl);
  void cmd_ping_on_ping_end(esp_ping_handle_t hdl);
  void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl);

  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  esp_ping_handle_t ping;
  esp_ping_callbacks_t cbs;

  void init_ping();
  uint32_t total_success_time;
  void reset() { this->total_success_time = 0; }
  void incr_total_success_time(int time) { this->total_success_time += time; }
};

}  // namespace ping
}  // namespace esphome
#endif
