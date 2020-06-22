#include "Task.h"
#ifndef FLYSKYI6TASK_H_
#define FLYSKYI6TASK_H_

#include <esp_log.h>

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct FlySkySample {
  int16_t throttle;  // left vertical
  int16_t yaw;       // left horizontal
  int16_t pitch;     // right vertical
  int16_t roll;      // right horizontal
  int16_t twist1;
  int16_t twist2;
  int8_t switch1;
  int8_t switch2;
  int8_t switch3;
  int8_t switch4;
};

template <typename T>
T map(T x, T x1, T x2, T y1, T y2) {
  return (x - x1) * (y2 - y1) / (x2 - x1) + y1;
}

inline int16_t mapAnalog(int16_t value) {
  return map(static_cast<int>(value), 1000, 2000, -500, 500);
}

inline int8_t mapDigital(int16_t value) {
  return static_cast<int8_t>(map(static_cast<int>(value), 1000, 2000, -1, 1));
}

class FlySkyI6Task : public Task {
 public:
  FlySkyI6Task();
  static QueueHandle_t xQueueSample;

 private:
  void run(void *data);  // { printf("do the flysky"); }
  static void loop(void *arg);
  static void resync(uint8_t *data);
  static FlySkySample parse(uint8_t *data);
};

#endif
