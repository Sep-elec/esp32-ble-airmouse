# ESP32 BLE Air Mouse

This project turns an ESP32 + MPU6050 into a BLE air mouse.



## Features

* Mouse movement via MPU6050
* Left / Right click buttons
* Scroll control
* Double click
* Hold and drag
* BLE (no USB needed)


## Hardware

* ESP32
* MPU6050
* 4 Push buttons


## Wiring

![Wiring Diagram](wiring\_diagram.png)


## How to use

1. Upload code to ESP32
2. Power the board
3. Connect via Bluetooth

## Library Note

This project uses a modified version of the BleMouse library.
A small change was made to improve responsiveness.
The modified library is included in the `lib/BleMouse` folder.


