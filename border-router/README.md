# ThreadNet Border Router

### Running:

Prerequisites:
- Requires an ESP Thread Border Router device.
- Install the [ESP-IDF v5.2.1](https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32/versions.html) SDK.

Running the border router:

1) Connect the border router device to your computer via the USB2 port on the border router device.
2) Determine the port of the border router.
3) Run the command `idf.py -p <port> build flash monitor` 

Note: any combination of build, flash, and monitor can be used depending on your desired outcome.
