# Feabhas CMake Project Notes

**Contents**
- [Feabhas CMake Project Notes](#feabhas-cmake-project-notes)
- [Toolchain](#toolchain)
- [Build the application](#build-the-application)
  - [VS Code build tasks](#vs-code-build-tasks)
  - [Command line build script](#command-line-build-script)
- [Working with the QEMU emulator](#working-with-the-qemu-emulator)
  - [Ubuntu VM or datakey](#ubuntu-vm-or-datakey)
  - [Docker  container](#docker--container)
- [Debugging](#debugging)
  - [VS Code debug](#vs-code-debug)
- [Building an exercise solution](#building-an-exercise-solution)
  - [Docker Image](#docker-image)
- [Creating template starter projects](#creating-template-starter-projects)
- [Static analysis using clang-tidy](#static-analysis-using-clang-tidy)
- [Testing support](#testing-support)
- [C/C++ Versions](#cc-versions)
  - [C++20 Modules](#c20-modules)
- [Disclaimer](#disclaimer)

# Toolchain

The Feabhas project build process uses [CMake](https://cmake.org/) as the underlying
build system. CMake is itself a build system generator and we have configured
it to generate the build files used by either [Ninja](https://ninja-build.org/) or
[GNU Make](https://www.gnu.org/software/make/): `ninja` is used in preference to `
make` if it is installed.

Using CMake is a two step process: generate build files and then build. To simplify 
this and to allow you to add additional source and header files we have 
created a front end script to automate the build.

You can add additional C/C++ source and header files to the `src` directory. If 
you prefer you can place you header files in the `include` directory.

The build process checks if the contents of the `src` and/or `include` folders
have changed and automatically regenerates the build configuration.

# Build the application

## VS Code build tasks

VS Code has been configured with tasks to build the code and run a gdb session.

From within VS Code you can use the keyboard shortcut **Ctrl-Shift-B** 
to run one of the build tasks:
    * **Build** standard build
    * **Clean** to remove object and executable files
    * **Reset** to regenerate the CMake build files

## Command line build script

In the project root run:

```
$ ./build.sh
```

This will generate the file `build/debug/Application.elf` on an Arm target 
project or `build/debug/Application` on the host. Additional size and hex files
used by some flash memory software tools are generated for the Arm target.

You can add a `-v` option see the underlying build commands:

```
$ ./build.sh -v
```

The `build.sh` script supports the `--help` option for more information.

You have additional build options:

   * **./build.sh clean**      *# delete working files for a clean rebuild*
   * **./build.sh reset**      *# regenerate the complete build configuration*

# Working with the QEMU emulator

Once you have successfully built you application you can run it using the 
supplied QEMU emulator. 

Initially you should start the Python GUI application provided as `qemu_wms.py`. 

## Ubuntu VM or datakey

On an Ubuntu VM or datakey supplied by Feabhas there will be an icon in the left 
hand *favourtites* panel that will launch the GUI application. 

Alternatively or when using your own Linux or macOS host you can run the command:

```
$ python3 qemu_wms.py
```

## Docker  container

**Important:** When working with Docker behind the GitHub `docker-target` 
project you must run the Python GUI application on you host 
(the docker container does not support graphic applications).

On your host start the GUI application as follows:
   * **Windows:** use File Explorer to select the project workspace and double click on the file `qemu_wms.py`
   * **Linux**	start a shell command in the project workspace and enter the command `python3 qemu_wms.py`
   * **macOS**	start a **Termianl** application in the project workspace and enter the command `python3 qemu_wms.py`

After starting the Python GUI go back to VS Code to start the QEMU emulator:
   * Press **Ctrl-Shift-P** (or **Shift-CMD-P** on macOS hosts) to launch the *Command Palette*
     (you can also use the **View -> Command Palette** menu option)
   * type `test task` and select **Tasks: Run Test Task** from the list
   * select **Run QEMU** from the list of display test tasks

The next time you use **Ctrl-Shift-P** the **asks: Run Test Task** will be at the top of the list. 

Running your application will trigger a rebuild if the application is out of date.

Alternatively from the command line enter:

```
$ ./run-qemu.sh
```

The run task starts QEMU which will pause waiting for a connection on localhost:8888.

   * In the Python GUI click on the **Connect** button.
 
This will connect to port 8888 on the emulator and QEMU will start executing your application. 

•	In the search panel enter test (or test task):
 
Other test tasks include:
    * **Run QEMU** to run the emulator using `./run-qemu.sh` 
    * **Run QEMU nographic** to run the emulator using `./run-qemu.sh --nographic` 
    * **Run QEMU serial** to run the emulator using `./run-qemu.sh serial` 
    * **Run QEMU serial nographic** to run the emulator using `./run-qemu.sh --nographic serial` 

For exercises that work with the USART3 serial port you will need to run start the QEMU emulator with USART3 connected to port 7777.

From within VS Code press:
   * **Ctrl-Shift-P** and select task **QEMU serial** 

From the command line you can use:

```
$ ./run_qemu.sh serial
```

When using the Python GUI click on the **Connect+Serial** button to connect to both the diagnostic (8888) and USART3 (7777) ports. The bottom area of the GUI will display an interactive text area you can use to send and receive using USART3.

# Debugging

## VS Code debug

To debug your code with the interactive (visual) debugger press the `<F5>` key or use the
**Run -> Start Debugging** menu.

The debug sessions with stop at the entry to the `main` function and *may* display 
a red error box saying:

```
Exception has occurred.
```

This is normal: just close the warning popup and use the debug icon commands at the top 
manage the debug system. The icons are (from left to right):
   * **continue** **stop over** **step into**  **step return** **restart** **quit**

When working with QEMU additional debug launch tasks are available from the drop down list
at the top of the debug view:

   * **QEMU debug** for a debug session with graphic WMS window
   * **QEMU nographic debug** for a debug session without the graphic WMS window
   * **QEMU serial debug** for a debug session using the serial port

Note that when using the debugger with a serial port you must use an external
Linux terminal to run Telnet and must have the graphic WMS window displayed.

# Building an exercise solution

To build a solution run the command:

```
$ python3 copy_solution.py
```

Select the required solution from the list you are shown. 

You may supply the solution number (optionally omitting a leading zero)
on the command line to avoid the interactive prompt.

On loading a solution the script will:

   * save and commit your current files using git
   * replace all of your source files with those from the the solution
   * rebuild the solution

**Note:** If the script cannot save your source files using git then they are
copied to a `src.bak` folder. Only that last set of source files are saved in
the backup folder.

Alternatively you can build any of the exercise solutions using the 
`build-one.sh` bash script:

```
$ ./build-one.sh N 
```

Where *N* is the exercise number. The exercises must be stored in the 
workspace folder in one of the following locations:
   * A cloned github repo name ending `_exercises`
   * An `exercises/solutions`sub-folder in the workspace
   * A `solutions`sub-folder in the workspace

**NOTE:** this script will copy all files in the `src`  and
`include` directories to a `src.bak` directory in the workspace; 
any files already present in `src.bak` will be deleted.

## Docker Image

Inside your workspace subfolder called `scripts` there is 
a `configure.py` script that can be used to copy the course exercises 
into your workspace. 

You can run this script at any time from your host environment
or, once you've opened the project workspace, from a terminal
window in VS Code using the command:

```
$ python3 configure.py
```

The script will supply a list of suitable courses for you to choose from and
these exercises will be download from the appropriate Feabhas GitHub repo.

You will now have a sub-folder with a name of the form `<COURSE>_exercises`.
where `<COURSE>` is the unique code for your course (such as cpp11-501).

If you know you course code you can supply this as a command line parameter
to the script.

# Creating template starter projects

Some training courses supply one or more template starter projects containing
a working application that will be refactored during the exercises.

These templates are used to generate fully configured projects in 
named subfolders. To generate the sub projects run the command:

```
$ ./build-template.sh
```

This will generate fully configured projects each starter template
as a sub project in teh root workspace. Each sub project
contains a fully configured CMake based build system including a 
copy of the solutions folder. The original toolchain build files in the
project are moved to a `project` sub-folder as they are no longer required.

For each exercise you can now open the appropriate sub-project
folder and work within that folder to build and run your application.

# Static analysis using clang-tidy

The CMake build scripts create a `clang-tidy` target in the generated build files if
`clang-tidy` is in the command search path (`$PATH` under Linux).

To check all of the build files run the command:
```
$ ./build.sh clang-tidy
```

To run `clang-tidy` as part of the compilation process edit the `CMakeLists.txt` file
and uncomment the line starting with `set(CMAKE_CXX_CLANG_TIDY`.

# Testing support

Create a sub-directory called `tests` with it's own `CMakeList.txt` and define
yoru test suite (you don't need to include `enable_testing()` as this is done
in the project root config).

Invoke the tests by adding the `test` option to the build command:

```
./build.sh test
```

Tests are only run on a successful build of the application and all tests.

You can also use `cmake` or `ctest` commands directly.

# C/C++ Versions

The build system supports compiling against different versions of C and C++ with the 
default set in `MakeLists.txt` as C11 and C++17. The `build.sh` and `build-one.sh` scripts
accept a version option to choose a different language option. To compile against C99 add 
the option `--c99 (or --C99) or for C++20 add --cpp20 (or --c++20 --C++20 --CPP20).

## C++20 Modules

Support for compiling C++ modules is enabled by creating a file `Modules.txt` in the
`src` folder and defining each module filename on a separate line in this file. The build 
ensures modules are compiled in the order defined in the `Modules.txt` file and before the 
main `src` files. Following MSVC and VS Code conventions the modules should be defined 
in `*.ixx` files.

# Disclaimer

Feabhas is furnishing these items *"as is"*. Feabhas does not provide any
warranty of them whatsoever, whether express, implied, or statutory,
including, but not limited to, any warranty of merchantability or fitness
for a particular purpose or any warranty that the contents their will
be error-free.

In no respect shall Feabhas incur any liability for any damages, including,
but limited to, direct, indirect, special, or consequential damages arising
out of, resulting from, or any way connected to the use of the item, whether
or not based upon warranty, contract, tort, or otherwise; whether or not
injury was sustained by persons or property or otherwise; and whether or not
loss was sustained from, or arose out of, the results of, the item, or any
services that may be provided by Feabhas.

The items are intended for use as an educational aid.Typically code solutions 
will show best practice of language features that have been introduced during 
the associated training, but do not represent production quality code. 
Comments and structured documentation are not included because the code 
itself is intended to be studied as part of the learning process.

