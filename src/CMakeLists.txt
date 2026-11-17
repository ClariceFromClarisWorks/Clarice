cmake_minimum_required(VERSION 3.16)
project(Clarice C ASM)
if(CLARICE_OFFLINE_INSTALLER)
 set(INSTALLER_RESOURCE Installer.r)
endif()
add_application(Clarice app.c hardware.c shell.c sizes.c reflow.c planes-unrolled.S glyph-planes.S kernels.S checksum.S App.r Icons.r ${INSTALLER_RESOURCE} TYPE APPL CREATOR CLRC)
# Retro68's Rez rule tracks App.r itself, but not its included build stamp.
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/App.r.rsrc.bin" APPEND
 DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/update/version.h")
target_compile_options(Clarice PRIVATE -Os -m68000 -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -ffunction-sections -fdata-sections)
set_target_properties(Clarice PROPERTIES LINK_FLAGS "-Wl,-gc-sections -Wl,--mac-single")

target_sources(Clarice PRIVATE vendor/mrsh/ast.c vendor/mrsh/array.c vendor/mrsh/buffer.c vendor/mrsh/arithm.c vendor/mrsh/parser/parser.c vendor/mrsh/parser/program.c vendor/mrsh/parser/word.c vendor/mrsh/parser/arithm.c)
target_include_directories(Clarice PRIVATE vendor/mrsh/include)
set_source_files_properties(vendor/mrsh/parser/parser.c PROPERTIES COMPILE_OPTIONS -Wno-unused-parameter)

# Opt-in guest diagnostics; absent from ordinary application builds.
if(CLARICE_INPUT_DIAGNOSTICS)
 target_compile_definitions(Clarice PRIVATE CLARICE_INPUT_DIAGNOSTICS=1)
endif()

if(CLARICE_INPUT_SAFETY_TEST)
 target_compile_definitions(Clarice PRIVATE CLARICE_INPUT_SAFETY_TEST=1)
endif()

if(CLARICE_SELECTION_DIAGNOSTICS)
 target_compile_definitions(Clarice PRIVATE CLARICE_SELECTION_DIAGNOSTICS=1)
endif()
if(CLARICE_SELECTION_FIXED)
 target_compile_definitions(Clarice PRIVATE CLARICE_SELECTION_FIXED=1)
endif()

if(CLARICE_USABILITY_DIAGNOSTICS)
 target_compile_definitions(Clarice PRIVATE CLARICE_USABILITY_DIAGNOSTICS=1)
endif()

if(CLARICE_NANO_PERF)
 target_compile_definitions(Clarice PRIVATE CLARICE_NANO_PERF=1)
endif()

if(CLARICE_DRAW_DIAGNOSTICS)
 target_compile_definitions(Clarice PRIVATE CLARICE_DRAW_DIAGNOSTICS=1)
endif()

if(CLARICE_ACTIVITY_DIAGNOSTICS)
 target_compile_definitions(Clarice PRIVATE CLARICE_ACTIVITY_DIAGNOSTICS=1)
endif()

# Deterministic parser exhaustion checks; excluded from production applications.
if(CLARICE_PARSER_SAFETY_TEST)
 target_compile_definitions(Clarice PRIVATE CLARICE_PARSER_SAFETY_TEST=1)
 target_sources(Clarice PRIVATE ../tests/parser/guest.c)
 set_property(SOURCE vendor/mrsh/ast.c vendor/mrsh/array.c vendor/mrsh/buffer.c
  vendor/mrsh/arithm.c vendor/mrsh/parser/parser.c vendor/mrsh/parser/program.c
  vendor/mrsh/parser/word.c vendor/mrsh/parser/arithm.c APPEND PROPERTY
  COMPILE_OPTIONS -include "${CMAKE_CURRENT_SOURCE_DIR}/../tests/parser/guest.h")
endif()

# Offline installation launcher only; the ordinary core gets no installer code.
if(CLARICE_OFFLINE_INSTALLER)
 target_compile_definitions(Clarice PRIVATE CLARICE_OFFLINE_INSTALLER=1)
 target_compile_options(Clarice PRIVATE -Wno-unused-function -Wno-sign-compare)
endif()
