#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

// 外部变量声明
extern int insptr;            // pcode指令指针
extern instruction code[200]; // pcode指令数组

static int stack[1024];               // 数据栈
static int max_stack_size = 1024;     // 最大栈大小

int p = 0;    // 指令指针
int b = 1;    // 基址
int t = 1;    // 栈顶

// 被调用函数建立栈帧
static int k = 3;

// 打印指令信息
void print_instruction(int p, instruction ins) {
    printf("p = %d: ", p);
    switch (ins.op) {
    case lit: printf("LIT %d", ins.value); break;
    case opr: printf("OPR %d", ins.value); break;
    case lod: printf("LOD %d", ins.value); break;
    case sto: printf("STO %d", ins.value); break;
    case cal: printf("CAL %d", ins.value); break;
    case ini: printf("INI %d", ins.value); break;
    case jmp: printf("JMP %d", ins.value); break;
    case jpc: printf("JPC %d", ins.value); break;
    case ret: printf("RET"); break;
    default: printf("UNKNOWN"); break;
    }
    printf("\n");
}

// 获取栈数据
void get_stack_data(int* base, int* top, int* stack_data) {
    *base = b;
    *top = t;
    for (int i = 0; i <= t; i++) {
        stack_data[i] = stack[i];
    }
}

// 打印栈内容
void print_stack() {
    printf("Stack: [");
    for (int i = 0; i <= t + k; i++) {
        printf("%d", stack[i]);
        if (i < t + k) printf(", ");
    }
    printf("]\n");
    printf("instruction:%d Base: %d, Top: %d, K:%d\n", p - 1, b, t, k);
    printf("----------------------------------------\n");
}

// 解释执行
void interpret()
{
    int max_instructions = 10000;     // 最大指令执行次数
    int instruction_count = 0;        // 已执行指令计数
    int param_count = 0;              // 当前函数参数计数

    instruction ins; // 当前指令

    // 栈的初始化
    stack[0] = 0;  // stack[0] 不使用
    stack[1] = 0;  // 参数起始位置 base 随调用过程更新
    stack[2] = 0;  // 返回地址
    stack[3] = 0;  // 返回值

    printf("\n=== Program Execution Start ===\n");
    printf("----------------------------------------\n");
    do {
        // 检查指令执行次数
        if (instruction_count++ > max_instructions) {
            printf("\nError: Maximum instruction count exceeded\n");
            printf("Last instruction: ");
            print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        // 检查栈是否溢出
        if (t >= max_stack_size) {
            printf("\nError: Stack overflow\n");
            printf("Last instruction: ");
            print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        // 检查指令指针是否越界
        if (p < 0 || p >= insptr) {
            printf("\nError: Instruction pointer out of bounds (p=%d, insptr=%d)\n", p, insptr);
            printf("Last valid instruction: ");
            if (p > 0) print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        ins = code_read(p++);
        print_instruction(p, ins);  // 打印当前指令

        switch (ins.op)
        {
        case lit:     // 将一个常量放到栈顶 
            stack[++t] = ins.value;
            break;

        case ini:     // 分配栈空间
            t += ins.value;
            if (t >= max_stack_size) {
                printf("\nError: Stack overflow during initialization\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            break;

        case lod:     // 将一个变量放到栈顶
            if (b + ins.value >= max_stack_size) {
                printf("\nError: Invalid variable access\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            stack[++t] = stack[b + ins.value];
            break;

        case sto:     // 将栈顶的值存入一个变量
            if (b + ins.value >= max_stack_size) {
                printf("\nError: Invalid variable access\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            stack[b + ins.value] = stack[t--];
            break;

        case jmp:     // 无条件转移
            if (ins.value < 0 || ins.value >= insptr) {
                printf("\nError: Invalid jump address %d (valid range: 0-%d)\n", ins.value, insptr - 1);
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            p = ins.value;
            break;

        case jpc:     // 栈顶为0则转移
            if (stack[t--] == 0) {
                if (ins.value < 0 || ins.value >= insptr) {
                    printf("\nError: Invalid jump address %d (valid range: 0-%d)\n", ins.value, insptr - 1);
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                p = ins.value;
            }
            break;

        case cal:     // 调用函数
            if (t + 3 >= max_stack_size) {
                printf("\nError: Stack overflow during function call\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            // 保存当前函数的状态
            stack[t + 1] = b;          // 保存当前基址（动态链信息）
            stack[t + 2] = p;          // 保存返回地址
            stack[t + 3] = 0;          // 保存返回值
            b = t + 1;                 // 更新基址
            if (ins.value < 0 || ins.value >= insptr) {
                printf("\nError: Invalid function address %d (valid range: 0-%d)\n", ins.value, insptr - 1);
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            p = ins.value;             // 跳转
            k = 3;
            break;
        case opr:     // 操作
            switch (ins.value)
            {
            case 0:       // 返回 (无返回值)
                t = b - 1;
                p = stack[t + 2];  // 恢复返回地址
                b = stack[t + 1];  // 恢复基址
                break;

            case 1:       // 返回 (有返回值)
                t = b;
                p = stack[t + 1];  // 恢复返回地址
                b = stack[t];  // 恢复基址
                stack[t] = stack[t + 2];  // 将返回值放到正确位置
                break;

            case 2:       // 栈顶元素取负
                stack[t] = -stack[t];
                break;

            case 3:       // 加法
                t--;
                stack[t] = stack[t] + stack[t + 1];
                break;

            case 4:       // 减法
                t--;
                stack[t] = stack[t] - stack[t + 1];
                break;

            case 5:       // 乘法
                t--;
                stack[t] = stack[t] * stack[t + 1];
                break;

            case 6:       // 除法
                t--;
                if (stack[t + 1] == 0) {
                    printf("\nError: Division by zero\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                stack[t] = stack[t] / stack[t + 1];
                break;

            case 7:       // 取模
                t--;
                if (stack[t + 1] == 0) {
                    printf("\nError: Division by zero\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                stack[t] = stack[t] % stack[t + 1];
                break;

            case 8:       // ==
                t--;
                stack[t] = (stack[t] == stack[t + 1] ? 1 : 0);
                break;

            case 9:       // !=
                t--;
                stack[t] = (stack[t] != stack[t + 1] ? 1 : 0);
                break;

            case 10:      // <
                t--;
                stack[t] = (stack[t] < stack[t + 1] ? 1 : 0);
                break;

            case 11:      // <=
                t--;
                stack[t] = (stack[t] <= stack[t + 1] ? 1 : 0);
                break;

            case 12:      // >
                t--;
                stack[t] = (stack[t] > stack[t + 1] ? 1 : 0);
                break;

            case 13:      // >=
                t--;
                stack[t] = (stack[t] >= stack[t + 1] ? 1 : 0);
                break;

            case 14:      // 输出 int
                printf("Output: %d\n", stack[t--]);
                break;

            case 15:      // 输入 int
                printf("Input int: ");
                scanf("%d", &stack[++t]);
                break;

            case 16:      // 参数传递
                if (t + 1 >= max_stack_size) {
                    printf("\nError: Stack overflow during parameter passing\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                if (param_count >= 10) {  // 限制最大参数数量
                    printf("\nError: Too many parameters\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                // 在当前的栈顶进行参数添加
                stack[t + k] = stack[t];
                k++;
                t--;
                break;
            }
            break;
        }
        //print_stack();  // 在每条指令执行后打印栈内容
    } while (p != 0);
    printf("\n=== Program Execution End ===\n");
}