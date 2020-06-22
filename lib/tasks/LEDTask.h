#ifndef LEDTASK_H_
#define LEDTASK_H_
#include <esp_log.h>

#include "Task.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class LEDTask : public Task {
 public:
  LEDTask();

 private:
  void run(void *data);
  static void loop(void *arg);
};

#endif
