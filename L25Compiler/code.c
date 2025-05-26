/* 中间代码生成 和语法分析 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

instruction code[200];         // 指令列表
int insptr;                    // 指令指针

// 生成一条pcode指令
void code_gen(Operation op, int value)
{
    if (insptr >= 200) {
        printf("Error: Code array overflow\n");
        exit(1);
    }
    code[insptr].op = op;
    code[insptr].value = value;
    insptr++;
}

// 读取第p条pcode指令
instruction code_read(int p)
{
    if (p < 0 || p >= insptr) {
        printf("Error: Invalid instruction pointer %d (valid range: 0-%d)\n", p, insptr - 1);
        exit(1);
    }
    return code[p];
}

// 从begin开始列出pcode列表
void code_list(int begin)
{
    for (int i = begin; i < insptr; i++) {
        printf("%2d %d %2d\n",
            i, code[i].op, code[i].value);
    }
}