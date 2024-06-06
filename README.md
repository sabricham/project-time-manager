## Time Manager
Time Manager device with timer, stopwatch, pomodoro session features and more!
Here you can find all the documentation and the code used in the project starting from a first prototype version to a fully functional product

## Table of contents
- [Introduction](#introduction)
- [Components](#components)
- [Diagrams](#diagrams)

## Introduction 
The device firmware is developed using ESP-IDF framework within VS Code environment for an embedded development experience instead of faster & easier alternatives such as Arduino IDE or PlatformIO plugin for VS Code.
Its case is accurately designed within Blender3D to ensure that all the components fit inside a very small cylinder-shaped area.
3D printing is used to create a functional proof-of-concept and prototype, other methods to build a stronger and more precise case will be used later.

## Components
- [ESP32-C3 Super Mini] very compact board integrating an ESP32-C3 chip, a 32-bit RISC-V single-core processor with Bluetooth & WIFI capabilities<br /><br />
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/33c9975b-c196-4d75-909e-e24988ed2d0a" width="200" height="200">
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/9cd5d7f8-e787-46f9-bbe2-b9a7701bd563" width="400" height="200">
<br /><br />
- [Rotary Encoder MC11] digital incremental encoder with no angular limitation and an integrated pressure switch<br /><br />
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/79502f33-718c-4055-8239-babceb66607e" width="200" height="200">
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/7225a2c7-1111-49ce-b1c6-4f3c0d4a3497" width="300" height="200">
<br /><br />
- [SH1106 OLED 128x64 Display] 1.3" oled monocromatic display with I2C communication interface<br /><br />
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/cf0251d3-efd8-4f6c-b578-5f997ed63d08" width="200" height="200">
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/2e6da7e5-f13a-49d8-bd86-58c78cf30cae" width="300" height="200">
<br /><br />

## Diagrams
- System schematic<br /><br />
<img src="https://github.com/sabricham/project-time-manager/assets/149872304/cb2df2a5-890c-4913-b02d-a43e02fb8b8b" width="450" height="300">
<br /><br />
- Circuit diagram
