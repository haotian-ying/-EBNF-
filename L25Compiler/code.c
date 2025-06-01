/* �м���������� �����﷨���� */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

instruction code[200];         // ָ���б�
int insptr;                    // ָ��ָ��

// ����һ��pcodeָ��
void code_gen(Operation op, int value)
{
    code[insptr].op = op;
    code[insptr].value = value;
    insptr++;
}

// ��ȡ��p��pcodeָ��
instruction code_read(int p)
{
    return code[p];
}

// �����begin��ʼ��pcode�б�
void code_list(int begin)
{
    for (int i = begin; i < insptr; i++) {
        printf("%2d %d %2d\n",
            i, code[i].op, code[i].value);
    }
}