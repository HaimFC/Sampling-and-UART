# UART Communication and Sampling Project

This repository contains the design and implementation of a UART-based communication system for Arduino. The project includes a Full-Duplex communication scheme and supports frame-based asynchronous data transmission with features such as parity checking and sampling.

## Files in the Repository

- **`design.pdf`**: Details the system design and flowcharts for the UART transmission and reception states. Key states include `IDLE`, `START`, `DATA`, `PARITY`, and `STOP`, with transitions based on timer and parity conditions.

- **`Lab2.pdf`**: Provides the lab instructions for implementing Full-Duplex UART communication. It describes the frame structure, including start bit, stop bit, parity bit, and data bits. The document also outlines the requirements for `uart_tx()` and `uart_rx()` functions, sampling logic, and handling corrupt transmissions.

- **`arduino.ino`**: Contains the Arduino code for UART transmission and reception. Implements the described logic in `Lab2.pdf` and follows the design principles outlined in `design.pdf`.

## Project Overview

### Frame Structure
The UART frame consists of:
1. **Start Bit**: A single `0` bit.
2. **Data Bits**: 8 bits of data, sent least significant bit (LSB) first.
3. **Parity Bit**: An odd parity bit ensures the total number of `1` bits is odd.
4. **Stop Bit**: A single `1` bit.

### Key Features
- **Full-Duplex Communication**: Both transmission and reception occur simultaneously on separate channels.
- **Asynchronous Operation**: No clock signal is shared; timing is managed via predefined bit times (`BIT_TIME`).
- **Sampling**: Each bit is sampled `SAMP_NUM=3` times to ensure accuracy.
- **Error Handling**: Frames with invalid parity, start, or stop bits are dropped.
- **Modularity**: Frame size and sampling parameters can be adjusted easily.

### Functions
- **`uart_tx()`**: 
  - Sends a frame according to the defined structure.
  - Includes random delays between transmissions.
  - Avoids busy-waiting techniques.
  
- **`uart_rx()`**:
  - Reads incoming frames and validates them.
  - Detects the start of transmission by identifying the start bit (`0`).
  - Handles sampling and error detection during the frame reception.

## Tools and References
- **Simulator**: [Tinkercad](https://www.tinkercad.com) - Use this for testing your circuit and code.
- **Arduino Reference**: [Arduino Documentation](https://www.arduino.cc/reference/en/) - Official Arduino language reference.
- **Odd Parity Calculator**: [ToolMeNow](https://www.toolmenow.com/33/Odd-Parity-Calculator) - Calculate parity bits for testing.

## Getting Started

1. **Hardware Setup**: Connect two Arduino boards with separate TX and RX channels.
2. **Upload Code**: Use the `arduino.ino` sketch to upload the UART logic to both devices.
3. **Test Communication**: Use serial monitors to verify frame transmission and reception, including error handling.

## Design Details

### Transmission States
1. **IDLE**: Waits for data to transmit.
2. **START**: Sends the start bit (`0`).
3. **DATA**: Transmits 8 bits of data, LSB first.
4. **PARITY**: Sends the parity bit based on odd parity.
5. **STOP**: Sends the stop bit (`1`).

### Reception States
1. **IDLE**: Waits for a start bit (`0`).
2. **DATA**: Samples and stores incoming bits.
3. **PARITY**: Validates the parity of the received data.
4. **STOP**: Checks the stop bit and completes the frame.

### Error Handling
Frames with invalid start bits, parity, or stop bits are dropped. Sampling errors (bad bits) result in the frame being ignored.

## Contribution

Contributions are welcome. Please follow the repository structure and document your changes thoroughly.
