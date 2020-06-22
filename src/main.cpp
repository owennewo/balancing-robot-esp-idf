#include <esp_log.h>
#include <esp_system.h>

#include "FlySkyI6Task.h"
#include "LEDTask.h"

FlySkyI6Task flysky;
LEDTask led;

extern "C" {
void app_main(void);
}

void app_main(void) {
  flysky.start();
  led.start();
}
