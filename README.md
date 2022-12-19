# Feabhas CMake Project Notes

# Basic Usage

The Feahbas project build process uses [CMake](https://cmake.org/) as the underlying
build system. CMake is itself a build system generator and we have configured
it to generate the build files used by [GNU Make](https://www.gnu.org/software/make/).

Using CMake is a two step process: generate build files and then build. To simplify 
this and to allow you to add additional source and header files we have 
created a front end script to automate the build.

You can add additional C/C++ source and header files to the `src` directory. If 
you prefer you can place your header files in the `include` directory.

## To build the application

From within VS Code you can use the keyboard shortcut `Ctrl-Shift-B` 
to run one of the build tasks:
    * **Build** standard build
    * **Clean** to remove object and executable files
    * **Reset** to regenerate the CMake build files

To run the application use `Ctrl-Shift-P` shortcut key, enter test in 
the search field and then select `Tasks: Run Test Task` from the list of tasks shown. 
The next time you use `Ctrl-Shift-P` the `Tasks: Run Test Task` will be at the top of the list. 

Run tasks are project specific. For the target
    * **Run QEMU** to run the emulator using `./run-qemu.sh` 
    * **Run QEMU nographic** to run the emulator using `./run-qemu.sh --nographic` 
    * **Run QEMU serial** to run the emulator using `./run-qemu.sh serial` 
    * **Run QEMU serial nographic** to run the emulator using `./run-qemu.sh --nographic serial` 

At the project root do:

```
$ ./build.sh
```

This will generate the file `build/debug/Application.elf`, additional size and 
hex files used by some flash memory software tools are also generated.

You can add a `-v` option to see the underlying build commands:

```
$ ./build.sh -v
```

## To clean the build

To delete all object files and recompile the complete project use
the `clean` option:

```
$ ./build.sh clean
```

To clean the entire build directory and regenerate a new build configuration use
the `reset` option:

```
$ ./build.sh reset
```

# Running the QEMU (WMS) simulator

The QEMU emulator can mimic the Washing Machine Simulator (WMS) on the Feabhas 
hardware target board.

In a terminal invoke the following script:

```
$ ./run_qemu.sh
```

A graphic image of the Cortex-M and WMS boards with be displayed and on the 
shell window there will be some debug output, which can be ignore for the moment. 

As the program runs changes to the state of the hardware is displayed on the
graphic window and diagnostic messages written to the console:

```
[led:A on]
[seven-segment 1]
[led:C on]
[seven-segment 5]
[led:A off]
[seven-segment 4]
```

Once `main` exits the QEMU emulator will stop. Closing the graphic window will 
stop the QEMU simulation.

Use the WMS graphic window to monitor and interact with the program. 

The Coretex-M board is to the left and has a reset button (middle left) which 
can be clicked with the mouse to reset the hardware, restarting the program. 
Four LED lights are shown on the bottom right of the board and will display 
coloured boxes when the appropriate GPIO-D pins are set. 

The WMS board is on the right and updates as the GPIO-D pins are set
and cleared:
   * seven segment display updates on changes to pins 8-11
   * the motor animates to shown on/off cw/acw rotation on pins 12-13
   * the latching mode (pin 14) for the PS keys has no direct visual feedback

The WMS boards has mouse click input for GPIO-D input pins:
   * Accept and Cancel keys (pins 5 & 4)
   * keys PS1, PS2 & PS3 (pins 1,2,3) switch on the led lights above the key
   * when latched the PS* leds remain illuminated when the key is released
   * door open key (pin 0) toggles open/closed when pressed
   * motor feedback sensor (pin 6) is raised once every .1 secs when the motor is on
   * click on the centre of the motor spinner to pulse the motor sensor (pin 6)

You can suppress display of the WMS graphic window if required: this can be 
useful for exercises that do not use the Washing Machine Simulator functionality
or when using the debugger.

Use the `--nographic` option of the QEMU emulator to start without the
graphic window:

```
./run-qemu.sh --nographic
```


## Simulating USART3 in QEMU
The IO from USART3 of the STM32F407 microcontroller has been mapped on to 
QEMU Serial Port 0 which can be accessed via a telnet session.

Use QEMU to test USART3 serial communications by running the command:

```
$ ./run_qemu.sh serial
```

The QEMU simulation will now wait until a connection is made on port `7777`.

In a second terminal start a telnet session on port `7777`:
```
$ telnet localhost 7777
Trying ::1...
Connected to localhost.
Escape character is '^]'.
```

Telnet will start in character mode (defined in the configuration file `~/.telnetrc`).
Each character you type is immediately transmitted to the USART receive buffer. No
character translation takes place on output so you need to send the string "\r\n" 
to start a newline.

When working with some of the USART exercises you may find it convenient 
to suppress the display graphic window using the `--nographic` option:

```
$ ./run_qemu.sh --nographic serial
```

# Debugging with QEMU emulation

## Running a simulation

To debug a program just using the GPIO port requires two terminal sessions.

1. In one terminal invoke the following script:
```
$ ./run_qemu.sh gdb
```
A monitor window will appear and there will be some debug output. 
The QEMU simulation will halt at the first instruction waiting for a GDB connection.

2. In another terminal, run GDB with
```
$ ./gdb-qemu.sh
```

Diagnostic output will appear in the `gdb` window ending with prompt to continue:
```
...
..
-- Type <RET> for more, q to quit, c to continue without paging--
```

Press <Enter> at this point to see the code of the `main` function and the `(gdb)` 
prompt for debug commands.

3. Type 
   * `c` (continue) to run
   * `n` for next (step-over)
   * `s` for step (step-in)

If GPIO-D pins 8..11 are written to, output will appear in the QEMU windows, such as:
```
[led:A on]
[seven-segment 1]
[led:C on]
[seven-segment 5]
[led:A off]
[seven-segment 4]
```

## Exiting a session

To exit:
1. Use Ctrl-C in the GDB window to interrupt an executing process to return to
the `gdb` prompt.

2. Enter the kill (`k`) command to stop the remote qemu process.

3. Finally `q` will quit gdb

## Debugging USART3 in QEMU

1. Start USART3 with both `gdb` and serial communications using:

```
$ ./run_qemu.sh gdb serial
```

The QEMU emulator is now is listening on `localhost:7777` for a telnet connection.

2. In a second terminal window start the gdb session with the command:
```
$ ./gdb_qemu.sh
```

3. In a third terminal invoke a telnet session on port `7777`:
```
$ telnet localhost 7777
Trying ::1...
Connected to localhost.
Escape character is '^]'.

```

Telnet will start in character mode (defined in the configuration file `~/.telnetrc`).
Each character you type is immediately transmitted to the USART receive buffer. No
character translation takes place on output so you need to send the string "\r\n" 
to start a newline.

4. You will need to use the gdb terminal to step through or run the code, and switch back
to the telnet session to enter character input when required.

# VS Code Debug

To debug your code with the interactive (visual) debugger press the `<F5>` key or use the
**Run -> Start Debugging** menu.

The debug sessions with stop at the entry to the `main` function and display 
a red error box saying:

```
Exception has occurred.
```

This is normal: just close the warning popup and use the debug icon commands at the top 
manage the debug system. The icons are (from left to right):
  **continue**, **stop over**, **step into**, **step return**, **restart** and **quit**
  
A number of debug launch tasks are shown in a drop down list at the top of the debug view.
Preselect one of the laucnh options before pressing `<F5>` to debug with:

    * **QEMU debug** to debug using `./run-qemu.sh gdb` 
    * **QEMU nographic debug** to debug using `./run-qemu.sh --nographic gdb` 
    * **QEMU serial debug** to debug using `./run-qemu.sh gdb serial` 

# Building an exercise solution

To build any of the exercise solutions run the script:
```
$ ./build-one.sh N 
```
where `N` is the exercise number.

**NOTE:** this will copy all files in the `src` directory to the `src.bak` directory having
removed any files already present in `src.bak`.

Do not use the `build-all.sh` script as this will build each solution in turn and is used
as part of our Continuous Integration testing.

