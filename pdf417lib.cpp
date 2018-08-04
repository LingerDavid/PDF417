/*
 * Copyright 2003-2005 by Paulo Soares.
 *
 * The contents of this file are subject to the Mozilla Public License Version 1.1
 * (the "License"); you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the License.
 *
 * The Original Code is 'pdf417lib, a library to generate the bidimensional barcode PDF417'.
 *
 * The Initial Developer of the Original Code is Paulo Soares. Portions created by
 * the Initial Developer are Copyright (C) 2003 by Paulo Soares.
 * All Rights Reserved.
 *
 * Contributor(s): all the names of the contributors are added in the source code
 * where applicable.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * LGPL license (the "GNU LIBRARY GENERAL PUBLIC LICENSE"), in which case the
 * provisions of LGPL are applicable instead of those above.  If you wish to
 * allow use of your version of this file only under the terms of the LGPL
 * License and not to allow others to use your version of this file under
 * the MPL, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the LGPL.
 * If you do not delete the provisions above, a recipient may use your version
 * of this file under either the MPL or the GNU LIBRARY GENERAL PUBLIC LICENSE.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MPL as stated above or under the terms of the GNU
 * Library General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Library general Public License for more
 * details.
 *
 * If you didn't download this code from the following link, you should check if
 * you aren't using an obsolete version:
 * http://sourceforge.net/projects/pdf417lib
 */
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>

#define __INCLUDE_PDF417LIBIMP_H__
#include "pdf417libimp.h"
#undef __INCLUDE_PDF417LIBIMP_H__
#include "pdf417lib.h"


/**************************************************************
*��GLI(ȫ���ʶ��Ƿ�)Ϊ0ʱ��TCģʽ�µĵ�ASCII�ַ���Ӧֵ
***************************************************************/

/**************************************************************
*MIXEDģʽ�ַ�     0123456789&rt,:#-.$/+%*=^
***************************************************************/
char* MIXED_SET = "0123456789&\r\t,:#-.$/+%*=^";
/**************************************************************
*PUNCUTATIONģʽ�ַ�     ;<>@[\]_`~!rt,:n-.$/"|*()?{}'
***************************************************************/
char* PUNCTUATION_SET = ";<>@[\\]_`~!\r\t,:\n-.$/\"|*()?{}'";




/**************************************************
*��ʼ���ṹ��listElement
***************************************************/
void listInit(pArrayList list) {
    list->capacity = 20;
    list->size = 0;
    list->array = (pListElement)malloc(list->capacity * sizeof(listElement));
}

/**************************************************
*�ͷŽṹ��listElement�ڴ�
***************************************************/
static void listFree(pArrayList list) {
    free(list->array);
    list->array = NULL;
}

/**************************************************
*������ݵ�listElement
***************************************************/
static void listAdd(pArrayList list, char type, int start, int end) {
    if (list->size == list->capacity) {
        pListElement temp;
        list->capacity *= 2;
        temp = (pListElement)malloc(list->capacity * sizeof(listElement));
        memcpy(temp, list->array, list->size * sizeof(listElement));
        free(list->array);
        list->array = temp;
    }
    list->array[list->size].type = type;
    list->array[list->size].start = start;
    list->array[list->size].end = end;
    ++list->size;
}

/**************************************************
*��listElement��ȡ����
***************************************************/
static pListElement listGet(pArrayList list, int idx) {
    if (idx >= list->size || idx < 0)
        return NULL;
    return list->array + idx;
}

/**************************************************
*��listElement�Ƴ�����
***************************************************/
static void listRemove(pArrayList list, int idx) {
    if (idx >= list->size || idx < 0)
        return;
    --list->size;
    memmove(list->array + idx, list->array + (idx + 1), (list->size - idx) * sizeof(listElement));
}

/**************************************************
*���listElementָ��Ԫ��
***************************************************/
static int checkElementType(pListElement p, char type) {
    if (!p)
        return 0;
    return (p->type == type);
}

static int getElementLength(pListElement p) {
    if (!p)
        return 0;
    return p->end - p->start;
}

/*************************************************
*��ʼ���ṹ��pPdf417class
*************************************************/
void pdf417classInit(pPdf417class p)
{
	p->cwPtr = 0;
	p->cwPtr = 0;
	p->param = NULL;
}

/**************************************************
*��ʼ���ṹ��pPdf417param
***************************************************/
void pdf417init(pPdf417param param) {
    param->options = 0;
    param->outBits = NULL;
    param->lenBits = 0;
    param->error = 0;
    param->lenText = 0;																						//ԭʼ����λ-1,�ָ�Ϊ0
    param->text = "";
    param->yHeight = 3;
    param->aspectRatio = 0.5;
	param->codeColumns = 0;
	param->codeRows = 0;
	memset(param->codewords, 0, sizeof(param->codewords));
}

/**************************************************
*�ͷŽṹ��pPdf417param�ڴ�
***************************************************/
void pdf417free(pPdf417param param) {
    if (param->outBits != NULL) {
        free(param->outBits);
        param->outBits = NULL;
    }
	if(param->text != NULL)
	{
		param->text = NULL;
	}
}

/**************************************************
*�ṹ��pPdf417param,17λ���ֲ���
***************************************************/
void outCodeword17(pPdf417class p, int codeword) {
    int bytePtr = p->bitPtr / 8;																		//�����ֽ���
    int bit = p->bitPtr - bytePtr * 8;																	//����ƫ�Ʊ�����
    p->param->outBits[bytePtr++] |= codeword >> (9 + bit) & (0xFF >> bit);								//��������λ9+ƫ����,ʹ���ַ��������ֵ�һ��8λ				
    p->param->outBits[bytePtr++] |= codeword >> (1 + bit) & 0xFF;										//��������λ1+ƫ����,ʹ���ַ��������ֵڶ���8λ
    codeword <<= 8;
    p->param->outBits[bytePtr] |= codeword >> (1 + bit) & ((0xFF >> (7 - bit)) << (7 - bit));								//��������λ1+ƫ����,ʹ���ַ������������1λ
    p->bitPtr += 17;																							//����λ�ƶ�����һ�ַ�
}

/***************************************************
*����17λ����
****************************************************/
void inCodeword17(pPdf417class p, int *codeword){
	int bytePtr = p->bitPtr / 8;
	int bit = p->bitPtr - bytePtr * 8;
	(*codeword) = 0;																				//00000001 11111111 11111110
	(*codeword) |= ((int)(p->param->outBits[bytePtr++] & 0xFF) & (0xFF >> bit)) << (9 + bit);						//��һ��8-bitλ
	(*codeword) |= ((int)(p->param->outBits[bytePtr++] & 0xFF)  << (1 + bit)) & (0xFF << (1 + bit));			//�ڶ���8λ
	(*codeword) |= (int)(p->param->outBits[bytePtr] & 0xFF) >> (7 - bit);								//������1+bitλ	
	p->bitPtr += 17;
}

/**************************************************
*�ṹ��pPdf417param,18λ���ֲ���
***************************************************/
static void outCodeword18(pPdf417class p, int codeword) {
    int bytePtr = p->bitPtr / 8;
    int bit = p->bitPtr - bytePtr * 8;
    p->param->outBits[bytePtr++] |= codeword >> (10 + bit);
    p->param->outBits[bytePtr++] |= codeword >> (2 + bit);
    codeword <<= 8;
    p->param->outBits[bytePtr] |= codeword >> (2 + bit);
    if (bit == 7)
        p->param->outBits[++bytePtr] |= 0x80;
    p->bitPtr += 18;
}

/***************************************************
*����18λ���� δ���
****************************************************/
static void inCodeword18(pPdf417class p, int *codeword){
	int bytePtr = p->bitPtr / 8;
	int bit = p->bitPtr - bytePtr * 8;
	(*codeword) = 0;
	(*codeword) |= (p->param->outBits[bytePtr++] & (0xFF >> bit)) << (10 + bit);			//��һ��8-bitλ
	(*codeword) |= p->param->outBits[bytePtr++] << (2 + bit);								//�ڶ���8λ
	(*codeword) |= p->param->outBits[bytePtr] >> (8 - bit);									//������1+bitλ
	p->bitPtr += 18;
}

/**************************************************
*��ָ�����ִ���
***************************************************/
static void outCodeword(pPdf417class p, int codeword) {
    outCodeword17(p, codeword);
}

/**************************************************
*����ָ������
***************************************************/
static void inCodeword(pPdf417class p, int *codeword) {
    inCodeword17(p, codeword);
}

/**************************************************
*����ֹ���ִ���
***************************************************/
static void outStopPattern(pPdf417class p) {
    outCodeword18(p, STOP_PATTERN);
}


/**************************************************
*����ʼ���ִ���
***************************************************/
static void outStartPattern(pPdf417class p) {
    outCodeword17(p, START_PATTERN);
}

 
/**********************************************
*���ݴ�����Ѱ��������ȼ�
***********************************************/
static int maxPossibleErrorLevel(int remain) {
    int level = 8;
    int size = 512;
    while (level > 0) {
        if (remain >= size)
            return level;
        --level;
        size >>= 1;
    }
    return 0;
}

/***************************************************
*�����������Ӧ����
****************************************************/
static int getCodeByOutbits(int *codeword, int rowmod)
{
	int i;
	for(i = 0; i < MOD; i++)
	{
		if((*codeword) == CLUSTERS[rowmod][i])
		{
			(*codeword) = i;
			return 1;
		}
	}
	return 0;
}

/**************************************************
*��������Ӧ�����
 7 331 83 158 2a2 209 77
ff 54 7a 9e 3a 8e 1d 5c  f e8 a4 
ff 54 7a 84 3e 1a 9f a9 cf e8 a4 
ff 54 6a f8 39 5f 95  f  f e8 a4 
ff 54 7d 7e a4 1b 1a f7 cf e8 a4 
ff 54 6b 82 38 58 9d 71 8f e8 a4 
ff 54 7d 70 bb d3 9e be cf e8 a4 
ff 54 69 cf 34 31 d4 ee  f e8 a4 
ff 54 7e 97 2f d9 d5 f9 cf e8 a4 
ff 54 53 7c 20 53 df 4e 8f e8 a4 
ff 54 51 dc 32 73 d4 63  f e8 a4 
ff 54 69 c2 3c 49 1d 39 8f e8 a4 
ff 54 68 9f 2e 63 9f a3 2f e8 a4 
ff 54 50 50 32 4c 14 30 cf e8 a4
ff 54 7a 21 35 c2 1e 89  f e8 a4 
ff 54 7a 1e a9 df 94  9 ef e8 a4
***************************************************/
void outPaintCode(pPdf417class p) {
    int codePtr = 0;
    int row;
    int rowMod;
    int *cluster;
    int edge;
    int column;
    p->param->bitColumns = START_CODE_SIZE * (p->param->codeColumns + 3) + STOP_SIZE;			//��ʼ��������+���ݱ�����+��ֹ��������+�ҿհ׷�
    p->param->lenBits = ((p->param->bitColumns - 1) / 8 + 1) * p->param->codeRows;				//����((�������� - һλ�ҿհ׷�)/8+1)*�����õ����ֽڳ�
    p->param->outBits = (char*)malloc(p->param->lenBits);										//�����ڴ�ռ�
    memset(p->param->outBits, 0, p->param->lenBits);											//��ʼ��Ϊ0
	
    for (row = 0; row < p->param->codeRows; ++row) {
        p->bitPtr = ((p->param->bitColumns - 1) / 8 + 1) * 8 * row;								//�����row�еı�����ʼλ
        rowMod = row % 3;																		//���㵱ǰ�����ڴ�
        cluster = CLUSTERS[rowMod];																//ȡ����Ӧ����
		/**************************************************
		*���㵱ǰ������ʼ������
		***************************************************/
        outStartPattern(p);																
        edge = 0;
		/**************************************************
		*���㵱ǰ����ָʾ������
		***************************************************/
        switch (rowMod) {
        case 0:
            edge = 30 * (row / 3) + ((p->param->codeRows - 1) / 3);								//����row�Ǵ�0��ʼ���Բ��ü�1,30*INT[(�к�-1)/3]+INT[(����-1)/3]
            break;
        case 1:
            edge = 30 * (row / 3) + p->param->errorLevel * 3 + ((p->param->codeRows - 1) % 3);	//30*INT[(�к�-1)/3]+��������ȼ�*3+(����-1)mod3
            break;
        default:
            edge = 30 * (row / 3) + p->param->codeColumns - 1;									//30*INT[(�к�-1)/3]+����������-1
            break;
        }
        outCodeword(p, cluster[edge]);
		/**************************************************
		*���㵱ǰ����������
		***************************************************/
        for (column = 0; column < p->param->codeColumns; ++column) {
			//printf("%2x ", p->param->codewords[codePtr]);
            outCodeword(p, cluster[p->param->codewords[codePtr++]]);
        }
        /**************************************************
		*���㵱ǰ����ָʾ������
		***************************************************/
        switch (rowMod) {
        case 0:
            edge = 30 * (row / 3) + p->param->codeColumns - 1;									//30*INT[(�к�-1)/3]+����������-1
            break;
        case 1:
            edge = 30 * (row / 3) + ((p->param->codeRows - 1) / 3);								//����row�Ǵ�0��ʼ���Բ��ü�1,30*INT[(�к�-1)/3]+INT[(����-1)/3]
            break;
        default:
            edge = 30 * (row / 3) + p->param->errorLevel * 3 + ((p->param->codeRows - 1) % 3);	//30*INT[(�к�-1)/3]+��������ȼ�*3+(����-1)mod3
            break;
        }
        outCodeword(p, cluster[edge]);
		/**************************************************
		*���㵱ǰ������ֹ������
		***************************************************/
        outStopPattern(p);
    }
	/**************************************************
	*�Ƿ���ѡ��תͼƬ
	***************************************************/
    if (p->param->options & PDF417_INVERT_BITMAP) {
        char* pm = p->param->outBits;															//ȡ����������
        char* end = pm + p->param->lenBits;														//ָ���ƶ������һ���ַ��ĵ�ַ
        /*************************************************
		*λͼ��תֱ��β��ַ
		**************************************************/
		while (pm < end)
            *(pm++) ^= 0xff;
    }

	
}

/**************************************************
*��������Ӧ����
***************************************************/
/**************************************************
*This is the code result of the test bitmapfile.
ff 54 7a 9e 3a 8e 1d 5c  f e8 a4 
ff 54 7a 84 3e 1a 9f a9 cf e8 a4 
ff 54 6a f8 39 5f 95  f  f e8 a4 
ff 54 7d 7e a4 1b 1a f7 cf e8 a4 
ff 54 6b 82 38 58 9d 71 8f e8 a4 
ff 54 7d 70 bb d3 9e be cf e8 a4 
ff 54 69 cf 34 31 d4 ee  f e8 a4 
ff 54 7e 97 2f d9 d5 f9 cf e8 a4 
ff 54 53 7c 20 53 df 4e 8f e8 a4 
ff 54 51 dc 32 73 d4 63  f e8 a4 
ff 54 69 c2 3c 49 1d 39 8f e8 a4 
ff 54 68 9f 2e 63 9f a3 2f e8 a4 
ff 54 50 50 32 4c 14 30 cf e8 a4 
ff 54 7a 21 35 c2 1e 89  f e8 a4 
ff 54 7a 1e a9 df 94  9 ef e8 a4

 //uncompress the codewords from the bitmap pixels
 7 31 83 58 a2  9 77 8d fb e0 fb 9c c2 24 46


ff 54 6a 30 29 dc 10 42 2d b0 c5 bd c3 a9 c1 fd 14 80
ff 54 7d 43 2f d6 1c 16 4e  b 27  5 93 ea 31 fd 14 80
ff 54 6a 7c 27 d9 17 b0 6a e3 f6 7e cb 50 f9 fd 14 80
ff 54 7a 5e 3c 93 d6 f1 ce d0 c7 24 e2 be f9 fd 14 80
ff 54 75 c1 af d6 1c 16 4c 17 47 5f 7b 5c 21 fd 14 80
ff 54 7a f2 31 e5 98 f2 cc 7a c5 33 e3 af 91 fd 14 80
ff 54 69 c7 b4 c3 90 42 2d b0 c5 bd c2 9c e1 fd 14 80
ff 54 7a 48 2f d6 1e f7 af 5c e7  5 db d2 81 fd 14 80
ff 54 53 3e 21 e2 91 b1 e8 28 f7 e4 d3 f4 ed fd 14 80
ff 54 51 ce 23 73 1b 6c  8 9c 76 42 62 8c 19 fd 14 80
ff 54 74 e1 bd 48 18 47 2f 47 74 43 d3 4e 21 fd 14 80
ff 54 51  f 3c be d3 8d c9 ce f4 f3 32 8c f9 fd 14 80
ff 54 74 17 3e af 9c 42 ef 4d f7 10 ba 87 1d fd 14 80
ff 54 7a 20 ba 39 9e b0 68 df 14 4f 43 d1 11 fd 14 80
ff 54 68 2f a3 37 98 f9 2c 7c 96 3e 4a 57 81 fd 14 80
ff 54 72 de 3a 77 dd ed  e 62 36 f3 93 28 61 fd 14 80
ff 54 51 f3 3a c4 1e a4  e bc e5 78 82 8f  5 fd 14 80
ff 54 4b 9f b9 fa 99 1f a9 c3 74 18 bb 97 d1 fd 14 80
ff 54 76 86 39 85 9c f3 4c d0 86 43 73 b4 61 fd 14 80
****************************************************/
void inPaintCode(pPdf417class p)
{
    int row, col;
	int rowMod, skip_col;
	int maxErr;
	if(p->param->codeRows <=0 || p->param->codeColumns <= 0)
	{
		return ;
	}
	p->cwPtr = 0;
	printf("\n");
	//�������𻵵�����½�����ȡ�����ֽڲ����н���
	for (row = 0; row < p->param->codeRows; ++row)																				//��֪codeRows��codeCols
	{
		skip_col = START_CODE_SIZE / 4;
		
		rowMod = row % 3;
		p->bitPtr = p->param->codeColumns * 8 * row + 2 * START_CODE_SIZE;												//������ʼ��ַ
		for(col = skip_col; col < p->param->codeColumns - skip_col - 2; col += 2, p->cwPtr++)
		{
			inCodeword17(p, p->param->codewords + (p->cwPtr));
			if(getCodeByOutbits(p->param->codewords + p->cwPtr, rowMod) == 0)
			{
				//printf("dont't find it");
				return ;
			}
			//printf("%x ", p->param->outBits[row * p->param->codeColumns + col] & 0xFF);
		}
		//printf("\n");
		/*for(col = 0; col < p->param->codeColumns; col++)
		{
			printf("%2x ", p->param->outBits[row * p->param->codeColumns + col] & 0xFF);
		}
		printf("\n");*/
	}
	p->param->lenCodewords = p->param->codewords[0];																				//��һ��codewordΪ���ֳ���
	maxErr = maxPossibleErrorLevel(MAX_DATA_CODEWORDS + 2 - p->param->lenCodewords);
	/*for(row = 1; row < p->param->lenCodewords; row++)	
	{
		printf("%2x ", p->param->codewords[row]);
	}
	printf("\n");*/
}

/***********************************************************
*�����������-����Reed-Solomon�㷨
************************************************************/
static void calculateErrorCorrection(pPdf417class p, int dest) {
    int t1 = 0;
    int t2 = 0;
    int t3 = 0;
    int *A;
    int Alength;
    int *E;
    int lastE;
    int k, e, j;
	
    if (p->param->errorLevel < 0 || p->param->errorLevel > 8)									//������ȼ�������Χ��ͳͳ����0
        p->param->errorLevel = 0;
    A = ERROR_LEVEL[p->param->errorLevel];														//ȡ����Ӧ�ľ����
    Alength = 2 << p->param->errorLevel;														//����2��(lenCodewords+1)�η����������
    E = p->param->codewords + dest;																//�ƶ�������β��
    memset(E, 0, Alength * sizeof(int));
    lastE = Alength - 1;
	/*******************************************************
	*�������
	********************************************************/
    for (k = 0; k < p->param->lenCodewords; ++k) {
        t1 = p->param->codewords[k] + E[0];														//t1=(di+ck-1)mod929
        for (e = 0; e <= lastE; ++e) {
            t2 = (t1 * A[lastE - e]) % MOD;														//t2=(t1*aj)mod929
            t3 = MOD - t2;																		//t3=929-t2
            E[e] = ((e == lastE ? 0 : E[e + 1]) + t3) % MOD;									//cj=(cj-1+t3)mod929|c0=t3mod929
        }
    }
	/*******************************************************
	*�Դ�������
	********************************************************/
    for (j = 0; j < Alength; ++j)
        E[j] = (MOD - E[j]) % MOD;
}

/*************************************************************
*��õ�ǰ�ַ�����
**************************************************************/
static int getTextTypeAndValue(char* text, int size, int idx) {
    int c;
    char *ms, *ps;
    if (idx >= size)																			//��֤��ǰ�ַ����С���ַ���ĩβ���
        return 0;
    c = text[idx];
    if (c >= 'A' && c <= 'Z')																	//��д��ĸ
        return (ALPHA + c - 'A');
    if (c >= 'a' && c <= 'z')																	//Сд��ĸ
        return (LOWER + c - 'a');
    if (c == ' ')																				//�ո�
        return (ALPHA + LOWER + MIXED + SPACE);
    ms = strchr(MIXED_SET, c);																	//MIXED�ַ������޴���
    ps = strchr(PUNCTUATION_SET, c);															//PUNC�ַ������޴���
    if (!ms && !ps)																				//������ĸ�����֡�����,��Ϊ����
        return (ISBYTE + (c & 0xff));
    if (ms - MIXED_SET == ps - PUNCTUATION_SET)													//����MIXED�ַ�����PUNCTUATION�ַ�
        return (MIXED + PUNCTUATION + (ms - MIXED_SET));
    if (ms != NULL)																				//������MIXED�ַ�
        return (MIXED + (ms - MIXED_SET));
    return (PUNCTUATION + (ps - PUNCTUATION_SET));												//������PUNCTUATION�ַ�
}

/********************************************************
*״̬ת������
-->��ת��
->������

TC��ģʽת��
ALPHA->MIXED			ml
ALPHE->LOWER			ll
LOWER-->ALPHA			as
LOWER-->PUNCTUATION		ps
LOWER->MIXED			ml
PUNCTUATION->ALPHA		pal
MIXED->ALPHA			al
MIXED->LOWER			ll
MIXED->PUCTUATION		pl
MIXED-->PUNCTUATION		ps
ģʽת��
TC->BC					901(mod6!=0)/924(mod6==0)
TC->NC					902
TC-->BC					913
BC->TC					900
BC->NC					902
NC->TC					900
NC->BC					901/924
���ķ���				BYTESHIT
*********************************************************/
void textCompaction(pPdf417class p, int start, int length) {
    int dest[ABSOLUTE_MAX_TEXT_SIZE * 2];														//��Ž��
    char* text = p->param->text;																//�ַ�ԭʼ����
    int mode = ALPHA;																			//��ʼģʽΪ�ı�ģʽ�е�ALPHA��ģʽ
    int ptr = 0;																				//��ǰ�ַ���
    int fullBytes = 0;
    int v = 0;
    int k;
    int size;
    memset(dest, 0, sizeof(dest));																//��dest��������
    length += start;																			//�ַ���ĩβ��
    for (k = start; k < length; ++k) {															//��ƫ����start��ʼ
        v = getTextTypeAndValue(text, length, k);												//��õ�ǰ�ַ�����
        if ((v & mode) != 0) {																	//������ַ��뵱ǰģʽһ��
            dest[ptr++] = v & 0xff;																//����8λ���ַ�ASCII��ֵ����
            continue;
        }
        if ((v & ISBYTE) != 0) {																//���ĺ���
            if ((ptr & 1) != 0) {																//��ǰ������ֵ,��Ҫ���Ϊż��
                dest[ptr++] = (mode & PUNCTUATION) != 0 ? PAL : PS;								//���ΪPUNCTUATIONģʽ����ı�ģʽ
                mode = (mode & PUNCTUATION) != 0 ? ALPHA : mode;								//�ı䵱ǰģʽ
            }
            dest[ptr++] = BYTESHIFT;
            dest[ptr++] = v & 0xff;
            fullBytes += 2;
            continue;
        }
        switch (mode) {																			//��ǰģʽ�ж�
        case ALPHA:																				//ALPHAģʽ
            if ((v & LOWER) != 0) {																//��ǰ�ַ�ΪMIXEDģʽ
                dest[ptr++] = LL;																//ALPHA->LOWER ll
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = LOWER;																	//����LOWERģʽ
            }
            else if ((v & MIXED) != 0) {														//��ǰ�ַ�ΪMIXEDģʽ
                dest[ptr++] = ML;																//ALPHA->MIXED ml
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = MIXED;																	//����MIXEDģʽ
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = ML;																//�������ַ�����PUNCTUATION�ַ�
                dest[ptr++] = PL;																//ALPHA->MIXED,MIXED->PUNCTUATION
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = PUNCTUATION;																//����ģʽΪPUNCTUATION
            }
            else {
                dest[ptr++] = PS;																//������PUNCTUATIONģʽ,ģʽת��,PLPHA-->PUNCTUATION
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
            }
            break;
        case LOWER:																				//LOWERģʽ
            if ((v & ALPHA) != 0) {																//��ǰ�ַ�ΪALPHA�ַ�
                if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & ALPHA) != 0) {
                    dest[ptr++] = ML;															//�������ַ���ΪALPHA,LOWER->MIXED
                    dest[ptr++] = AL;															//MIXED->ALPHA
                    mode = ALPHA;																//����ģʽΪALPHA
                }
                else {
                    dest[ptr++] = AS;															//LOWER-->ALPHA
                }
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
            }
            else if ((v & MIXED) != 0) {														//��ǰ�ַ�ΪMIXED�ַ�
                dest[ptr++] = ML;																//LOWER->MIXED
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = MIXED;																	//����ģʽΪMIXED
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = ML;																//LOWER->MIXED
                dest[ptr++] = PL;																//MIXED->PUNCTUATION
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = PUNCTUATION;																//�ඩģʽΪPUNCTUATION
            }
            else {
                dest[ptr++] = PS;																//LOWER-->PUNCTUATION
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
            }
            break;
        case MIXED:																				//ģʽΪMIXED
            if ((v & LOWER) != 0) {																//��ǰ�ַ�ΪLOWER�ַ�
                dest[ptr++] = LL;																//MIXED->LOWER ll
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = LOWER;																	//����ģʽΪLOWER
            }
            else if ((v & ALPHA) != 0) {														//��ǰ�ַ�ΪALPHAģʽ
                dest[ptr++] = AL;																//MIXED->ALPHA al
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = ALPHA;																	//����ģʽΪALPHA
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = PL;																//MIXED->PUNCTUATION pl
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
                mode = PUNCTUATION;																//����ģʽΪPUNCTUATION
            }
            else {																				//ģʽת��
                dest[ptr++] = PS;																//MIXED-->PUNCTUATION ps
                dest[ptr++] = v & 0xff;															//��¼ASCII��ֵ
            }
            break;
        case PUNCTUATION:																		//PUNCTUATIONģʽ
            dest[ptr++] = PAL;																	//PUNCTUATION->ALPHA 
            mode = ALPHA;																		//����ģʽΪALPHA
            --k;																				//���±Ƚ�
            break;
        }
    }
    if ((ptr & 1) != 0)																			//������
        dest[ptr++] = PS;																		//β�����ps
    size = (ptr + fullBytes) / 2;																//�õ��ַ�����
    if (size + p->cwPtr > MAX_DATA_CODEWORDS) {
        p->param->error = PDF417_ERROR_TEXT_TOO_BIG;
        return;
    }
    length = ptr;																				
    ptr = 0;
    while (ptr < length) {																			//��������
        v = dest[ptr++];																			//��ǰ��λ����30ֱ��ʹ��
        if (v >= 30) {																				//��������
            p->param->codewords[p->cwPtr++] = v;
            p->param->codewords[p->cwPtr++] = dest[ptr++];
        }
        else																						//��ǰ��λС��30����30*h+l
            p->param->codewords[p->cwPtr++] = v * 30 + dest[ptr++];
    }
}

/**********************************************************************
*�ı���ѹ
***********************************************************************/
void textUncompress(pPdf417class p, int start, int length)
{
	int dest[ABSOLUTE_MAX_TEXT_SIZE * 2];															//��Ž��
    int *codewords = p->param->codewords;
    char* text = p->param->text;																	//��������ַ�����ַ�
    int mode = ALPHA;																				//��ʼģʽΪ�ı�ģʽ�е�ALPHA��ģʽ
    int ptr = 0;																					//��ǰ�ַ���
    int fullBytes = 0;
    int v = 0;
    int k;
	ptr = 0;
	k = start;
	length += start;
	while(k < length)
	{
		if(p->param->codewords[k] == BYTESHIFT)														//����Ǻ����ַ�
		{
			dest[ptr++] = p->param->codewords[k++];
			dest[ptr++] = p->param->codewords[k++];
			continue;
		}
		dest[ptr++] = p->param->codewords[k] / 30;													//���Ǻ��ֵ��������ȡ30��������������
		dest[ptr++] = p->param->codewords[k] % 30;
		k += 1;
	}

	if(dest[ptr - 1] == PS)																			//β�����PS��ԭ��Ϊ������
	{
		ptr--;
	}
	length = ptr;
	ptr = 0;
	
	while(ptr < length)
	{
		v = dest[ptr];
		if(v == BYTESHIFT)																			//���Ϊ����,�Ѿ�����
		{
			p->param->text[p->param->lenText++] = dest[ptr + 1] & 0xff;								//���ΪPUNCTUATIONģʽ����ı�ģʽ
			p->param->text[p->param->lenText++] = dest[ptr + 3] & 0xff;
			ptr += 4;
            mode = (mode & PUNCTUATION) != 0 ? ALPHA : mode;
			continue;
		}
		if(v == SPACE)																				//�ո�
		{
			p->param->text[p->param->lenText++] = ' ';
			ptr += 1;
			continue;
		}
		switch(mode)
		{
		case ALPHA:
			/************************************
			*ALPHA->MIXED				ml	����
			*ALPHE->LOWER				ll	����
			*************************************/
			if(v == LL)
			{
				p->param->text[p->param->lenText++] = dest[ptr + 1] + 'a';
				ptr += 2;
				mode = LOWER;
			}
			else if(v == ML)
			{
				if(dest[ptr + 1] == PL)
				{
					p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 2]];
					ptr += 3;
					mode = PUNCTUATION;																		//������PUNCTUATION
				}
				else
				{
					p->param->text[p->param->lenText++] = MIXED_SET[dest[ptr + 1]];							//������MIXED
					ptr += 2;
					mode = MIXED;
				}
			}
			else if(v == PS)
			{
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//ģʽת��
				ptr += 2;
			}
			else
			{
				p->param->text[p->param->lenText++] = dest[ptr] + 'A';
				ptr += 1;
			}
			break;
		case LOWER:
			/*************************************
			*LOWER-->ALPHA				as	ת��
			*LOWER-->PUNCTUATION		ps	����
			*LOWER->MIXED				ml	����
			**************************************/
			if(v == AS)
			{
				p->param->text[p->param->lenText++] = dest[ptr + 1] + 'A';
				ptr += 2;																					//ģʽת��
			}
			else if(v == PS)
			{
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//ģʽת��
				ptr += 2;
			}
			else if(v == ML)
			{
				if(dest[ptr + 1] == AL)
				{
					p->param->text[p->param->lenText++] = dest[ptr + 2] + 'A';								//����ALPHAģʽ
					ptr += 3;
					mode = ALPHA;
				}
				else if(dest[ptr + 1] == PL)
				{
					p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 2]];					//����PUNCTUATIONģʽ
					ptr += 3;
					mode = PUNCTUATION;
				}
				else
				{
					p->param->text[p->param->lenText++] = MIXED_SET[dest[ptr + 1]];							//������MIXED
					ptr += 2;
					mode = MIXED;
				}
			}
			else
			{
				p->param->text[p->param->lenText++] = dest[ptr] + 'a';
				ptr += 1;
			}
			break;
		case MIXED:
			/*************************************
			*MIXED->ALPHA				al	����
			*MIXED->LOWER				ll	����
			*MIXED->PUCTUATION			pl	����
			*MIXED-->PUNCTUATION		ps	ת��
			***************************************/
			if(v == LL)
			{
				p->param->text[p->param->lenText++] = dest[ptr + 1] + 'a';
				ptr += 2;
				mode = LOWER;
			}
				else if(v == AL)
			{
				p->param->text[p->param->lenText++] = dest[ptr + 1] + 'A';
				ptr += 2;
				mode = ALPHA;
			}
			else if(v == PL)
			{
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//����PUNCTUATIONģʽ
				ptr += 2;
				mode = PUNCTUATION;
			}
			else if(v == PS)
			{	
				if(dest[ptr + 1] != BYTESHIFT)
				{
					p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];					//ת��PUNCTUATIONģʽ
					ptr += 2;
				}
				else
				{
					ptr += 1;
				}
			}
			else
			{
				p->param->text[p->param->lenText++] = MIXED_SET[dest[ptr]];
				ptr += 1;
			}
			break;
		case PUNCTUATION:
			/*************************************
			*PUNCTUATION->ALPHA			pal	����
			**************************************/
			mode = ALPHA;
			ptr += 1;
			break;
		}
	}
}

/***********************************************************************
*��ֵת��(10->900)-�����㷨
************************************************************************/
void basicNumberCompaction(pPdf417class p, int start, int length) {
    char* text = p->param->text;
    int* ret = p->param->codewords + p->cwPtr;
    int retLast = length / 3;																			//�µĳ���ΪINT[LENGTH/3]+1
    int ni, k;
    p->cwPtr += retLast + 1;
    memset(ret, 0, (retLast + 1) * sizeof(int));
    ret[retLast] = 1;																					//ǰ����Ϊ1
    length += start;
    for (ni = start; ni < length; ++ni) {																//(ai*10^i...a0)*10=(bi900^i...b0)*10
        // multiply by 10
        for (k = retLast; k >= 0; --k)
            ret[k] *= 10;
        // add the digit
        ret[retLast] += text[ni] - '0';
        // propagate carry
        for (k = retLast; k > 0; --k) {
            ret[k - 1] += ret[k] / 900;
            ret[k] %= 900;
        }
    }
}

/***********************************************************************
*��ֵת��(900->10)-�����㷨 �޸����ʱ��:2015/4/2
************************************************************************/
void basicNumberUncompress(pPdf417class p, int start, int length) {
    char* text = p->param->text;
    int* ret = p->param->codewords + p->cwPtr;
	int* res;
    int retLast = length * 3;																		//����Ϊ(length - 1) * 3 - 1
    int ni, k;
	res = (int *)malloc((retLast + 1) * sizeof(int));
	memset(res, 0, (retLast + 1) * sizeof(int));
    length += start;
    for (ni = start; ni < length; ++ni) {																//(ai*10^i...a0)*10=(bi900^i...b0)*10
        // multiply by 900
        for (k = retLast; k >= 0; --k)
            res[k] *= 900;
        // add the digit
        res[retLast] += p->param->codewords[ni];
        // propagate carry
        for (k = retLast; k > 0; --k) {
            res[k - 1] += res[k] / 10;
            res[k] %= 10;
        } 
    }

	for(ni = 0; ni <= retLast && res[ni] != 1; ni++);													//Ѱ��ǰ����1

	for(ni += 1; ni <= retLast; ni++)																	//ת��Ϊ�ַ�ֵ
	{
		p->param->text[p->param->lenText++] = res[ni] + '0';
	}
	
}	

/*********************************************************************
*����ת������ 44���ֽ�->15������
**********************************************************************/
void numberCompaction(pPdf417class p, int start, int length) {
    int full = (length / 44) * 15;																		//��������
    int size = length % 44;																				//���һ�鳤��
    int k;
    if (size == 0)																						//��������Ϊ44��������
        size = full;
    else
        size = full + size / 3 + 1;
    if (size + p->cwPtr > MAX_DATA_CODEWORDS) {
        p->param->error = PDF417_ERROR_TEXT_TOO_BIG;
        return;
    }
    length += start;
    for (k = start; k < length; k += 44) {
        size = length - k < 44 ? length - k : 44;
        basicNumberCompaction(p, k, size);
    }
}

/*********************************************************************
*����ת������ 15������->44���ֽ�
**********************************************************************/
void numberUncompress(pPdf417class p, int start, int length)
{
	int full = (length / 15) * 44;
	int size = length % 15;
	int k;
	if(size == 0)
		size = full;
	else
		size = size * 3 + full + 1;
	length += start;
	for (k = start; k < length; k += 15) {
        size = length - k < 15 ? length - k : 15;
        basicNumberUncompress(p, k, size);
    }
}

/********************************************************
*��ֵת��(256->900)
*********************************************************/
void byteCompaction6(pPdf417class p, int start) {
    int length = 6;
    char* text = p->param->text;
    int* ret = p->param->codewords + p->cwPtr;
    int retLast = 4;
    int ni, k;
    p->cwPtr += retLast + 1;
    memset(ret, 0, (retLast + 1) * sizeof(int));
    length += start;
    for (ni = start; ni < length; ++ni) {
        // multiply by 256
        for (k = retLast; k >= 0; --k)
            ret[k] *= 256;
        // add the digit
        ret[retLast] += (int)text[ni] & 0xff;
        // propagate carry
        for (k = retLast; k > 0; --k) {
            ret[k - 1] += ret[k] / 900;
            ret[k] %= 900;
        }
    }
}

/********************************************************
*��ֵת��(900->256)
*********************************************************/
void byteUncompress6(pPdf417class p, int start) {
    int length = 5;
    int* codewords = p->param->codewords;
    int* ret = NULL;
	//char* temp = p->param->text;
    int retLast = 5;
    int ni, k;

	/**************************************************************
	*malloc�����ڴ�й¶
	***************************************************************/
	/****************************************************************
	if(!p->param->text)
	{
		p->param->text = malloc(length);
		memset(p->param->text, '\0', p->param->lenText + length);
	}
	else																											//�����ֽڿռ�
	{
		if(temp)
		{
			free(temp);
			temp = NULL;
		}
		temp = malloc(p->param->lenText);
		memset(temp, '\0', p->param->lenText);
		memcpy(temp, p->param->text, p->param->lenText);															//����ԭ���ַ���
		free(p->param->text);
		p->param->text = NULL;
		p->param->text = malloc(p->param->lenText + length);														//�����ı��ڴ泤��
		memset(p->param->text, '\0', p->param->lenText + length);
		memcpy(p->param->text, temp, p->param->lenText);
	}
	********************************************************************/

	ret = (int *)malloc((retLast + 1) * sizeof(int));
	memset(ret, 0, (retLast + 1) * sizeof(int));

    length += start;
    for (ni = start; ni < length; ++ni) {
        // multiply by 900
        for (k = retLast; k >= 0; --k)
            ret[k] *= 900;
        // add the digit
        ret[retLast] += codewords[ni];
        // propagate carry
        for (k = retLast; k > 0; --k) {
            ret[k - 1] += ret[k] / 256;
            ret[k] %= 256;
        }
    }
	for(ni = 0; ni <= retLast; ni++)																				//д���ַ�
	{
		p->param->text[p->param->lenText++] = (ret[ni] & 0xff);
	}																				//�ı���������6
}

/****************************************************************
*BC-�ֽ�ѹ�� 6���ֽ�->5������
	[0]	0x00000000
	[1]	0x0000039c
	[2]	0x00000158
	[3]	0x00000383
	[4]	0x00000206
	[5]	0x00000060

	[0x1]	0x00000385
	[0x2]	0x000000cd
	[0x3]	0x000000de
	[0x4]	0x00000031
	[0x5]	0x000000cd

*****************************************************************/
void byteCompaction(pPdf417class p, int start, int length) {
    int k, j;
    int size = (length / 6) * 5 + (length % 6);
    if (size + p->cwPtr > MAX_DATA_CODEWORDS) {																		//���ó�������ֽ���
        p->param->error = PDF417_ERROR_TEXT_TOO_BIG;
        return;
    }
    length += start;
    for (k = start; k < length; k += 6) {
        size = length - k < 6 ? length - k : 6;																		//��2015/3/31�޸�:ԭʼ����size = length - k < 44 ? length - k : 6;	
        if (size < 6) {
            for (j = 0; j < size; ++j)
                p->param->codewords[p->cwPtr++] = (int)p->param->text[k + j] & 0xff;
        }
        else {	
            byteCompaction6(p, k);
        }
    }
}

/****************************************************************
*BC-�ֽڽ�ѹ 5������->6���ֽ�
	[0x0]	0x00000008
	[0x1]	0x00000385
	[0x2]	0x00000158
	[0x3]	0x00000383
	[0x4]	0x00000206
	[0x5]	0x0000002f

	[0x0]	0x00000007
	[0x1]	0x00000385
	[0x2]	0x000000cd
	[0x3]	0x000000de
	[0x4]	0x00000031
	[0x5]	0x000000cd

*****************************************************************/
void byteUncompress(pPdf417class p, int start, int length, char mode)
{
	int k, j, i, r;
	int size;
	length += start;
	for (k = start, r = 0; k < length; k += 5) {
        size = length - k < 5 ? length - k : 5;																		//��2015/3/31�޸�:ԭʼ����size = length - k < 44 ? length - k : 6;	
        if (size < 5) {
			i = start + (r / 6) * 5;
            for (j = 0; j < size; ++j)
               p->param->text[p->param->lenText++] = p->param->codewords[i + j] & 0xff;
        }
        else {
			if(length - k == 5)
			{
				if(mode == 'b')
				{
					i = start + (r / 6) * 5;
					for (j = 0; j < 5; ++j)
						p->param->text[p->param->lenText++] = p->param->codewords[i + j] & 0xff;
				}
				else
				{
					byteUncompress6(p, k);
					r += 6;
				}
			}
			else
			{
				byteUncompress6(p, k);
				r += 6;
			}
        }
    }
}

/************************************************************
*�ַ����ض�
*************************************************************/
void breakString(pPdf417class p, pArrayList list) {
    char* text = p->param->text;
    int textLength = p->param->lenText;
    int lastP = 0;
    int startN = 0;
    int nd = 0;
    char c = 0;
    int k, ptrS, lastTxt, j, txt;
    pListElement v;
    pListElement vp;
    pListElement vn;
    list->size = 0;
    for (k = 0; k < textLength; ++k) {
        c = text[k];
        if (c >= '0' && c <= '9') {																			//��¼�����ַ�
            if (nd == 0)
                startN = k;
            ++nd;
            continue;
        }
        if (nd >= 13) {																						//���������ָ�������13
            if (lastP != startN) {
                c = text[lastP];
                ptrS = lastP;
                lastTxt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
                for (j = lastP; j < startN; ++j) {															//ǰ�������յ㵽��ǰ�������
                    c = text[j];
                    txt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
                    if (txt != lastTxt) {
                        listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, j);								//�Ǻ�����ΪBYTE,������ΪTEXT
                        lastP = j;
                        lastTxt = txt;
                    }
                }
                listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, startN);									//��TEXT����BYTE
            }
            listAdd(list, 'N', startN, k);																	//�������������յ�
            lastP = k;																						//���¶ϵ�
        }
        nd = 0;
    }
    if (nd < 13)
        startN = textLength;
    if (lastP != startN) {
        c = text[lastP];
        ptrS = lastP;
        lastTxt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
        for (j = lastP; j < startN; ++j) {
            c = text[j];
            txt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
            if (txt != lastTxt) {
                listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, j);
                lastP = j;
                lastTxt = txt;
            }
        }
        listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, startN);
    }
    if (nd >= 13)
        listAdd(list, 'N', startN, textLength);
    //optimize
    //merge short binary
    for (k = 0; k < list->size; ++k) {
        v = listGet(list, k);
        vp = listGet(list, k - 1);
        vn = listGet(list, k + 1);;
        if (checkElementType(v, 'B') && getElementLength(v) == 1) {													//��ǰ��������ΪBYTE�ҳ���Ϊ1
            if (checkElementType(vp, 'T') && checkElementType(vn, 'T')												//����к��Ҷ��о�ΪTEXT
                && getElementLength(vp) + getElementLength(vn) >= 3) {
                vp->end = vn->end;
                listRemove(list, k);
                listRemove(list, k);
                k = -1;
                continue;
            }
        }
    }
    //merge text sections
    for (k = 0; k < list->size; ++k) {
        v = listGet(list, k);
        vp = listGet(list, k - 1);
        vn = listGet(list, k + 1);
        if (checkElementType(v, 'T') && getElementLength(v) >= 5) {												//��ǰ����ΪTEXT�ҳ��ȴ��ڵ���5
            int redo = 0;
            if ((checkElementType(vp, 'B') && getElementLength(vp) == 1) || checkElementType(vp, 'T')) {		//���������ΪBYTE�ҳ���С��5�������������ΪTEXT
                redo = 1;
                v->start = vp->start;
                listRemove(list, k - 1);
                --k;
            }
            if ((checkElementType(vn, 'B') && getElementLength(vn) == 1) || checkElementType(vn, 'T')) {		//�Ҷ�������ΪBYTE�ҳ���С��5�������������ΪTEXT
                redo = 1;
                v->end = vn->end;
                listRemove(list, k + 1);
            }
            if (redo) {
                k = -1;
                continue;
            }
        }
    }
    //merge binary sections
    for (k = 0; k < list->size; ++k) {
        v = listGet(list, k);
        vp = listGet(list, k - 1);
        vn = listGet(list, k + 1);
        if (checkElementType(v, 'B')) {
            int redo = 0;
            if ((checkElementType(vp, 'T') && getElementLength(vp) < 5) || checkElementType(vp, 'B')) {			//���������ΪTEXT�ҳ���С��5�������������ΪBYTE
                redo = 1;
                v->start = vp->start;
                listRemove(list, k - 1);
                --k;
            }
            if ((checkElementType(vn, 'T') && getElementLength(vn) < 5) || checkElementType(vn, 'B')) {			//�Ҷ�������ΪTEXT�ҳ���С��5�����Ҷ�������ΪBYTE
                redo = 1;
                v->end = vn->end;
                listRemove(list, k + 1);
            }
            if (redo) {
                k = -1;
                continue;
            }
        }
    }
    // check if all numbers
    if (list->size == 1 && (v = listGet(list, 0))->type == 'T' && getElementLength(v) >= 8) {
        for (k = v->start; k < v->end; ++k) {
            c = text[k];
            if (c < '0' || c > '9')
                break;
        }
        if (k == v->end)
            v->type = 'N';
    }
}

/****************************************************
*���Ԫ������
*****************************************************/
char GetElementType(int mode)
{
	switch(mode)
	{
	case TEXT_MODE:
		return 'T';
	case NUMERIC_MODE:
		return 'N';
	case BYTE_MODE:
		return 'b';
	case BYTE_MODE_6:
		return 'B';
	}
	return '\0';
}

/************************************************************
*�ַ���ƴ�� ��δ�����Ż�����
*************************************************************/
void connectString(pPdf417class p, pArrayList list)
{
    int* codewords = p->param->codewords;
    int codeLength = p->param->lenCodewords;
    int lastP = 0, lastMode = 0;
    int startN = 0, endN = 0;
    int nd = 0;
    char c = 0, type;
    int k;
	k = 1;																													//codeword���Ϊ1
	while(k < codeLength)
	{
		type = GetElementType(p->param->codewords[k]);
		if(type != '\0' || (type == '\0' && k == 1))																			//��ʼģʽ
		{
			if(k == 1 && type == '\0')
			{
				type = 'T';
				startN = k;
			}
			else
				startN = (++k);
			for(endN = startN + 1; endN < codeLength - 1
				&& (GetElementType(p->param->codewords[endN]) == '\0'); endN++);
			if(endN - startN >= 1)									//�з���ģʽת�����ߵִ�ĩβ?
				listAdd(list, type, startN, endN);
			k = endN;
			continue;
		}
	}
	if(p->param->error)
		return ;
}

/********************************************************
*��������ת��
********************************************************/
void assemble(pPdf417class p, pArrayList list) {
    int k;
    if (list->size == 0)
        return;
    p->cwPtr = 1;																										//codeword���Ϊ1
    for (k = 0; k < list->size; ++k) {
        pListElement v = listGet(list, k);
        switch (v->type) {
        case 'T':
            if (k != 0)
                p->param->codewords[p->cwPtr++] = TEXT_MODE;
            textCompaction(p, v->start, v->end - v->start);
            break;
        case 'N':
            p->param->codewords[p->cwPtr++] = NUMERIC_MODE;
            numberCompaction(p, v->start, v->end - v->start);
            break;
        case 'B':
            p->param->codewords[p->cwPtr++] = (v->end - v->start) % 6 ? BYTE_MODE : BYTE_MODE_6;
            byteCompaction(p, v->start, v->end - v->start);
            break;
        }
        if (p->param->error)
            return;
    }
}
	/*
	[1]	902
	[2]	190
	[3]	441
	[4]	566
	[5]	613
	[6]	798
	[7]	85
	[8]	289*/

/***************************************************************
*��������������ת�� ��δ�����Ż�����
****************************************************************/
void unassemble(pPdf417class p, pArrayList list)
{
    int k;
    if (list->size == 0)
        return;
	for(k = 0; k < list->size; k++)
	{
		pListElement v = listGet(list, k);
        switch (v->type) {
        case 'T':
            textUncompress(p, v->start, v->end - v->start);
            break;
        case 'N':
            numberUncompress(p, v->start, v->end - v->start);
            break;
        case 'b':																								//������6
            byteUncompress(p, v->start, v->end - v->start, 'b');
			break;
		case 'B':																								//����6
			byteUncompress(p, v->start, v->end - v->start, 'B');
            break;
        }
        if (p->param->error)
            return;
	}
}

/********************************************************
*ԭʼ�ַ�Ԫ���б�
********************************************************/
static void dumpList(pPdf417class p, pArrayList list) {
    int k;
    if (list->size == 0)
        return;
    for (k = 0; k < list->size; ++k) {
        pListElement v = listGet(list, k);
        printf("%c%.*s\n", v->type, v->end - v->start, p->param->text + v->start);
    }
}

/********************************************************
*����ַ�Ԫ���б�
********************************************************/
static void bitsList(pPdf417class p, pArrayList list) {
    int k, i;
    if (list->size == 0)
        return;
    for (k = 0; k < list->size; ++k) {
        pListElement v = listGet(list, k);
		printf("%c ", v->type);
		for(i = v->start; i < v->end; i++)
		{
			printf("%2x ", p->param->codewords[i]);
		}
		printf("\n");
    }
}

/*****************************************************
*������ƽ��
******************************************************/
static int getMaxSquare(pPdf417param p) {
    if (p->codeColumns > 21) {
        p->codeColumns = 29;
        p->codeRows = 32;
    }
    else {
        p->codeColumns = 16;
        p->codeRows = 58;
    }
    return MAX_DATA_CODEWORDS + 2;
}

/******************************************************
*�������� ��������ѡ��
*******************************************************/
void paintCode(pPdf417param p) {
    pdf417class pp;
    arrayList list;
    int maxErr, fixedColumn, lenErr, tot, skipRowColAdjust, pad;
    pp.param = p;
    p->error = 0;
    if (p->options & PDF417_USE_RAW_CODEWORDS) {
        if (p->lenCodewords > MAX_DATA_CODEWORDS || p->lenCodewords < 1 || p->lenCodewords != p->codewords[0]) {
            p->error = PDF417_ERROR_INVALID_PARAMS;
            return;
        }
    }
    else {
        if (p->lenText < 0)
            p->lenText = strlen(p->text);
        if (p->lenText > ABSOLUTE_MAX_TEXT_SIZE) {
            p->error = PDF417_ERROR_TEXT_TOO_BIG;
            return;
        }
        listInit(&list);
        breakString(&pp, &list);
        dumpList(&pp, &list);
        assemble(&pp, &list);
        listFree(&list);
		/****************************************************
		*�鿴���ݵı���
		*****************************************************/
		/*for(i = 0; i < pp.cwPtr; i++)
		{
			printf("%2x ", pp.param->codewords[i]);
		}*/
        if (p->error)
            return;
        p->codewords[0] = p->lenCodewords = pp.cwPtr;																//Ԥ����һλΪ���ֳ���
    }
    maxErr = maxPossibleErrorLevel(MAX_DATA_CODEWORDS + 2 - p->lenCodewords);
    if (!(p->options & PDF417_USE_ERROR_LEVEL)) {
        if (p->lenCodewords < 41)
            p->errorLevel = 2;
        else if (p->lenCodewords < 161)
            p->errorLevel = 3;
        else if (p->lenCodewords < 321)
            p->errorLevel = 4;
        else
            p->errorLevel = 5;
    }
	/****************************************
	*�жϴ����Ƿ���Ϲ淶
	*****************************************/
    if (p->errorLevel < 0)
        p->errorLevel = 0;
    else if (p->errorLevel > maxErr)
        p->errorLevel = maxErr;
    if (p->codeColumns < 1)
        p->codeColumns = 1;
	/****************************************
	*�ж������Ƿ���Ϲ淶
	*****************************************/
    else if (p->codeColumns > 30)
        p->codeColumns = 30;
    if (p->codeRows < 3)
        p->codeRows = 3;
    else if (p->codeRows > 90)
        p->codeRows = 90;
    lenErr = 2 << p->errorLevel;
    fixedColumn = !(p->options & PDF417_FIXED_ROWS);
    skipRowColAdjust = 0;
    tot = p->lenCodewords + lenErr;
    if (p->options & PDF417_FIXED_RECTANGLE) {
        tot = p->codeColumns * p->codeRows;
        if (tot > MAX_DATA_CODEWORDS + 2) {
            tot = getMaxSquare(p);
        }
        if (tot < p->lenCodewords + lenErr)
            tot = p->lenCodewords + lenErr;
        else
            skipRowColAdjust = 1;
    }
    else if (!(p->options & (PDF417_FIXED_COLUMNS | PDF417_FIXED_ROWS))) {
        double c, b;
        fixedColumn = 1;
        if (p->aspectRatio < 0.001)
            p->aspectRatio = 0.001f;
        else if (p->aspectRatio > 1000)
            p->aspectRatio = 1000;
        b = 73 * p->aspectRatio - 4;
        c = (-b + sqrt(b * b + 4 * 17 * p->aspectRatio * (p->lenCodewords + lenErr) * p->yHeight)) / (2 * 17 * p->aspectRatio);
        p->codeColumns = (int)(c + 0.5);
        if (p->codeColumns < 1)
            p->codeColumns = 1;
        else if (p->codeColumns > 30)
            p->codeColumns = 30;
    }
    if (!skipRowColAdjust) {
        if (fixedColumn) {
            p->codeRows = (tot - 1) / p->codeColumns + 1;
            if (p->codeRows < 3)
                p->codeRows = 3;
            else if (p->codeRows > 90) {
                p->codeRows = 90;
                p->codeColumns = (tot - 1) / 90 + 1;
            }
        }
        else {
            p->codeColumns = (tot - 1) / p->codeRows + 1;
            if (p->codeColumns > 30) {
                p->codeColumns = 30;
                p->codeRows = (tot - 1) / 30 + 1;
            }
        }
        tot = p->codeRows * p->codeColumns;
    }
    if (tot > MAX_DATA_CODEWORDS + 2) {
        tot = getMaxSquare(p);
    }
    p->errorLevel = maxPossibleErrorLevel(tot - p->lenCodewords);
    lenErr = 2 << p->errorLevel;
    pad = tot - lenErr - p->lenCodewords;
    pp.cwPtr = p->lenCodewords;
    while (pad--)
        p->codewords[pp.cwPtr++] = TEXT_MODE;
    p->codewords[0] = p->lenCodewords = pp.cwPtr;
    calculateErrorCorrection(&pp, pp.param->lenCodewords);
    pp.param->lenCodewords = tot;
    outPaintCode(&pp);
}

/*****************************************************************
*��λͼ�õ�����ֽ�
******************************************************************/
int GetBitsFromBMP(pPdf417class p, char *pfileName)
{
	HANDLE hfile;
	BITMAPFILEHEADER *pbmfh;
	BITMAPINFOHEADER *pbmifh;
	DWORD dwFileSize, dwHighSize, dwBytesRead;
	BOOL bSuccess;
	char *pbits, *temp = NULL;
	int bmpWidth, bmpHeight, bitCounts;
	int column;
	int firstPos = 0, lastPos = 0, curPos = 0, curCode = 0;																//��ʼλ��,��ֹλ��,��ǰ�ֽ�λ��,��ǰ�������
	int i = 0, j = 0, k = 0, oldsize = 0, newsize = 0;
	BOOL haveFoundStartLine = FALSE, 
		haveFoundStartCol = FALSE, 
		haveFoundEndCol = FALSE, 
		haveFoundEndLine = FALSE;
	int startLine = 0, endLine = 0, startCol = 0, endCol = 0;
	if(pfileName == NULL)																								//�ļ���Ϊ��
	{
		//printf("file name is null\n");
		return 0;
	}
	hfile = CreateFile(pfileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,												//����λͼ�ļ���Ϣ
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hfile == INVALID_HANDLE_VALUE)																					//��λͼʧ��
	{
		//printf("open file fail\n");
		return 0;
	}
	dwFileSize = GetFileSize(hfile, &dwHighSize);
	if(dwHighSize)																										//�ļ�����2G
	{
		//printf("can't deal with this size\n");
		CloseHandle(hfile);
		return 0;
	}
	pbmfh = (BITMAPFILEHEADER *)malloc(dwFileSize);
	if(pbmfh == NULL)																									//�ڴ����ʧ��
	{
		//printf("malloc memory fails\n");
		CloseHandle(hfile);
		return 0;
	}
	bSuccess = ReadFile(hfile, pbmfh, dwFileSize, &dwBytesRead, NULL);
	if(bSuccess == FALSE)
	{
		//printf("read data from file fails\n");
		CloseHandle(hfile);
		return 0;
	}
	pbmifh = (BITMAPINFOHEADER *)(pbmfh + 1);
	bmpWidth = pbmifh->biWidth;
	bmpHeight = pbmifh->biHeight;
	bitCounts = pbmifh->biBitCount;
	pbits = (char *)(pbmfh) + pbmfh->bfOffBits;
	if(hfile)
	{
		CloseHandle(hfile);
	}
	if(bitCounts == 1)																									//1λ��
	{
		column = ((bmpWidth * bitCounts + 31) & ~31) >> 3;
		
		for(i = 0; i < bmpHeight && !haveFoundEndLine; i++)																//��Ѱ��ʼ��,��ֹ��,��ʼ��,��ֹ��
		{
			for(j = 0; j < column && !haveFoundStartLine; j++)															//ɨ����ֱ����ʼ��
			{
				if((pbits[i * column + j] & 0xFF) != 0xFF)
				{
					haveFoundStartLine = TRUE;
					startLine = i;
				}
			}
			for(j = 0; j < column && (!haveFoundEndCol || !haveFoundStartCol); j++)																//����ʼ�п�ʼ
			{
				for(k = 0; k < 8 && !haveFoundStartCol; k++)
				{
					int temp = 0x01;
					if((((pbits[i * column + j] & 0xFF) & (temp << k)) >> k) == 0)									//��ȡ���ص�
					{
						haveFoundStartCol = TRUE;
						startCol = j;
						firstPos = i * column + j + k;																	//��һ��Ϊ�ڵ�����
						p->bitPtr = firstPos;
					}
				}
				
				if(haveFoundStartCol && (pbits[i * column + j] & 0xFF) == 0xFF)											//���һ��+1
				{
					haveFoundEndCol = TRUE;
					endCol = j - 1;
					p->param->codeColumns = endCol - startCol + 1;															//����е��ֽڳ���
					p->param->bitColumns = (endCol - startCol + 1) * 8;	
				}
			}

			for(j = startCol; j <= endCol && (pbits[i * column + j] & 0xFF) == 0xFF; j++);
			if(j == endCol + 1)																							//���һ�м�1
			{
				haveFoundEndLine = TRUE;
				endLine = i - 1;																						//ȷ��β�к�
				p->param->codeRows = endLine - startLine + 1;															//����и�
				p->param->bitColumns = p->param->codeColumns * 8;
				p->param->lenBits = p->param->codeColumns * p->param->codeRows;
				p->param->lenCodewords = p->param->codeRows * (p->param->codeColumns / 8 + 1);
				continue;
			}
			
			if(temp)
			{
				free(temp);
				temp = NULL;
			}
			if(p->param->lenBits)																						//���ϸ���outBits���ֽڳ�
			{
				oldsize = p->param->lenBits;
				temp = (char *)malloc(oldsize);
				memcpy(temp, p->param->outBits, p->param->lenBits);
				free(p->param->outBits);
				newsize = p->param->lenBits + endCol - startCol + 1;
				p->param->outBits = (char *)malloc(newsize);
				memcpy(p->param->outBits, temp, oldsize);
			}
			else
			{
				p->param->outBits = (char *)malloc(endCol - startCol + 1);
			}
			for(j = startCol; j <= endCol; j++)																			//������д�ص�outBits��
			{
				p->param->outBits[p->param->lenBits++] = ~pbits[i * column + j];									//������1Ϊ�� 0Ϊ��,����ȡ��
				//printf("%2x ", (255 - pbits[i * column + j]) & 0xFF);
			}
			//printf("\n");
		}
	}
	else if(bitCounts == 4)																								//4λ��
	{
	}
	else if(bitCounts == 8)																								//8λ��
	{
	}
	else if(bitCounts == 24)																							//24λ��
	{
	}
	else if(bitCounts == 32)
	{
		//�ֽ��п�
		column = ((bmpWidth * bitCounts + 31) & ~31) >> 3;
		
		for(i = 0; i < bmpHeight && !haveFoundEndLine; i++)																//��Ѱ��ʼ��,��ֹ��,��ʼ��,��ֹ��
		{
			for(j = 0; j < column && !haveFoundStartLine; j += 4)															//ɨ����ֱ����ʼ��
			{
				if(pbits[i * column + j + 1] != char(0XFF)
					|| pbits[i * column + j + 2] != char(0XFF)
					|| pbits[i * column + j + 3] != char(0XFF))
				{
					haveFoundStartLine = TRUE;
					startLine = i;
				}
			}
			//��������ѡȡ
			for(j = 0; j < column && (!haveFoundEndCol || !haveFoundStartCol); j += 4)																//����ʼ�п�ʼ
			{
				if(pbits[i * column + j + 1] == 0
					&& pbits[i * column + j + 2] == 0
					&& pbits[i * column + j + 3] == 0)									//��ȡ���ص�
				{
					haveFoundStartCol = TRUE;
					startCol = j;
					firstPos = i * column / 32 + j;																	//��һ��Ϊ�ڵ�����
					p->bitPtr = firstPos;
					break;
				}
			}
			
			//��������ѡȡ
			for(j = column; j >= 0 && !haveFoundEndCol; j -= 4)																//����ʼ�п�ʼ
			{
				if(pbits[i * column + j + 1] == 0
					&& pbits[i * column + j + 2] == 0
					&& pbits[i * column + j + 3] == 0)									//��ȡ���ص�
				{
					haveFoundEndCol = TRUE;
					endCol = j;
					p->param->codeColumns = (endCol - startCol + 1) / 32;												//����е��ֽڳ���
					p->param->bitColumns = p->param->codeColumns * 8;
					break;
				}
			}

			for(j = startCol; j <= endCol
				&& pbits[i * column + j + 1] == char(0xFF)
				&& pbits[i * column + j + 2] == char(0xFF)
				&& pbits[i * column + j + 3] == char(0xFF); j += 4);
			if(j > endCol)																						//���һ�м�1
			{
				haveFoundEndLine = TRUE;
				endLine = i - 1;																						//ȷ��β�к�
				p->param->codeRows = endLine - startLine + 1;															//����и�
				p->param->bitColumns = p->param->codeColumns * 8;
				p->param->lenBits = p->param->codeColumns * p->param->codeRows;
				p->param->lenCodewords = p->param->codeRows * (p->param->codeColumns / 8 + 1);
				continue;
			}
			else if(i == bmpHeight - 1 && !haveFoundEndLine)
			{
				haveFoundEndLine = TRUE;
				endLine = i;																						//ȷ��β�к�
				p->param->codeRows = endLine - startLine + 1;															//����и�
				p->param->bitColumns = p->param->codeColumns * 8;
				p->param->lenBits = p->param->codeColumns * p->param->codeRows;
				p->param->lenCodewords = p->param->codeRows * (p->param->codeColumns / 8 + 1);
				continue;
			}
			
			if(temp)
			{
				free(temp);
				temp = NULL;
			}
			if(p->param->lenBits)																						//���ϸ���outBits���ֽڳ�
			{
				oldsize = p->param->lenBits;
				temp = (char *)malloc(oldsize);
				memcpy(temp, p->param->outBits, p->param->lenBits);
				free(p->param->outBits);
				newsize = p->param->lenBits + (endCol - startCol) / 32 + 1;
				p->param->outBits = (char *)malloc(newsize);
				memcpy(p->param->outBits, temp, oldsize);
			}
			else
			{
				p->param->outBits = (char *)malloc((endCol - startCol) / 32 + 1);
			}
			for(j = startCol; j < endCol; j += 32, p->param->lenBits++)																			//������д�ص�outBits��
			{
				int k = 0;
				p->param->outBits[p->param->lenBits] = 0;
				for(k = 0; k < 8; k++)
				{
					//ÿһ������1λ 
					p->param->outBits[p->param->lenBits] |= (((((unsigned char)(pbits[i * column + j + k * 4 + 1]) + (unsigned char)(pbits[i * column + j + k * 4 + 2]) + (unsigned char)(pbits[i * column + j + k * 4 + 3])) / 765))  << (8 - (k + 1)));									//������1Ϊ�� 0Ϊ��,����ȡ��
					//printf("%2x ", (255 - pbits[i * column + j]) & 0xFF);
				}
				p->param->outBits[p->param->lenBits] = ~p->param->outBits[p->param->lenBits];
			}
			//printf("\n");
		}
	}
	else
	{
		//printf("unkonwn format\n");
		if(hfile)
		{
			CloseHandle(hfile);
		}
		return 0;
	}
	return 1; 
}

/********************************************************************
*�ָ��ַ��� δ�����Ż�
********************************************************************/
void recoverString(pPdf417class p, char *pfileName, pArrayList list)
{
	GetBitsFromBMP(p, pfileName);																						//���������ݻָ������
	inPaintCode(p);																										//������
	connectString(p, list);																								//ƴ���ַ���
	bitsList(p, list);
	unassemble(p, list);																								//�ָ��ַ���
}


/*****************************************************************
*������д������λͼ ����1��4��8��24λλͼ
******************************************************************/
int PaintBMP(pdf417param p, char *pfileName)
{
	BOOL bSuccess;
	DWORD dwFileSize, dwHighSize, dwBytesRead, dwBytesWritten;
	HANDLE hFile;
	BITMAPFILEHEADER *pbmfh;
	BITMAPINFO *pbmf;
	BITMAPINFOHEADER bmih;
	char *pbits;
	int i = 0, j = 0, k = 0, bitCount = 0;
	int bmp_width, bmp_height;
	char temp = 0;
	int column, cur_col, cur_color;
	hFile = CreateFile(pfileName, GENERIC_READ, 0, NULL,																//����λͼ�ļ���Ϣ
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		//printf("create file failed!\n");
		hFile = CreateFile(pfileName, GENERIC_READ, 0, NULL,																//����λͼ�ļ���Ϣ
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			return 0;
	}
	dwFileSize = GetFileSize(hFile, &dwHighSize);																		//����ļ��ֽڴ�С
	if(dwHighSize)
	{
		CloseHandle(hFile);
		printf("fail to deal with file size!\n");
		return 0;

	}
	pbmfh = (BITMAPFILEHEADER *)malloc(dwFileSize);
	if(!pbmfh)
	{
		printf("fail to malloc memeory\n");
		return 0;
	}
	bSuccess = ReadFile(hFile, pbmfh, dwFileSize, &dwBytesRead, NULL);
	if(!bSuccess)
	{
		free(pbmfh);
		CloseHandle(hFile);
		printf("fail to read file\n");
		return 0;
	}
	pbmf = (BITMAPINFO *)(pbmfh + 1);
	bmih = pbmf->bmiHeader;																								//����ļ���Ϣͷ
	bmp_width = bmih.biWidth;																							//����ļ����ؿ�
	bmp_height = bmih.biHeight;																							//����ļ����س�
	bitCount = bmih.biBitCount;
	pbits = (char *)(pbmfh + pbmfh->bfOffBits);																			//�������������ʼ��ַ	
	if(hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	column = ((p.bitColumns + 7)& ~7) >> 3;
	for(i = 0; i < bmp_height; i++)																		//���ԭͼ��
	{
		for(j = 0; j < column ; j++)
		{
			pbits[i * column + j] = (char)0xFF;
		}
	}
	
	hFile = CreateFile(pfileName, GENERIC_WRITE, 0, NULL,											
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);																		//������д��ͼ��
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		CloseHandle(hFile);
		printf("create file failed!\n");
		return 0;
	}
	
	bSuccess = WriteFile(hFile, pbmfh, dwFileSize, &dwBytesWritten, NULL);												//д������õ�����
	if(!bSuccess)
	{
		free(pbmfh);
		CloseHandle(hFile);
		printf("fail towrite file\n");
		return 0;
	}
	if(hFile)																											//�ر��ļ����
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	
	if(bitCount == 1)																									//1λ��
	{

		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//���밴1�ֽڲ���,���Գ���ӦΪ����((p.bitColumns + 7)& ~7) >> 3
			{
				temp = 255 - p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j;											//λͼ��4�ֽڲ���,���Գ���ӦΪ����((bmp_width * bitCount + 31) & ~31) >> 3
				pbits[column] = temp;
				//printf("%2x ", (255 - temp) & 0xFF);
			}
			//printf("\n");
		}
	}
	else if(bitCount == 4)																								//4λ��
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//���밴1�ֽڲ���,���Գ���ӦΪ����((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 4;										//λͼ��4�ֽڲ���,���Գ���ӦΪ����((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)																					//�ɶԸ�ֵ
				{
					cur_color = (temp & (0x01 << k)) >> k;																//��λȡ����Ӧ��ֵ								
					cur_col = column + (k / 2);
					pbits[cur_col] &= (((((1 - cur_color) * 15) << (4 * (1 - k % 2)))) | ((1 - k % 2) ? 0x0f : 0xf0));	//��ֵ
				}
			}
		}
	}
	else if(bitCount == 8)																								//8λ��
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//���밴1�ֽڲ���,���Գ���ӦΪ����((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 8;										//λͼ��4�ֽڲ���,���Գ���ӦΪ����((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)
				{
					cur_color = (temp & (0x01 << k)) >> k;																//��λȡ����Ӧ��ֵ								
					cur_col = column + k;
					pbits[cur_col] = (1 - cur_color) * 255;																//ֱ�ӽ��и�ֵ
				}
			}
		}
	}
	else if(bitCount == 24)																								//24λ��																				
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//���밴1�ֽڲ���,���Գ���ӦΪ����((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 3 * 8;									//λͼ��4�ֽڲ���,���Գ���ӦΪ����((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)
				{
					cur_color = (temp & (0x01 << k)) >> k;																//��λȡ����Ӧ��ֵ								
					cur_col = column + k * 3;
					pbits[cur_col] = (1 - cur_color) * 255;																//Rֵ
					pbits[cur_col + 1] = (1 - cur_color) * 255;															//Gֵ
					pbits[cur_col + 2] = (1 - cur_color) * 255;															//Bֵ
				}
			}
		}
	}
	else
	{
		return 0;
	}
	
	hFile = CreateFile(pfileName, GENERIC_WRITE, 0, NULL,											
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);																		//������д��ͼ��
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		CloseHandle(hFile);
		printf("create file failed!\n");
		return 0;
	}
	
	bSuccess = WriteFile(hFile, pbmfh, dwFileSize, &dwBytesWritten, NULL);												//д��������

	if(hFile)																											//�ر��ļ����
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return 1;
}