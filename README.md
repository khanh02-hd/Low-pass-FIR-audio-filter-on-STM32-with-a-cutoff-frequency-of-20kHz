# Real-Time Low-Pass FIR Audio Filter on STM32 (20kHz Cutoff)

![STM32](https://img.shields.io/badge/STM32-F411CEU6-blue.svg)
![DSP](https://img.shields.io/badge/DSP-FIR%20Filter-green.svg)
![MATLAB](https://img.shields.io/badge/MATLAB-Visualization-orange.svg)

This repository contains a complete Digital Signal Processing (DSP) project that implements a **Real-Time Low-Pass FIR (Finite Impulse Response) Filter** on the STM32F411CEU6 microcontroller. It is designed to filter incoming I2S digital audio from a microphone, applying a precise cutoff frequency of 20kHz, and transmitting the processed data to a PC via USB for real-time visualization in MATLAB.

## 🌟 Key Features

- **Real-Time Audio Processing:** Captures audio data from an I2S microphone using DMA.
- **51-Tap FIR Filter:** Implements a highly optimized 51-tap Low-Pass FIR filter (designed via the Window Method in MATLAB).
- **Zero Audio Dropouts:** Utilizes Double Buffering (Circular DMA) to ensure continuous, gapless audio sampling and processing.
- **High-Speed USB Transmission:** Features USB CDC with data batching (aggregating samples into large buffers) to increase throughput by 10-20x compared to standard byte-by-byte transmission.
- **Dual Operating Modes:**
  - `MODE_MIC`: Processes real acoustic data from the I2S microphone.
  - `MODE_TEST`: Generates an internal simulated waveform for algorithm verification without hardware sensors.
- **MATLAB Integration:** Includes dedicated MATLAB scripts to receive USB data, compute the Fast Fourier Transform (FFT), and visualize both raw and filtered signals in the frequency and time domains.

## 📂 Repository Structure

- `STM32F411CEU6/`: The complete STM32CubeIDE project containing the C source code, HAL drivers, USB device library, and hardware configuration (`.ioc`).
  - `Core/Src/main.c`: The core application logic, including the convolution algorithm and DMA callbacks.
- `MATLAB/`: Scripts for data visualization and filter design.
  - `firfft.m`: Calculates filter coefficients.
  - `mode_mic.m` / `mode_test.m`: Reads USB serial data from the STM32 and plots real-time waveforms and FFT spectrums.

## 🛠️ Hardware Requirements

- **Microcontroller:** STM32F411CEU6 (e.g., Black Pill development board)
- **Audio Sensor:** INMP441 or any compatible I2S Omnidirectional Microphone
- **Programmer:** ST-Link V2

## 🚀 Getting Started

### 1. Flashing the STM32
1. Clone this repository:
   ```bash
   git clone https://github.com/khanh02-hd/Low-pass-FIR-audio-filter-on-STM32-with-a-cutoff-frequency-of-20kHz.git
   ```
2. Open **STM32CubeIDE**.
3. Go to `File` > `Import...` > `General` > `Existing Projects into Workspace`.
4. Select the `STM32F411CEU6/` directory and click `Finish`.
5. Connect your ST-Link and click **Debug/Run** to flash the firmware.

### 2. Running MATLAB Visualizer
1. Connect the STM32 to your PC via the secondary USB port (acting as a USB CDC Virtual COM Port).
2. Note the COM Port number assigned by your OS (e.g., `COM3` on Windows).
3. Open MATLAB, navigate to the `MATLAB/` folder.
4. Modify the COM port string in `mode_mic.m` to match your device.
5. Run the script to see the real-time audio waveforms and frequency spectrums!

## ⚙️ Filter Specifications
- **Type:** Low-Pass FIR
- **Cutoff Frequency:** 20,000 Hz
- **Taps:** 51
- **Window:** Hamming
