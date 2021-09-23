#include "ping_esp8266.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ping {

static const char *const TAG = "PingSensorESP32";

void PingSensorESP32::setup() override {
  init_ping();
  esp_ping_new_session(&this->ping_config, &this->cbs, &this->ping);
  esp_ping_start(this->ping);
}

void PingSensorESP32::update() override {
  float loss = this->get_latest_loss();
  uint32_t latency_ms = this->get_latest_latency();

  if (loss >= 0 && this->packet_loss_sensor_ != nullptr) {
    packet_loss_sensor_->publish_state(loss);
  }
  if (latency_ms >= 0 && this->latency_sensor_ != nullptr) {
    latency_sensor_->publish_state((float) latency_ms / 1000);
  }
  esp_ping_new_session(&this->ping_config, &this->cbs, &this->ping);
}

void PingSensorESP32::cmd_ping_on_ping_success(esp_ping_handle_t hdl) {
  uint8_t ttl;
  uint16_t seqno;
  uint32_t elapsed_time, recv_len;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
  ESP_LOGI(TAG, "%d bytes from %s icmp_seq=%d ttl=%d time=%d ms", recv_len, ipaddr_ntoa((ip_addr_t *) &target_addr),
           seqno, ttl, elapsed_time);
  this->incr_total_success_time(elapsed_time);
}

void PingSensorESP32::cmd_ping_on_ping_end(esp_ping_handle_t hdl) {
  ip_addr_t target_addr;
  uint32_t transmitted;
  uint32_t received;
  uint32_t total_time_ms;
  uint32_t mean = 0;

  esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
  esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
  uint32_t loss = (uint32_t) ((1 - ((float) received) / transmitted) * 100);
  if (received != 0) {
    mean = this->total_success_time / received;
  }

  if (IP_IS_V4(&target_addr)) {
    ESP_LOGI(TAG, "--- %s ping statistics ---", inet_ntoa(*ip_2_ip4(&target_addr)));
  } else {
    ESP_LOGI(TAG, "--- %s ping statistics ---", inet6_ntoa(*ip_2_ip6(&target_addr)));
  }
  ESP_LOGI(TAG, "%d packets transmitted, %d received, %d%% packet loss, total time %dms avg time %dms", transmitted,
           received, loss, total_time_ms, mean);

  this->set_latest_loss(loss);
  this->set_latest_latency(mean);
  this->reset();
  esp_ping_delete_session(hdl);
}

void PingSensorESP32::cmd_ping_on_ping_timeout(esp_ping_handle_t hdl) {
  uint16_t seqno;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  ESP_LOGI(TAG, "From %s icmp_seq=%d timeout", ipaddr_ntoa((ip_addr_t *) &target_addr), seqno);
}

void PingSensorESP32::init_ping() {
  ip_addr_t target_addr;
  int8_t err;

  memset(&target_addr, 0, sizeof(target_addr));
  err = inet_pton(AF_INET, this->target.c_str(), &target_addr);
  if (err == 0) {
    ESP_LOGE(TAG, "invalid address: `%s`", this->target.c_str());
    this->status_set_warning();
    return;
  } else if (err < 0) {
    ESP_LOGE(TAG, "inet_pton(): %s", esp_err_to_name(err));
    this->status_set_warning();
    return;
  }

  this->ping_config.target_addr = target_addr;
  this->ping_config.count = this->n_packet;

  this->cbs.on_ping_success = PingSensorESP32::cb_ping_on_ping_success;
  this->cbs.on_ping_timeout = PingSensorESP32::cb_cmd_ping_on_ping_timeout;
  this->cbs.on_ping_end = PingSensorESP32::cb_cmd_ping_on_ping_end;
  this->cbs.cb_args = this;
}

}  // namespace ping
}  // namespace esphome
