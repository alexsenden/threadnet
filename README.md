# ThreadNet

### What is ThreadNet?

ThreadNet is a tool used to view the topology, node metadata, and network performance at a per-node and network-wide scale within [Thread networks](https://en.wikipedia.org/wiki/Thread_(network_protocol)).

ThreadNet was created as a project for `COMP 4300 - Computer Networks` at the University of Manitoba, and was created by Alex Senden and Landon Colburn.

For more information about ThreadNet, including how it works and how it was tested, please read the ThreadNet project report.

### Hardware

ThreadNet requires 2 specific pieces of hardware, to create the network, and requires a third device (computer capable of connecting to a Wi-Fi network) to interact with the Thread network via the Next.js web interface.

The ThreadNet border router runs on a ESP Thread Border Router, which is a combination of the ESP32-S3 and ESP32-H2 on the same PCB.

The ThreadNet end devices run on ESP32-C6 devices.
