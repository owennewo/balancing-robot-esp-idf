
#include "LEDTask.h"

#include "FlySkyI6Task.h"

#define LED_GPIO (GPIO_NUM_2)

static const char *TASK_TAG = "LEDTask";

LEDTask::LEDTask() : Task(TASK_TAG, 1024, 10) {}

void LEDTask::run(void *data) {
  xTaskCreate(loop, m_taskName.c_str(), 2048 * 2, NULL, configMAX_PRIORITIES,
              NULL);
}

void LEDTask::loop(void *arg) {
  gpio_pad_select_gpio(LED_GPIO);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  static int status = 0;

  struct FlySkySample sample;
  while (1) {
    status = !status;
    gpio_set_level(LED_GPIO, status);
    vTaskDelay(pdMS_TO_TICKS(100));

    if (xQueuePeek(FlySkyI6Task::xQueueSample, &sample, pdMS_TO_TICKS(10))) {
      printf("LED received %d %d %d %d %d %d %d %d %d %d\n", sample.throttle,
             sample.yaw, sample.pitch, sample.roll, sample.twist1,
             sample.twist2, sample.switch1, sample.switch2, sample.switch3,
             sample.switch4);
    }
  }
}
