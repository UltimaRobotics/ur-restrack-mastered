# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build

# Include any dependencies generated for this target.
include thirdparty/ur-threadder-api/CMakeFiles/example.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include thirdparty/ur-threadder-api/CMakeFiles/example.dir/compiler_depend.make

# Include the progress variables for this target.
include thirdparty/ur-threadder-api/CMakeFiles/example.dir/progress.make

# Include the compile flags for this target's objects.
include thirdparty/ur-threadder-api/CMakeFiles/example.dir/flags.make

thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o: thirdparty/ur-threadder-api/CMakeFiles/example.dir/flags.make
thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o: ../thirdparty/ur-threadder-api/example/example.c
thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o: thirdparty/ur-threadder-api/CMakeFiles/example.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o"
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o -MF CMakeFiles/example.dir/example/example.c.o.d -o CMakeFiles/example.dir/example/example.c.o -c /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/thirdparty/ur-threadder-api/example/example.c

thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/example.dir/example/example.c.i"
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/thirdparty/ur-threadder-api/example/example.c > CMakeFiles/example.dir/example/example.c.i

thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/example.dir/example/example.c.s"
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/thirdparty/ur-threadder-api/example/example.c -o CMakeFiles/example.dir/example/example.c.s

# Object files for target example
example_OBJECTS = \
"CMakeFiles/example.dir/example/example.c.o"

# External object files for target example
example_EXTERNAL_OBJECTS =

thirdparty/ur-threadder-api/example: thirdparty/ur-threadder-api/CMakeFiles/example.dir/example/example.c.o
thirdparty/ur-threadder-api/example: thirdparty/ur-threadder-api/CMakeFiles/example.dir/build.make
thirdparty/ur-threadder-api/example: thirdparty/ur-threadder-api/libthreadmanager.a
thirdparty/ur-threadder-api/example: thirdparty/ur-threadder-api/CMakeFiles/example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable example"
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
thirdparty/ur-threadder-api/CMakeFiles/example.dir/build: thirdparty/ur-threadder-api/example
.PHONY : thirdparty/ur-threadder-api/CMakeFiles/example.dir/build

thirdparty/ur-threadder-api/CMakeFiles/example.dir/clean:
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api && $(CMAKE_COMMAND) -P CMakeFiles/example.dir/cmake_clean.cmake
.PHONY : thirdparty/ur-threadder-api/CMakeFiles/example.dir/clean

thirdparty/ur-threadder-api/CMakeFiles/example.dir/depend:
	cd /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/thirdparty/ur-threadder-api /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api /home/fyou/Documents/ported/openwrt/package/ur-stack/ur-restrack-mastered/pkg_src/build/thirdparty/ur-threadder-api/CMakeFiles/example.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : thirdparty/ur-threadder-api/CMakeFiles/example.dir/depend

