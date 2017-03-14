#!/usr/bin/scons

import os
import subprocess

AddOption(
        '--debug-build',
        action='store_true',
        help='Debug build. Optimization disabled and with debug symbols.',
        default=False)

AddOption(
        '--lldb',
        action='store_true',
        help='For debug builds, this sets the optimization for the target debugger to LLDB',
        default=False)

AddOption(
        '--debug-profiler',
        action='store_true',
        help='Debug build configuration for running the code profiler tools',
        default=False)

AddOption(
        '--debug-sanitize',
        action='store_true',
        help='Debug build with heap sanitizer activated. This injects a lot of overhead.',
        default=False)

global_env = Environment(ENV = os.environ, tools = ['default'])

# Make sure we build with the current terminal
global_env['ENV']['TERM'] = os.environ['TERM']

# Use the Compiler set in the environment
global_env["CC"] = os.getenv("CC") or global_env["CC"]
global_env["CXX"] = os.getenv("CXX") or global_env["CXX"]
global_env["LINK"] = os.getenv("LINK") or global_env["LINK"]
global_env["ENV"].update(x for x in os.environ.items() if x[0].startswith("CCC_"))
global_env["CXXFLAGS"] = "-std=c++11 -pthread"

# --------------------------------------------------
# Global options
# --------------------------------------------------
global_options = Split("""
-Weverything
-Wno-c++98-compat
""")

# --------------------------------------------------
# Build Options
# --------------------------------------------------

# --------------------------------------------------
# Target specific compile flags
# --------------------------------------------------
if GetOption('debug_build'):
    extra_options = Split("-g -O0")
    global_env["CC"] += " -fstandalone-debug"
    global_env["CXX"] += " -fstandalone-debug"
    build_name = "Debug"
    debug_target = True

    # Configure for Debugger symbols
    if GetOption('lldb'):
        extra_options.extend(Split("-glldb"))

    else:
        extra_options.extend(Split("-ggdb"))

elif GetOption('debug_profiler'):
    extra_options = Split("-g -O0")
    global_env["CC"] += " -fno-omit-frame-pointer"
    global_env["CXX"] += " -fno-omit-frame-pointer"
    build_name = "Debug Profiler"
    debug_target = True

elif GetOption('debug_sanitize'):
    extra_options = Split("-g -O0")
    global_env["CC"] += " -fno-omit-frame-pointer -fsanitize=null, -fsanitize=bounds -fsanitize=undefined -fsanitize=address -fno-sanitize=vptr"
    global_env["CXX"] += " -fno-omit-frame-pointer -fsanitize=null, -fsanitize=bounds -fsanitize=undefined -fsanitize=address -fno-sanitize=vptr"
    build_name = "Debug Sanitize"
    debug_target = True

else:
    extra_options = Split("-g")
    if GetOption('lldb'):
      extra_options.extend(Split("-glldb"))
    else:
      extra_options.extend(Split("-ggdb"))

    extra_options.extend(Split("-O3"))
    build_name = "Release"
    debug_target = False

# Add the extra options to the list
global_options.extend(extra_options)

#print global_env['ENV']['ROOT']
#install_dir = os.path.join(global_env['ENV']['ROOT'], "build")
install_dir = os.path.join(os.getcwd(), "build")

Export('global_env global_options install_dir')

# Build Options Dictionary
build_options = {
  "Debug" : ("build_debug", "debug"),
  "Debug Sanitize" : ("build_sanitize", "debug"),
  "Debug Profiler" : ("build_profiler", "debug"),
  "Release" : ("build_release", "release")
}

##################################################
# Set the build targets
##################################################
global_env.Append(CPPDEFINES = { 'DEBUG_TARGET': str(int(debug_target)) })

build_targets = [
  'src'
  ]

# Get the options for the specified build
build_dir, build_mode = build_options[build_name]

# Build the sub-projects
for t in build_targets:
  SConscript(t + '/SConscript', variant_dir='.' + build_dir + '/' + t, duplicate=0, exports={'MODE' : build_mode})
