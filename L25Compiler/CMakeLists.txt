# CMakeList.txt: L25Compiler 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable(L25Compiler 
    main.c
    lexer.c
    block.c
    code.c
    "interprete.c")
