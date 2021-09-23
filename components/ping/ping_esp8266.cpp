#include "ping_esp8266.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ping {

static const char *const TAG = "PingSensorESP8266";

void PingSensorESP8266::setup() {
  ping.on(EACH_RESULT, [this](const AsyncPingResponse &response) {
    if (response.answer) {
      ESP_LOGI(TAG, "%d bytes from %s: icmp_seq=%d ttl=%d time=%d ms", response.size, target.c_str(), response.icmp_seq,
               response.ttl, response.time);

      this->incr_total_success_time(response.time);
    } else {
      ESP_LOGI(TAG, "no reply from %s", target.c_str());
    }
    return false;
  });

  /* at the end, set the result */
  ping.on(END, [this](const AsyncPingResponse &response) {
    float loss = 0;
    uint32_t total_failed_count = response.total_sent - response.total_recv;
    if (response.total_sent != 0) {
      loss = (float) total_failed_count / response.total_sent;
    }

    uint32_t mean = 0;
    if (response.total_recv != 0) {
      mean = total_success_time / response.total_recv;
    }

    this->set_latest_loss(loss * 100);
    this->set_latest_latency(mean);

    ESP_LOGI(TAG, "packet loss: %0.1f %% latency: %d ms", loss * 100, mean);
    this->reset();
    return true;
  });
  ping.begin(target.c_str(), n_packet, timeout);
}

void PingSensorESP8266::update() {
  float loss;
  uint32_t latency_ms;

  loss = this->get_latest_loss();
  latency_ms = this->get_latest_latency();

  if (loss >= 0 && this->packet_loss_sensor_ != nullptr) {
    packet_loss_sensor_->publish_state(loss);
  }
  if (latency_ms >= 0 && this->latency_sensor_ != nullptr) {
    latency_sensor_->publish_state((float) latency_ms / 1000);
  }
  ping.begin(target.c_str(), n_packet, timeout);
}

}  // namespace ping
}  // namespace esphome
