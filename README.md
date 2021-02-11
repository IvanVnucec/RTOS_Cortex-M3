[![Build Status](https://travis-ci.com/IvanVnucec/RTOS_Cortex-M3.svg)](https://travis-ci.com/IvanVnucec/RTOS_Cortex-M3)
[![codecov](https://codecov.io/gh/IvanVnucec/RTOS_Cortex-M3/branch/development/graph/badge.svg?token=B8ATM6FR67)](https://codecov.io/gh/IvanVnucec/RTOS_Cortex-M3)
# Cortex-M3 RTOS
- preemptive, 
- priority-based, 
- statically linked. 

## Description
Cortex-M3 RTOS project with preemtive and priority-based scheduler, delay functions, mutexes and many more functionalities.  
RTOS project files are located under `src/rtos` folder.
  
Implemented:
- preempting and priority-based scheduler, 
- delay functions, 
- mutexes.
  
To do:
- semaphores,
- queues,
- messages, 
- etc.
  
## Building from source
The RTOS is developed on a so-called "Bluepill" board with an STM32F103C8T6 microcontroller. Also, we are using Windows OS.   
If you have some other microcontroller, you will need to change a few things (I will explain what you need to change if you have a different setup). 
  
The project is using:
- VS Code as an IDE, 
- GNU Make as a builder,
- arm-none-eabi toolchain for the GCC and the GDB, 
- ST-Link as debugging probe, 
- and OpenOCD as a program flasher.   
  
First and foremost, we consider that the user already installed the following programs:  
1. VS Code
2. arm-none-eabi toolchain
3. OpenOCD
4. ST-Link drivers
5. GNU Make  
  
Next, follow the steps below:
1. Open VS Code and go to `File -> Open Folder` and select root folder (where you have `README.md` file)
2. Install the following VS Code extensions:
 - ms-vscode.cpptools and
 - marus25.cortex-debug
3. Restart VS Code
4. Open `.vscode/c_cpp_properties.json` file
5. Under `"compilerPath"` set path to point to `arm-none-eabi-gcc.exe` executable.
6. Open `.vscode/launch.json` file
7. Under `"armToolchainPath"` set paths to point to `bin` folder of the amr-none-eabi toolchain.
8. Open `.vscode/settings.json` file
9. Under `"cortex-debug.armToolchainPath"` provide a path to the bin directory of gcc-arm-none-eabi toolchain.
10. Under `"cortex-debug.openocdPath"` provide path to `openocd.exe` executable.
11. Open your favorite terminal and run GNU Make by executing `make` command. The project should build and the .hex, .bin, and .elf files should be located in the `build` directory.
12. Connect Bluepill board with ST-Link and connect ST-Link with the PC.
13. Go into the Run taskbar in VS Code (or press `Ctrl + Shift + D`) and press the green play button to run debug. The program should stop at the `main` function and you can now debug it.

## Contributing
Please look into Issues. Also if you need futher info or guidance, please do not hesitate to contact me at `vnucec.ivan@gmail.com`.

## License
MIT License
