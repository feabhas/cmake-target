# Feabhas CMake Project Notes

# Basic Usage

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

The CMake build makes use of [Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
which are configured to use Ninja. The `build.sh` script checks whether Ninja is installed
and if not it will fall back to use Unix Makfiles instead.

## To build the application

At the project root do:

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

# Using VS Code

VS Code has been configured with tasks to build the code and run a gdb session.

## VS Build Tasks

Use the keyboard shortcut `Ctrl-Shift-B` to run the default build.

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

For the host:
    * **Run Application** to run the built executable 
    
## VS Debug

To debug your code with the interactive (visual) debugger press the `<F5>` key or use the
**Run -> Start Debugging** menu.

The debug sessions with stop at the entry to the `main` function and *may* display 
a red error box saying:

```
Exception has occurred.
```

This is normal: just close the warning popup and use the debug icon commands at the top 
manage the debug system. The icons are (from left to right):
  **continue**, **stop over**, **step into**, **step return**, **restart** and **quit**

When working with QEMU additional debug launch tasks are available from the drop down list
at the top of the debug view. There are:

    * **QEMU debug** for a debug session with graphic WMS window
    * **QEMU nographic debug** for a debug session without the graphic WMS window
    * **QEMU serial debug** for a debug session using the serial port

Note that when using the debugger with a serial port you must use an external
Linux terminal to run Telnet and must have the graphic WMS window displayed.

# Building an exercise solution

To build any of the exercise solutions run the script:
```
$ ./build-one.sh N 
```
where `N` is the exercise number.

In the pre-built VM images the solutions are stored in the home folder but a 
fodler called `solutions`. 

When working with a Docker image you will either be given an archive of the
solutions, an archive of the exercises, a web link to download the archive,
or a link to clone a GIT repo. 

Once you have identified your local copy of the solutions you should 
copy this folder into the workspace and rename it to `solutions`.

A workspace sub-folder called `solutions` is always used in preference to any other 
location.

**NOTE:** building a solution will copy all files in the `src` directory to 
the `src.bak` directory having removed any files already present in `src.bak`.

The `build-one.sh` script supports the `--help` option for more information.

Do not use the `--all` option as this will build each solution in turn and is used
as part of our Continuous Integration testing.


# Creating the template starter projects

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

You can also use `cmake` or `ctest` directly.

# C/C++ Versions

The build system supports compiling against different versions of C and C++ with the 
default set in `MakeLists.txt` as C11 and C++17. The `build.sh` and `build-one.sh` scripts
accept a version option to choose a different language option. To compile against C99 add 
the option `--c99 (or --C99) or for C++20 add --cpp20 (or --c++20 --C++20 --CPP20).

# C++20 Modules

Support for compiling C++ modules is enabled by creating a file `Modules.txt` in the
`src` folder and defining each module filename on a separate line in this file. The build 
ensures modules are compiled in the order defined in the `Modules.txt` file and before the 
main `src` files. Following MSVC and VS Code conventions the modules should be defined 
in `*.ixx` files.

# Implementation Notes

The `build.sh` script and `CMakeLists.txt` files are configured to automate the
building projects as much as possible. The same configuration files can be used
for both C and C++ projects with both host and target environments. The
target environment also needs the Arm compiler settings file `toolchain-STM32F407.cmake`

The `build-one.sh` script works with both scons and CMake (default): 
   1. CMake is used if there is a `CMakeFiles.txt` file present
   2. scons if `SConstruct` is present
   3. otherwise the build is rejected
 
## Host and Target Projects

The host and target projects use separate `build` and `CMakeList.txt` files which 
have a lot in common. This description is based on the more complex target 
configuration on the basis that the host version simply omits the functionality
required for cross compiling to an Arm embedded system.

## Build Shell Script

The `build.sh` script will generate simple `main.c` or `main.cpp` sources files
for the appropriate project type if these are not present.

Neither template has a `src` folder, just the build files: a suitable
starter `main.c` or `main.cpp` file is generated when the build script is first run.

In the case of the target the following support folders:
   * drivers-c     // supporting files for C courses
   * drivers-cpp   // supporting files for C++ courses
   * middleware    // RTOS files for all courses
   * system        // Cortex-M files for all courses

## Creating source files main.c or main.cpp

The first time the `build.sh` script is run it will create any missing
`src` and `include` folders and generate the appropriate `main.c/main.cpp`
file. The VM hostnames are set to the course code so that on a VM the correct 
C or C++ file will be created on the assumption that host names starting
with `c-` or `ac-` will be C and all other hosts will be C++. The 
presence of the `system` folder is used to identify an Arm target project.

The course exercise build instructions could specify that the first time the script
is run the options `-c` or `-cpp` can be added to the command line to generate
the correct file for the course when not using a VM:

```
$ ./build.sh -c        # generate main.c
$ ./build.sh -cpp      # generate main.cpp
```

## Handling user added or renamed source files

**WARNING**: The CMake configuration uses wildcards for the source files and headers 
in `src` and `include`. Once the make build files have been generated any 
additional files added to these directories will not be included in the build. 

The `build.sh` script attempts to check if the contents of the `src` and/or `include` folders
have changed an automatically forces a `reset` build if required.

