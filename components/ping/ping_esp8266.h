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

#ifdef ARDUINO_ARCH_ESP8266
#include "AsyncPing.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ping {

enum ping_esp8266_request {
  EACH_RESULT = true,
  END = false,
};

class PingSensorESP8266 : public PingSensor {
 public:
  void setup() override;
  void update() override;

 protected:
  uint32_t total_success_time;
  void reset() { this->total_success_time = 0; }
  void incr_total_success_time(int time) { this->total_success_time += time; }

  AsyncPing ping;
};

}  // namespace ping
}  // namespace esphome
#endif
