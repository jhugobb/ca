# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/yellow/Desktop/MIRI/2year/ca/Crowd

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yellow/Desktop/MIRI/2year/ca/Crowd/build

# Include any dependencies generated for this target.
include CMakeFiles/BaseCode.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/BaseCode.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/BaseCode.dir/flags.make

CMakeFiles/BaseCode.dir/main.cpp.o: CMakeFiles/BaseCode.dir/flags.make
CMakeFiles/BaseCode.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yellow/Desktop/MIRI/2year/ca/Crowd/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/BaseCode.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/BaseCode.dir/main.cpp.o -c /home/yellow/Desktop/MIRI/2year/ca/Crowd/main.cpp

CMakeFiles/BaseCode.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/BaseCode.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yellow/Desktop/MIRI/2year/ca/Crowd/main.cpp > CMakeFiles/BaseCode.dir/main.cpp.i

CMakeFiles/BaseCode.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/BaseCode.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yellow/Desktop/MIRI/2year/ca/Crowd/main.cpp -o CMakeFiles/BaseCode.dir/main.cpp.s

CMakeFiles/BaseCode.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/BaseCode.dir/main.cpp.o.requires

CMakeFiles/BaseCode.dir/main.cpp.o.provides: CMakeFiles/BaseCode.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/BaseCode.dir/build.make CMakeFiles/BaseCode.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/BaseCode.dir/main.cpp.o.provides

CMakeFiles/BaseCode.dir/main.cpp.o.provides.build: CMakeFiles/BaseCode.dir/main.cpp.o


# Object files for target BaseCode
BaseCode_OBJECTS = \
"CMakeFiles/BaseCode.dir/main.cpp.o"

# External object files for target BaseCode
BaseCode_EXTERNAL_OBJECTS =

BaseCode: CMakeFiles/BaseCode.dir/main.cpp.o
BaseCode: CMakeFiles/BaseCode.dir/build.make
BaseCode: /usr/lib/x86_64-linux-gnu/libGL.so
BaseCode: /usr/lib/x86_64-linux-gnu/libGLU.so
BaseCode: /usr/lib/x86_64-linux-gnu/libglut.so
BaseCode: /usr/lib/x86_64-linux-gnu/libXi.so
BaseCode: /usr/lib/x86_64-linux-gnu/libGLEW.so
BaseCode: /usr/lib/x86_64-linux-gnu/libfreetype.so
BaseCode: /usr/lib/libcal3d.so
BaseCode: /usr/lib/libcal3d.a
BaseCode: CMakeFiles/BaseCode.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/yellow/Desktop/MIRI/2year/ca/Crowd/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable BaseCode"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/BaseCode.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/BaseCode.dir/build: BaseCode

.PHONY : CMakeFiles/BaseCode.dir/build

CMakeFiles/BaseCode.dir/requires: CMakeFiles/BaseCode.dir/main.cpp.o.requires

.PHONY : CMakeFiles/BaseCode.dir/requires

CMakeFiles/BaseCode.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/BaseCode.dir/cmake_clean.cmake
.PHONY : CMakeFiles/BaseCode.dir/clean

CMakeFiles/BaseCode.dir/depend:
	cd /home/yellow/Desktop/MIRI/2year/ca/Crowd/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yellow/Desktop/MIRI/2year/ca/Crowd /home/yellow/Desktop/MIRI/2year/ca/Crowd /home/yellow/Desktop/MIRI/2year/ca/Crowd/build /home/yellow/Desktop/MIRI/2year/ca/Crowd/build /home/yellow/Desktop/MIRI/2year/ca/Crowd/build/CMakeFiles/BaseCode.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/BaseCode.dir/depend
