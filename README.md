ADC PCIe Data Visualizer

This project implements a high-performance digital backend on a Xilinx ZU19EG device for capturing and streaming data over PCIe to a Python GUI running on the host PC. This project is a subsystem of a larger system and is designed to operate on an independent SoC core (Verified in final implementation). Project currently adapted to specific hardware. Not for general use.

Since the project is tightly coupled to Vitis, the following steps are needed to reconstruct the project.

1) Create a platform using the following file: ZU19EG_PCIe_Streaming/Vivado2023.1/project_1/design_1_wrapper.xsa

2) Once the project is built, import the source files from ZU19EG_PCIe_Streaming/Vitis2023.1/ZU19_XDMA_Application/src

3) Build the project using the Vitis toolchain. 
