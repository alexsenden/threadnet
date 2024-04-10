# ThreadNet End Device

### Running:

Prerequisites:
- Requires an ESP32-C6 device.
- Install the [ESP-IDF v5.2.1](https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32/versions.html) SDK.

Running the end device:

1) Connect the ESP32-C6 to your computer via the port labelled `USB` on the ESP32-C6.
2) Determine the port of the end device on your computer.
3) Run the command `idf.py -p <port> build flash monitor` 

Note: any combination of build, flash, and monitor can be used depending on your desired outcome.
