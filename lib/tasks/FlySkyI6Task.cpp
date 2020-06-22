#include "FlySkyI6Task.h"

#define RXD_PIN 16
#define TXD_PIN 17
#define RX_BUF_SIZE 96
#define PROTOCOL_CHANNELS 10

#define SYNC_BYTE 0x20
#define COMMAND_BYTE 0x40
#define TASK_SLEEP_MILLISECONDS 3
#define READ_TIMEOUT 100

static const char *TASK_TAG = "FlySkyI6Task";

QueueHandle_t FlySkyI6Task::xQueueSample =
    xQueueCreate(1, sizeof(FlySkySample));

FlySkyI6Task::FlySkyI6Task() : Task(TASK_TAG, 4096, 10) {}

void FlySkyI6Task::run(void *data) {
  const uart_config_t uart_config = {.baud_rate = 115200,
                                     .data_bits = UART_DATA_8_BITS,
                                     .parity = UART_PARITY_DISABLE,
                                     .stop_bits = UART_STOP_BITS_1,
                                     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                     .rx_flow_ctrl_thresh = 0,
                                     .use_ref_tick = 0};

  uart_param_config(UART_NUM_1, &uart_config);
  uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);

  xTaskCreate(loop, m_taskName.c_str(), 1024 * 2, NULL, configMAX_PRIORITIES,
              NULL);
}

void FlySkyI6Task::loop(void *arg) {
  esp_log_level_set(TASK_TAG, ESP_LOG_INFO);

  uint8_t data[RX_BUF_SIZE + 1];
  while (1) {
    const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE,
                                        pdMS_TO_TICKS(READ_TIMEOUT));
    if (rxBytes > 0) {
      if (data[0] == SYNC_BYTE && data[1] == COMMAND_BYTE) {
        FlySkySample sample = parse(data);
        xQueueOverwrite(xQueueSample, &sample);
      } else {
        xQueueReset(xQueueSample);
        resync(data);
      }
    } else {
      ESP_LOGD(TASK_TAG, "Not data");
    }
    vTaskDelay(pdMS_TO_TICKS(TASK_SLEEP_MILLISECONDS));
  }  // while
}

// A sample is 32 bytes.  We need to find byte 0 in the stream!
void FlySkyI6Task::resync(uint8_t *data) {
  ESP_LOGI(TASK_TAG, "Not sync'd - looking for magic numbers: '%d' and '%d",
           SYNC_BYTE, COMMAND_BYTE);
  for (int i = 0; i <= 32; i++) {
    if (data[i] == SYNC_BYTE && data[i + 1] == COMMAND_BYTE) {
      const int rxBytes =
          uart_read_bytes(UART_NUM_1, data, i, pdMS_TO_TICKS(READ_TIMEOUT));
      ESP_LOGI(TASK_TAG, "Sync by shifting stream %d bytes", rxBytes);
      break;
    }
  }
}

FlySkySample FlySkyI6Task::parse(uint8_t *data) {
  int16_t channel[PROTOCOL_CHANNELS];
  for (u_int8_t i = 0; i < PROTOCOL_CHANNELS; i++) {
    channel[i] = data[2 + i * 2] | (data[3 + i * 2] << 8);
  }

  // ESP_LOG_BUFFER_HEXDUMP(TASK_TAG, data, rxBytes, ESP_LOG_INFO);
  return FlySkySample{
      .throttle = mapAnalog(channel[0]),
      .yaw = mapAnalog(channel[1]),    // left horizontal
      .pitch = mapAnalog(channel[2]),  // right vertical
      .roll = mapAnalog(channel[3]),   // right horizontal
      .twist1 = mapAnalog(channel[4]),
      .twist2 = mapAnalog(channel[5]),
      .switch1 = mapDigital(channel[6]),
      .switch2 = mapDigital(channel[7]),
      .switch3 = mapDigital(channel[8]),
      .switch4 = mapDigital(channel[9]),
  };
}