/* ����ִ��pcode���� */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static int stack[1024];               // ����ջ

// ����ִ��
void interpret()
{
    int p = 0;    // ָ��
    int b = 1;    // ����ַ
    int t = 0;    // ջ��
    int k = 4;    // ����λ��

    instruction ins; // ��ŵ�ǰָ��

    // ջ�ĳ�ʼ��
    stack[0] = 0;  // stack[0] ��ʹ��
    stack[1] = 0;  // ��̬��
    stack[2] = 0;  // ��̬��
    stack[3] = 0;  // ���ص�ַ
    stack[4] = 0;  // ����ֵ

    do {
        ins = code_read(p++);  // ����һ��ָ��
        switch (ins.op)
        {
        case lit:     // ��һ���������ŵ�ջ�� 
            stack[++t] = ins.value;
            break;

        case ini:     // ����ջ�ռ�
            t += ins.value;
            break;

        case lod:     // ��һ�������ŵ�ջ��(��ʱջ���������������ƫ����)
            stack[t] = stack[b + ins.value + stack[t]];
            break;

        case sto:    // ��ջ����ֵ����һ������(��ʱ��ջ���������������ƫ����)
            stack[b + ins.value + stack[t - 1]] = stack[t];
            t = t - 2;
            break;

        case jmp:    // ��������ת
            p = ins.value;
            break;

        case jpc:    // ջ��Ϊ0��ת
            if (stack[t--] == 0)
                p = ins.value;
            break;

        case cal:   // ��������
            stack[t + 1] = b; // ��̬��
            stack[t + 2] = b;                      // ��̬��
            stack[t + 3] = p;                      // ���ص�ַ
            stack[t + 4] = 0;                      // ����ֵ
            b = t + 1;                             // ���»���ַ
            p = ins.value;                         // ��ת
            k = 4;                                 // ����kΪ��ʼ״̬ 
            break;

        case opr:  // ����
            switch (ins.value)
            {
            case 0:       // ���÷��� (�޷���ֵ)
                t = b - 1;
                b = stack[t + 2];
                p = stack[t + 3];
                break;

            case 1:       // ���÷��� (�з���ֵ, ����ֵ����ջ��)
                t = b;
                b = stack[t + 1];
                p = stack[t + 2];
                stack[t] = stack[t + 3];
                break;

            case 2:       // ջ��Ԫ��ȡ��
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
                stack[t] = stack[t] / stack[t + 1];
                break;

            case 7:       // ȡ��
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

            case 14:       // ��� int
                printf("%d\n", stack[t--]);
                break;

            case 15:       // ���� int
                printf("input int: ");
                scanf("%d", &stack[++t]);
                break;

            case 16:       // ��� bool
                if (stack[t] == 1) printf("true\n");
                else if (stack[t] == 0) printf("false\n");
                else printf("not bool\n");
                t--;
                break;

            case 17:       // ���� bool
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

            case 21:     // ������������
                stack[t + k] = stack[t];
                k++;
                t--;
                break;
            }
            break;
        }
    } while (p != 0);
}