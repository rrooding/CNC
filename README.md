# My CNC build

This repository contains all resources for my CNC build, including the RootCNC controller, CNC pendant controller, and CNC machine.

## RootCNC Controller

The RootCNC controller is a custom controller based on the ESP32 microcontroller. It runs the FluidNC firmware and provides a web interface for CNC control.

## CNC Pendant Controller

The CNC pendant controller is a handheld controller that interfaces with the RootCNC controller. It translates rotary encoder signals and switch inputs into FluidNC-compatible G-code commands, sent over UART.
