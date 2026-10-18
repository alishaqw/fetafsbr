#include "FtFsBrowser.h"

void setup() {
  Serial.begin(115200);
  initfsb("MyESP32", "12345678");
}

void loop() {}