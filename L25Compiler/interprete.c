/* 解释执行pcode代码 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static int stack[1024];               // 运行栈

// 解释执行
void interpret()
{
    int p = 0;    // 指令
    int b = 1;    // 基地址
    int t = 0;    // 栈顶
    int k = 4;    // 参数位置

    instruction ins; // 存放当前指令

    // 栈的初始化
    stack[0] = 0;  // stack[0] 不使用
    stack[1] = 0;  // 静态链
    stack[2] = 0;  // 动态链
    stack[3] = 0;  // 返回地址
    stack[4] = 0;  // 返回值

    do {
        ins = code_read(p++);  // 读出一条指令
        switch (ins.op)
        {
        case lit:     // 将一个立即数放到栈顶 
            stack[++t] = ins.value;
            break;

        case ini:     // 开辟栈空间
            t += ins.value;
            break;

        case lod:     // 将一个变量放到栈顶(此时栈顶放着这个变量的偏移量)
            stack[t] = stack[b + ins.value + stack[t]];
            break;

        case sto:    // 将栈顶的值存入一个变量(此时次栈顶放着这个变量的偏移量)
            stack[b + ins.value + stack[t - 1]] = stack[t];
            t = t - 2;
            break;

        case jmp:    // 无条件跳转
            p = ins.value;
            break;

        case jpc:    // 栈顶为0跳转
            if (stack[t--] == 0)
                p = ins.value;
            break;

        case cal:   // 函数调用
            stack[t + 1] = b; // 静态链
            stack[t + 2] = b;                      // 动态链
            stack[t + 3] = p;                      // 返回地址
            stack[t + 4] = 0;                      // 返回值
            b = t + 1;                             // 更新基地址
            p = ins.value;                         // 跳转
            k = 4;                                 // 更新k为初始状态 
            break;

        case opr:  // 运算
            switch (ins.value)
            {
            case 0:       // 调用返回 (无返回值)
                t = b - 1;
                b = stack[t + 2];
                p = stack[t + 3];
                break;

            case 1:       // 调用返回 (有返回值, 返回值放在栈顶)
                t = b;
                b = stack[t + 1];
                p = stack[t + 2];
                stack[t] = stack[t + 3];
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
                stack[t] = stack[t] / stack[t + 1];
                break;

            case 7:       // 取余
                t--;
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

            case 11:       // >=
                t--;
                stack[t] = (stack[t] >= stack[t + 1] ? 1 : 0);
                break;

            case 12:       // >
                t--;
                stack[t] = (stack[t] > stack[t + 1] ? 1 : 0);
                break;

            case 13:       // <=
                t--;
                stack[t] = (stack[t] <= stack[t + 1] ? 1 : 0);
                break;

            case 14:       // 输出 int
                printf("%d\n", stack[t--]);
                break;

            case 15:       // 输入 int
                printf("input int: ");
                scanf("%d", &stack[++t]);
                break;

            case 16:       // 输出 bool
                if (stack[t] == 1) printf("true\n");
                else if (stack[t] == 0) printf("false\n");
                else printf("not bool\n");
                t--;
                break;

            case 17:       // 输入 bool
                char str[10];
                printf("input bool: ");
                scanf("%s", str);
                t++;
                if (str[0] == 't') stack[t] = 1;
                else stack[t] = 0;
                break;

            case 18:      // &
                t--;
                if (stack[t] == 0 && stack[t + 1] == 0) stack[t] = 0;
                else if (stack[t] == 0 && stack[t + 1] == 1) stack[t] = 0;
                else if (stack[t] == 1 && stack[t + 1] == 0) stack[t] = 0;
                else if (stack[t] == 1 && stack[t + 1] == 1) stack[t] = 1;
                break;

            case 19:      // |
                t--;
                if (stack[t] == 0 && stack[t + 1] == 0) stack[t] = 0;
                else if (stack[t] == 0 && stack[t + 1] == 1) stack[t] = 1;
                else if (stack[t] == 1 && stack[t + 1] == 0) stack[t] = 1;
                else if (stack[t] == 1 && stack[t + 1] == 1) stack[t] = 1;
                break;

            case 20:     // !
                if (stack[t] == 0) stack[t] = 1;
                else if (stack[t] == 1) stack[t] = 0;
                break;

            case 21:     // 函数参数传入
                stack[t + k] = stack[t];
                k++;
                t--;
                break;
            }
            break;
        }
    } while (p != 0);
}