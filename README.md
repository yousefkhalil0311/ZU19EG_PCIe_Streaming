ADC PCIe Data Visualizer

This project implements a high-performance digital backend on a Xilinx ZU19EG device for capturing and streaming data over PCIe to a Python GUI running on the host PC. This project is a subsystem of a larger system and is designed to operate on an independent SoC core (Verified in final implementation).

Features 
Real-time streaming of 8 ADC channels (4 per AD9209)

Hardware control from the Python GUI (resets, clocks, parameter updates)

PCIe-based interface for data and control using Xilinx XDMA

Python GUI with live plotting using matplotlib and PyQt5

FPGA implemented on ZU19EG (Zynq UltraScale+) with hardened PCIe

Optional BRAM logging and memory-mapped debug access


FPGA
Custom Xilinx ZU19EG board with:

PCIe interface enabled (XDMA)

Two AD9209 chips connected via JESD204C

Custom HDL for ADC interfacing and PCIe data streaming

How It Works
The FPGA receives data from two AD9209 ADCs (8 total channels).

Data is streamed over PCIe using the Xilinx XDMA IP to the host PC.

The Python GUI maps PCIe memory and reads incoming ADC data.

The GUI also sends control signals to the FPGA to manage settings like clock enables, resets, and trigger modes.
