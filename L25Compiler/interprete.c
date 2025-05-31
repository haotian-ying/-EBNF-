#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

extern int insptr;            // pcodeָ
extern instruction code[200]; // pcodeָ

static int stack[1024];               
static int max_stack_size = 1024;     
int p = 0;   
int b = 1;    
int t = 1;    

// 开辟的额外空间
static int k = 3;

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

void get_stack_data(int* base, int* top, int* stack_data) {
    *base = b;
    *top = t;
    for (int i = 0; i <= t; i++) {
        stack_data[i] = stack[i];
    }
}

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

void interpret()
{
    int max_instructions = 10000;     
    int instruction_count = 0;        
    int param_count = 0;              

    instruction ins; 

    
    stack[0] = 0;  // stack[0]空出
    stack[1] = 0;  // 保存base
    stack[2] = 0;  // 保存动态链
    stack[3] = 0;  // 保存返回值

    printf("\n=== Program Execution Start ===\n");
    printf("----------------------------------------\n");
    do {
        if (instruction_count++ > max_instructions) {
            printf("\nError: Maximum instruction count exceeded\n");
            printf("Last instruction: ");
            print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        if (t >= max_stack_size) {
            printf("\nError: Stack overflow\n");
            printf("Last instruction: ");
            print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        if (p < 0 || p >= insptr) {
            printf("\nError: Instruction pointer out of bounds (p=%d, insptr=%d)\n", p, insptr);
            printf("Last valid instruction: ");
            if (p > 0) print_instruction(p - 1, code[p - 1]);
            exit(1);
        }

        ins = code_read(p++);
        print_instruction(p, ins);  

        switch (ins.op)
        {
        case lit:     
            stack[++t] = ins.value;
            break;

        case ini:     // 开辟空间
            t += ins.value;
            if (t >= max_stack_size) {
                printf("\nError: Stack overflow during initialization\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            break;

        case lod:     
            if (b + ins.value >= max_stack_size) {
                printf("\nError: Invalid variable access\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            stack[++t] = stack[b + ins.value];
            break;

        case sto:     
            if (b + ins.value >= max_stack_size) {
                printf("\nError: Invalid variable access\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            stack[b + ins.value] = stack[t--];
            break;

        case jmp:     
            if (ins.value < 0 || ins.value >= insptr) {
                printf("\nError: Invalid jump address %d (valid range: 0-%d)\n", ins.value, insptr - 1);
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            p = ins.value;
            break;

        case jpc:  
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

        case cal:   
            if (t + 3 >= max_stack_size) {
                printf("\nError: Stack overflow during function call\n");
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            // 变换基址等
            stack[t + 1] = b;          // 
            stack[t + 2] = p;          //
            stack[t + 3] = 0;          // 
            b = t + 1;                 // 
            if (ins.value < 0 || ins.value >= insptr) {
                printf("\nError: Invalid function address %d (valid range: 0-%d)\n", ins.value, insptr - 1);
                printf("Instruction: ");
                print_instruction(p - 1, ins);
                exit(1);
            }
            p = ins.value;             // 跳转至对应指令
            k = 3;
            break;
        case opr:     
            switch (ins.value)
            {
            case 0:       // main返回
                t = b - 1;
                p = stack[t + 2];  
                b = stack[t + 1];  
                break;

            case 1:       // function的有返回值返回
                t = b;
                p = stack[t + 1];  
                b = stack[t];  
                stack[t] = stack[t + 2];  
                break;

            case 2:       // ��ջԪ��ȡ��
                stack[t] = -stack[t];
                break;

            case 3:       // �ӷ�
                t--;
                stack[t] = stack[t] + stack[t + 1];
                break;

            case 4:       // ����
                t--;
                stack[t] = stack[t] - stack[t + 1];
                break;

            case 5:       // �˷�
                t--;
                stack[t] = stack[t] * stack[t + 1];
                break;

            case 6:       // ����
                t--;
                if (stack[t + 1] == 0) {
                    printf("\nError: Division by zero\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                stack[t] = stack[t] / stack[t + 1];
                break;

            case 7:       // ȡģ
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

            case 14:      // ��� int
                printf("\n+--------------------------------------------------+\n");
                printf("|                  PROGRAM OUTPUT                    |\n");
                printf("+--------------------------------------------------+\n");
                printf("| Value: %-42d |\n", stack[t--]);
                printf("+--------------------------------------------------+\n\n");
                break;

            case 15:      // ���� int
                printf("Input int: ");
                scanf("%d", &stack[++t]);
                break;

            case 16:      // ��������
                if (t + 1 >= max_stack_size) {
                    printf("\nError: Stack overflow during parameter passing\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                if (param_count >= 10) {  // ������������
                    printf("\nError: Too many parameters\n");
                    printf("Instruction: ");
                    print_instruction(p - 1, ins);
                    exit(1);
                }
                // �ڵ�ǰ��ջ�в������
                stack[t + k] = stack[t];
                k++;
                t--;
                break;
            }
            break;
        }
        print_stack();  // ÿ��ָ��ִ�к��ӡ��ջ
    } while (p != 0);
    printf("\n=== Program Execution End ===\n");
}