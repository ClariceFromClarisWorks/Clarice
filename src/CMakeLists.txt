cmake_minimum_required(VERSION 3.16)
project(Clarice C ASM)
add_application(Clarice app.c shell.c sizes.c reflow.c kernels.S checksum.S App.r Icons.r ../work/build/Commands.r TYPE APPL CREATOR CLRC)
target_compile_options(Clarice PRIVATE -Os -m68000 -Wall -Wextra -Werror -Wno-multichar -Wno-misleading-indentation -ffunction-sections -fdata-sections)
set_target_properties(Clarice PROPERTIES LINK_FLAGS "-Wl,-gc-sections -Wl,--mac-single")

target_sources(Clarice PRIVATE vendor/mrsh/ast.c vendor/mrsh/array.c vendor/mrsh/buffer.c vendor/mrsh/arithm.c vendor/mrsh/parser/parser.c vendor/mrsh/parser/program.c vendor/mrsh/parser/word.c vendor/mrsh/parser/arithm.c)
target_include_directories(Clarice PRIVATE vendor/mrsh/include)
set_source_files_properties(vendor/mrsh/parser/parser.c PROPERTIES COMPILE_OPTIONS -Wno-unused-parameter)
