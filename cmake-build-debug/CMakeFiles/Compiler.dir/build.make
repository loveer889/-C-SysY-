# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.13

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2018.3.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2018.3.4\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\CPP\AllProjects\Compiler

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\CPP\AllProjects\Compiler\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Compiler.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Compiler.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Compiler.dir/flags.make

CMakeFiles/Compiler.dir/main.cpp.obj: CMakeFiles/Compiler.dir/flags.make
CMakeFiles/Compiler.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\CPP\AllProjects\Compiler\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Compiler.dir/main.cpp.obj"
	C:\PROGRA~1\MinGW\bin\G__~1.EXE  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\Compiler.dir\main.cpp.obj -c E:\CPP\AllProjects\Compiler\main.cpp

CMakeFiles/Compiler.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Compiler.dir/main.cpp.i"
	C:\PROGRA~1\MinGW\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\CPP\AllProjects\Compiler\main.cpp > CMakeFiles\Compiler.dir\main.cpp.i

CMakeFiles/Compiler.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Compiler.dir/main.cpp.s"
	C:\PROGRA~1\MinGW\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\CPP\AllProjects\Compiler\main.cpp -o CMakeFiles\Compiler.dir\main.cpp.s

# Object files for target Compiler
Compiler_OBJECTS = \
"CMakeFiles/Compiler.dir/main.cpp.obj"

# External object files for target Compiler
Compiler_EXTERNAL_OBJECTS =

Compiler.exe: CMakeFiles/Compiler.dir/main.cpp.obj
Compiler.exe: CMakeFiles/Compiler.dir/build.make
Compiler.exe: CMakeFiles/Compiler.dir/linklibs.rsp
Compiler.exe: CMakeFiles/Compiler.dir/objects1.rsp
Compiler.exe: CMakeFiles/Compiler.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\CPP\AllProjects\Compiler\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Compiler.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\Compiler.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Compiler.dir/build: Compiler.exe

.PHONY : CMakeFiles/Compiler.dir/build

CMakeFiles/Compiler.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\Compiler.dir\cmake_clean.cmake
.PHONY : CMakeFiles/Compiler.dir/clean

CMakeFiles/Compiler.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\CPP\AllProjects\Compiler E:\CPP\AllProjects\Compiler E:\CPP\AllProjects\Compiler\cmake-build-debug E:\CPP\AllProjects\Compiler\cmake-build-debug E:\CPP\AllProjects\Compiler\cmake-build-debug\CMakeFiles\Compiler.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Compiler.dir/depend

