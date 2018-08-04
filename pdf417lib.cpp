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
*当GLI(全球标识标记符)为0时在TC模式下的的ASCII字符对应值
***************************************************************/

/**************************************************************
*MIXED模式字符     0123456789&rt,:#-.$/+%*=^
***************************************************************/
char* MIXED_SET = "0123456789&\r\t,:#-.$/+%*=^";
/**************************************************************
*PUNCUTATION模式字符     ;<>@[\]_`~!rt,:n-.$/"|*()?{}'
***************************************************************/
char* PUNCTUATION_SET = ";<>@[\\]_`~!\r\t,:\n-.$/\"|*()?{}'";




/**************************************************
*初始化结构体listElement
***************************************************/
void listInit(pArrayList list) {
    list->capacity = 20;
    list->size = 0;
    list->array = (pListElement)malloc(list->capacity * sizeof(listElement));
}

/**************************************************
*释放结构体listElement内存
***************************************************/
static void listFree(pArrayList list) {
    free(list->array);
    list->array = NULL;
}

/**************************************************
*添加数据到listElement
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
*从listElement提取数据
***************************************************/
static pListElement listGet(pArrayList list, int idx) {
    if (idx >= list->size || idx < 0)
        return NULL;
    return list->array + idx;
}

/**************************************************
*从listElement移除数据
***************************************************/
static void listRemove(pArrayList list, int idx) {
    if (idx >= list->size || idx < 0)
        return;
    --list->size;
    memmove(list->array + idx, list->array + (idx + 1), (list->size - idx) * sizeof(listElement));
}

/**************************************************
*检查listElement指定元素
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
*初始化结构体pPdf417class
*************************************************/
void pdf417classInit(pPdf417class p)
{
	p->cwPtr = 0;
	p->cwPtr = 0;
	p->param = NULL;
}

/**************************************************
*初始化结构体pPdf417param
***************************************************/
void pdf417init(pPdf417param param) {
    param->options = 0;
    param->outBits = NULL;
    param->lenBits = 0;
    param->error = 0;
    param->lenText = 0;																						//原始数据位-1,现改为0
    param->text = "";
    param->yHeight = 3;
    param->aspectRatio = 0.5;
	param->codeColumns = 0;
	param->codeRows = 0;
	memset(param->codewords, 0, sizeof(param->codewords));
}

/**************************************************
*释放结构体pPdf417param内存
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
*结构体pPdf417param,17位码字操作
***************************************************/
void outCodeword17(pPdf417class p, int codeword) {
    int bytePtr = p->bitPtr / 8;																		//计算字节数
    int bit = p->bitPtr - bytePtr * 8;																	//计算偏移比特数
    p->param->outBits[bytePtr++] |= codeword >> (9 + bit) & (0xFF >> bit);								//码字右移位9+偏移量,使得字符加上码字第一个8位				
    p->param->outBits[bytePtr++] |= codeword >> (1 + bit) & 0xFF;										//码字右移位1+偏移量,使得字符加上码字第二个8位
    codeword <<= 8;
    p->param->outBits[bytePtr] |= codeword >> (1 + bit) & ((0xFF >> (7 - bit)) << (7 - bit));								//码字右移位1+偏移量,使得字符加上码字最后1位
    p->bitPtr += 17;																							//比特位移动到下一字符
}

/***************************************************
*反求17位码字
****************************************************/
void inCodeword17(pPdf417class p, int *codeword){
	int bytePtr = p->bitPtr / 8;
	int bit = p->bitPtr - bytePtr * 8;
	(*codeword) = 0;																				//00000001 11111111 11111110
	(*codeword) |= ((int)(p->param->outBits[bytePtr++] & 0xFF) & (0xFF >> bit)) << (9 + bit);						//第一个8-bit位
	(*codeword) |= ((int)(p->param->outBits[bytePtr++] & 0xFF)  << (1 + bit)) & (0xFF << (1 + bit));			//第二个8位
	(*codeword) |= (int)(p->param->outBits[bytePtr] & 0xFF) >> (7 - bit);								//第三个1+bit位	
	p->bitPtr += 17;
}

/**************************************************
*结构体pPdf417param,18位码字操作
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
*反求18位码字 未检测
****************************************************/
static void inCodeword18(pPdf417class p, int *codeword){
	int bytePtr = p->bitPtr / 8;
	int bit = p->bitPtr - bytePtr * 8;
	(*codeword) = 0;
	(*codeword) |= (p->param->outBits[bytePtr++] & (0xFF >> bit)) << (10 + bit);			//第一个8-bit位
	(*codeword) |= p->param->outBits[bytePtr++] << (2 + bit);								//第二个8位
	(*codeword) |= p->param->outBits[bytePtr] >> (8 - bit);									//第三个1+bit位
	p->bitPtr += 18;
}

/**************************************************
*将指定码字存入
***************************************************/
static void outCodeword(pPdf417class p, int codeword) {
    outCodeword17(p, codeword);
}

/**************************************************
*反求指定码字
***************************************************/
static void inCodeword(pPdf417class p, int *codeword) {
    inCodeword17(p, codeword);
}

/**************************************************
*将终止码字存入
***************************************************/
static void outStopPattern(pPdf417class p) {
    outCodeword18(p, STOP_PATTERN);
}


/**************************************************
*将起始码字存入
***************************************************/
static void outStartPattern(pPdf417class p) {
    outCodeword17(p, START_PATTERN);
}

 
/**********************************************
*根据错误数寻找最大纠错等级
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
*搜索输出符对应码字
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
*计算矩阵对应输出码
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
    p->param->bitColumns = START_CODE_SIZE * (p->param->codeColumns + 3) + STOP_SIZE;			//起始符比特数+数据比特数+终止符比特数+右空白符
    p->param->lenBits = ((p->param->bitColumns - 1) / 8 + 1) * p->param->codeRows;				//利用((比特列数 - 一位右空白符)/8+1)*行数得到总字节长
    p->param->outBits = (char*)malloc(p->param->lenBits);										//分配内存空间
    memset(p->param->outBits, 0, p->param->lenBits);											//初始化为0
	
    for (row = 0; row < p->param->codeRows; ++row) {
        p->bitPtr = ((p->param->bitColumns - 1) / 8 + 1) * 8 * row;								//计算第row行的比特起始位
        rowMod = row % 3;																		//计算当前行所在簇
        cluster = CLUSTERS[rowMod];																//取出对应码字
		/**************************************************
		*计算当前行左起始符码字
		***************************************************/
        outStartPattern(p);																
        edge = 0;
		/**************************************************
		*计算当前行左指示符码字
		***************************************************/
        switch (rowMod) {
        case 0:
            edge = 30 * (row / 3) + ((p->param->codeRows - 1) / 3);								//这里row是从0开始所以不用减1,30*INT[(行号-1)/3]+INT[(行数-1)/3]
            break;
        case 1:
            edge = 30 * (row / 3) + p->param->errorLevel * 3 + ((p->param->codeRows - 1) % 3);	//30*INT[(行号-1)/3]+错误纠正等级*3+(行数-1)mod3
            break;
        default:
            edge = 30 * (row / 3) + p->param->codeColumns - 1;									//30*INT[(行号-1)/3]+数据区列数-1
            break;
        }
        outCodeword(p, cluster[edge]);
		/**************************************************
		*计算当前行数据码字
		***************************************************/
        for (column = 0; column < p->param->codeColumns; ++column) {
			//printf("%2x ", p->param->codewords[codePtr]);
            outCodeword(p, cluster[p->param->codewords[codePtr++]]);
        }
        /**************************************************
		*计算当前行右指示符码字
		***************************************************/
        switch (rowMod) {
        case 0:
            edge = 30 * (row / 3) + p->param->codeColumns - 1;									//30*INT[(行号-1)/3]+数据区列数-1
            break;
        case 1:
            edge = 30 * (row / 3) + ((p->param->codeRows - 1) / 3);								//这里row是从0开始所以不用减1,30*INT[(行号-1)/3]+INT[(行数-1)/3]
            break;
        default:
            edge = 30 * (row / 3) + p->param->errorLevel * 3 + ((p->param->codeRows - 1) % 3);	//30*INT[(行号-1)/3]+错误纠正等级*3+(行数-1)mod3
            break;
        }
        outCodeword(p, cluster[edge]);
		/**************************************************
		*计算当前行右终止符码字
		***************************************************/
        outStopPattern(p);
    }
	/**************************************************
	*是否有选择翻转图片
	***************************************************/
    if (p->param->options & PDF417_INVERT_BITMAP) {
        char* pm = p->param->outBits;															//取出码字数据
        char* end = pm + p->param->lenBits;														//指针移动至最后一个字符的地址
        /*************************************************
		*位图翻转直至尾地址
		**************************************************/
		while (pm < end)
            *(pm++) ^= 0xff;
    }

	
}

/**************************************************
*计算矩阵对应码字
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
	//数据无损坏的情况下仅仅提取数据字节并进行解码
	for (row = 0; row < p->param->codeRows; ++row)																				//已知codeRows和codeCols
	{
		skip_col = START_CODE_SIZE / 4;
		
		rowMod = row % 3;
		p->bitPtr = p->param->codeColumns * 8 * row + 2 * START_CODE_SIZE;												//数据起始地址
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
	p->param->lenCodewords = p->param->codewords[0];																				//第一个codeword为码字长度
	maxErr = maxPossibleErrorLevel(MAX_DATA_CODEWORDS + 2 - p->param->lenCodewords);
	/*for(row = 1; row < p->param->lenCodewords; row++)	
	{
		printf("%2x ", p->param->codewords[row]);
	}
	printf("\n");*/
}

/***********************************************************
*计算纠错码字-采用Reed-Solomon算法
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
	
    if (p->param->errorLevel < 0 || p->param->errorLevel > 8)									//若纠错等级超出范围则统统归于0
        p->param->errorLevel = 0;
    A = ERROR_LEVEL[p->param->errorLevel];														//取出对应的纠错表
    Alength = 2 << p->param->errorLevel;														//利用2的(lenCodewords+1)次方求出纠错长度
    E = p->param->codewords + dest;																//移动至码字尾部
    memset(E, 0, Alength * sizeof(int));
    lastE = Alength - 1;
	/*******************************************************
	*求错误码
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
	*对错误码求补
	********************************************************/
    for (j = 0; j < Alength; ++j)
        E[j] = (MOD - E[j]) % MOD;
}

/*************************************************************
*获得当前字符类型
**************************************************************/
static int getTextTypeAndValue(char* text, int size, int idx) {
    int c;
    char *ms, *ps;
    if (idx >= size)																			//保证当前字符编号小于字符串末尾编号
        return 0;
    c = text[idx];
    if (c >= 'A' && c <= 'Z')																	//大写字母
        return (ALPHA + c - 'A');
    if (c >= 'a' && c <= 'z')																	//小写字母
        return (LOWER + c - 'a');
    if (c == ' ')																				//空格
        return (ALPHA + LOWER + MIXED + SPACE);
    ms = strchr(MIXED_SET, c);																	//MIXED字符中有无存在
    ps = strchr(PUNCTUATION_SET, c);															//PUNC字符中有无存在
    if (!ms && !ps)																				//不是字母、数字、标点符,即为汉字
        return (ISBYTE + (c & 0xff));
    if (ms - MIXED_SET == ps - PUNCTUATION_SET)													//既是MIXED字符又是PUNCTUATION字符
        return (MIXED + PUNCTUATION + (ms - MIXED_SET));
    if (ms != NULL)																				//仅仅是MIXED字符
        return (MIXED + (ms - MIXED_SET));
    return (PUNCTUATION + (ps - PUNCTUATION_SET));												//仅仅是PUNCTUATION字符
}

/********************************************************
*状态转换机制
-->表转移
->表锁定

TC子模式转换
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
模式转换
TC->BC					901(mod6!=0)/924(mod6==0)
TC->NC					902
TC-->BC					913
BC->TC					900
BC->NC					902
NC->TC					900
NC->BC					901/924
中文符号				BYTESHIT
*********************************************************/
void textCompaction(pPdf417class p, int start, int length) {
    int dest[ABSOLUTE_MAX_TEXT_SIZE * 2];														//存放结果
    char* text = p->param->text;																//字符原始数据
    int mode = ALPHA;																			//初始模式为文本模式中的ALPHA子模式
    int ptr = 0;																				//当前字符号
    int fullBytes = 0;
    int v = 0;
    int k;
    int size;
    memset(dest, 0, sizeof(dest));																//对dest数组清零
    length += start;																			//字符串末尾号
    for (k = start; k < length; ++k) {															//从偏移量start开始
        v = getTextTypeAndValue(text, length, k);												//获得当前字符类型
        if ((v & mode) != 0) {																	//如果该字符与当前模式一致
            dest[ptr++] = v & 0xff;																//将后8位即字符ASCII码值存入
            continue;
        }
        if ((v & ISBYTE) != 0) {																//中文汉字
            if ((ptr & 1) != 0) {																//当前有奇数值,需要填充为偶数
                dest[ptr++] = (mode & PUNCTUATION) != 0 ? PAL : PS;								//如果为PUNCTUATION模式则需改变模式
                mode = (mode & PUNCTUATION) != 0 ? ALPHA : mode;								//改变当前模式
            }
            dest[ptr++] = BYTESHIFT;
            dest[ptr++] = v & 0xff;
            fullBytes += 2;
            continue;
        }
        switch (mode) {																			//当前模式判断
        case ALPHA:																				//ALPHA模式
            if ((v & LOWER) != 0) {																//当前字符为MIXED模式
                dest[ptr++] = LL;																//ALPHA->LOWER ll
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = LOWER;																	//锁定LOWER模式
            }
            else if ((v & MIXED) != 0) {														//当前字符为MIXED模式
                dest[ptr++] = ML;																//ALPHA->MIXED ml
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = MIXED;																	//锁定MIXED模式
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = ML;																//后两个字符均是PUNCTUATION字符
                dest[ptr++] = PL;																//ALPHA->MIXED,MIXED->PUNCTUATION
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = PUNCTUATION;																//锁定模式为PUNCTUATION
            }
            else {
                dest[ptr++] = PS;																//仅仅是PUNCTUATION模式,模式转移,PLPHA-->PUNCTUATION
                dest[ptr++] = v & 0xff;															//记录ASCII码值
            }
            break;
        case LOWER:																				//LOWER模式
            if ((v & ALPHA) != 0) {																//当前字符为ALPHA字符
                if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & ALPHA) != 0) {
                    dest[ptr++] = ML;															//后两个字符均为ALPHA,LOWER->MIXED
                    dest[ptr++] = AL;															//MIXED->ALPHA
                    mode = ALPHA;																//锁定模式为ALPHA
                }
                else {
                    dest[ptr++] = AS;															//LOWER-->ALPHA
                }
                dest[ptr++] = v & 0xff;															//记录ASCII码值
            }
            else if ((v & MIXED) != 0) {														//当前字符为MIXED字符
                dest[ptr++] = ML;																//LOWER->MIXED
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = MIXED;																	//锁定模式为MIXED
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = ML;																//LOWER->MIXED
                dest[ptr++] = PL;																//MIXED->PUNCTUATION
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = PUNCTUATION;																//多订模式为PUNCTUATION
            }
            else {
                dest[ptr++] = PS;																//LOWER-->PUNCTUATION
                dest[ptr++] = v & 0xff;															//记录ASCII码值
            }
            break;
        case MIXED:																				//模式为MIXED
            if ((v & LOWER) != 0) {																//当前字符为LOWER字符
                dest[ptr++] = LL;																//MIXED->LOWER ll
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = LOWER;																	//锁定模式为LOWER
            }
            else if ((v & ALPHA) != 0) {														//当前字符为ALPHA模式
                dest[ptr++] = AL;																//MIXED->ALPHA al
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = ALPHA;																	//锁定模式为ALPHA
            }
            else if ((getTextTypeAndValue(text, length, k + 1) & getTextTypeAndValue(text, length, k + 2) & PUNCTUATION) != 0) {
                dest[ptr++] = PL;																//MIXED->PUNCTUATION pl
                dest[ptr++] = v & 0xff;															//记录ASCII码值
                mode = PUNCTUATION;																//锁定模式为PUNCTUATION
            }
            else {																				//模式转移
                dest[ptr++] = PS;																//MIXED-->PUNCTUATION ps
                dest[ptr++] = v & 0xff;															//记录ASCII码值
            }
            break;
        case PUNCTUATION:																		//PUNCTUATION模式
            dest[ptr++] = PAL;																	//PUNCTUATION->ALPHA 
            mode = ALPHA;																		//锁定模式为ALPHA
            --k;																				//重新比较
            break;
        }
    }
    if ((ptr & 1) != 0)																			//奇数个
        dest[ptr++] = PS;																		//尾部填充ps
    size = (ptr + fullBytes) / 2;																//得到字符对数
    if (size + p->cwPtr > MAX_DATA_CODEWORDS) {
        p->param->error = PDF417_ERROR_TEXT_TOO_BIG;
        return;
    }
    length = ptr;																				
    ptr = 0;
    while (ptr < length) {																			//计算码字
        v = dest[ptr++];																			//当前高位大于30直接使用
        if (v >= 30) {																				//碰到汉字
            p->param->codewords[p->cwPtr++] = v;
            p->param->codewords[p->cwPtr++] = dest[ptr++];
        }
        else																						//当前高位小于30采用30*h+l
            p->param->codewords[p->cwPtr++] = v * 30 + dest[ptr++];
    }
}

/**********************************************************************
*文本解压
***********************************************************************/
void textUncompress(pPdf417class p, int start, int length)
{
	int dest[ABSOLUTE_MAX_TEXT_SIZE * 2];															//存放结果
    int *codewords = p->param->codewords;
    char* text = p->param->text;																	//存放最终字符结果字符
    int mode = ALPHA;																				//初始模式为文本模式中的ALPHA子模式
    int ptr = 0;																					//当前字符号
    int fullBytes = 0;
    int v = 0;
    int k;
	ptr = 0;
	k = start;
	length += start;
	while(k < length)
	{
		if(p->param->codewords[k] == BYTESHIFT)														//如果是汉字字符
		{
			dest[ptr++] = p->param->codewords[k++];
			dest[ptr++] = p->param->codewords[k++];
			continue;
		}
		dest[ptr++] = p->param->codewords[k] / 30;													//不是汉字的情况下求取30基的整数和余数
		dest[ptr++] = p->param->codewords[k] % 30;
		k += 1;
	}

	if(dest[ptr - 1] == PS)																			//尾部填充PS则原先为奇数个
	{
		ptr--;
	}
	length = ptr;
	ptr = 0;
	
	while(ptr < length)
	{
		v = dest[ptr];
		if(v == BYTESHIFT)																			//如果为汉字,已经对齐
		{
			p->param->text[p->param->lenText++] = dest[ptr + 1] & 0xff;								//如果为PUNCTUATION模式则需改变模式
			p->param->text[p->param->lenText++] = dest[ptr + 3] & 0xff;
			ptr += 4;
            mode = (mode & PUNCTUATION) != 0 ? ALPHA : mode;
			continue;
		}
		if(v == SPACE)																				//空格
		{
			p->param->text[p->param->lenText++] = ' ';
			ptr += 1;
			continue;
		}
		switch(mode)
		{
		case ALPHA:
			/************************************
			*ALPHA->MIXED				ml	锁定
			*ALPHE->LOWER				ll	锁定
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
					mode = PUNCTUATION;																		//锁定到PUNCTUATION
				}
				else
				{
					p->param->text[p->param->lenText++] = MIXED_SET[dest[ptr + 1]];							//锁定到MIXED
					ptr += 2;
					mode = MIXED;
				}
			}
			else if(v == PS)
			{
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//模式转移
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
			*LOWER-->ALPHA				as	转换
			*LOWER-->PUNCTUATION		ps	锁定
			*LOWER->MIXED				ml	锁定
			**************************************/
			if(v == AS)
			{
				p->param->text[p->param->lenText++] = dest[ptr + 1] + 'A';
				ptr += 2;																					//模式转移
			}
			else if(v == PS)
			{
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//模式转移
				ptr += 2;
			}
			else if(v == ML)
			{
				if(dest[ptr + 1] == AL)
				{
					p->param->text[p->param->lenText++] = dest[ptr + 2] + 'A';								//锁定ALPHA模式
					ptr += 3;
					mode = ALPHA;
				}
				else if(dest[ptr + 1] == PL)
				{
					p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 2]];					//锁定PUNCTUATION模式
					ptr += 3;
					mode = PUNCTUATION;
				}
				else
				{
					p->param->text[p->param->lenText++] = MIXED_SET[dest[ptr + 1]];							//锁定到MIXED
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
			*MIXED->ALPHA				al	锁定
			*MIXED->LOWER				ll	锁定
			*MIXED->PUCTUATION			pl	锁定
			*MIXED-->PUNCTUATION		ps	转换
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
				p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];						//锁定PUNCTUATION模式
				ptr += 2;
				mode = PUNCTUATION;
			}
			else if(v == PS)
			{	
				if(dest[ptr + 1] != BYTESHIFT)
				{
					p->param->text[p->param->lenText++] = PUNCTUATION_SET[dest[ptr + 1]];					//转移PUNCTUATION模式
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
			*PUNCTUATION->ALPHA			pal	锁定
			**************************************/
			mode = ALPHA;
			ptr += 1;
			break;
		}
	}
}

/***********************************************************************
*基值转换(10->900)-传播算法
************************************************************************/
void basicNumberCompaction(pPdf417class p, int start, int length) {
    char* text = p->param->text;
    int* ret = p->param->codewords + p->cwPtr;
    int retLast = length / 3;																			//新的长度为INT[LENGTH/3]+1
    int ni, k;
    p->cwPtr += retLast + 1;
    memset(ret, 0, (retLast + 1) * sizeof(int));
    ret[retLast] = 1;																					//前导符为1
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
*基值转换(900->10)-传播算法 修改完毕时间:2015/4/2
************************************************************************/
void basicNumberUncompress(pPdf417class p, int start, int length) {
    char* text = p->param->text;
    int* ret = p->param->codewords + p->cwPtr;
	int* res;
    int retLast = length * 3;																		//长度为(length - 1) * 3 - 1
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

	for(ni = 0; ni <= retLast && res[ni] != 1; ni++);													//寻找前导符1

	for(ni += 1; ni <= retLast; ni++)																	//转换为字符值
	{
		p->param->text[p->param->lenText++] = res[ni] + '0';
	}
	
}	

/*********************************************************************
*数字转换机制 44个字节->15个码字
**********************************************************************/
void numberCompaction(pPdf417class p, int start, int length) {
    int full = (length / 44) * 15;																		//码字总数
    int size = length % 44;																				//最后一组长度
    int k;
    if (size == 0)																						//长度正好为44的整数倍
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
*数字转换机制 15个码字->44个字节
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
*基值转换(256->900)
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
*基值转换(900->256)
*********************************************************/
void byteUncompress6(pPdf417class p, int start) {
    int length = 5;
    int* codewords = p->param->codewords;
    int* ret = NULL;
	//char* temp = p->param->text;
    int retLast = 5;
    int ni, k;

	/**************************************************************
	*malloc发生内存泄露
	***************************************************************/
	/****************************************************************
	if(!p->param->text)
	{
		p->param->text = malloc(length);
		memset(p->param->text, '\0', p->param->lenText + length);
	}
	else																											//扩充字节空间
	{
		if(temp)
		{
			free(temp);
			temp = NULL;
		}
		temp = malloc(p->param->lenText);
		memset(temp, '\0', p->param->lenText);
		memcpy(temp, p->param->text, p->param->lenText);															//保存原先字符串
		free(p->param->text);
		p->param->text = NULL;
		p->param->text = malloc(p->param->lenText + length);														//更新文本内存长度
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
	for(ni = 0; ni <= retLast; ni++)																				//写入字符
	{
		p->param->text[p->param->lenText++] = (ret[ni] & 0xff);
	}																				//文本长度增加6
}

/****************************************************************
*BC-字节压缩 6个字节->5个码字
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
    if (size + p->cwPtr > MAX_DATA_CODEWORDS) {																		//不得超过最大字节数
        p->param->error = PDF417_ERROR_TEXT_TOO_BIG;
        return;
    }
    length += start;
    for (k = start; k < length; k += 6) {
        size = length - k < 6 ? length - k : 6;																		//于2015/3/31修改:原始代吗size = length - k < 44 ? length - k : 6;	
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
*BC-字节解压 5个码字->6个字节
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
        size = length - k < 5 ? length - k : 5;																		//于2015/3/31修改:原始代吗size = length - k < 44 ? length - k : 6;	
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
*字符串截断
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
        if (c >= '0' && c <= '9') {																			//记录数字字符
            if (nd == 0)
                startN = k;
            ++nd;
            continue;
        }
        if (nd >= 13) {																						//连续的数字个数大于13
            if (lastP != startN) {
                c = text[lastP];
                ptrS = lastP;
                lastTxt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
                for (j = lastP; j < startN; ++j) {															//前次数字终点到当前数字起点
                    c = text[j];
                    txt = (c >= ' ' && c < 127) || c == '\r' || c == '\n' || c == '\t';
                    if (txt != lastTxt) {
                        listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, j);								//是汉字则为BYTE,不是则为TEXT
                        lastP = j;
                        lastTxt = txt;
                    }
                }
                listAdd(list, (char)(lastTxt ? 'T' : 'B'), lastP, startN);									//是TEXT还是BYTE
            }
            listAdd(list, 'N', startN, k);																	//插入数字起点和终点
            lastP = k;																						//更新断点
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
        if (checkElementType(v, 'B') && getElementLength(v) == 1) {													//当前队列类型为BYTE且长度为1
            if (checkElementType(vp, 'T') && checkElementType(vn, 'T')												//左队列和右队列均为TEXT
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
        if (checkElementType(v, 'T') && getElementLength(v) >= 5) {												//当前队列为TEXT且长度大于等于5
            int redo = 0;
            if ((checkElementType(vp, 'B') && getElementLength(vp) == 1) || checkElementType(vp, 'T')) {		//左队列类型为BYTE且长度小于5或者左队列类型为TEXT
                redo = 1;
                v->start = vp->start;
                listRemove(list, k - 1);
                --k;
            }
            if ((checkElementType(vn, 'B') && getElementLength(vn) == 1) || checkElementType(vn, 'T')) {		//右队列类型为BYTE且长度小于5或者左队列类型为TEXT
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
            if ((checkElementType(vp, 'T') && getElementLength(vp) < 5) || checkElementType(vp, 'B')) {			//左队列类型为TEXT且长度小于5或者左队列类型为BYTE
                redo = 1;
                v->start = vp->start;
                listRemove(list, k - 1);
                --k;
            }
            if ((checkElementType(vn, 'T') && getElementLength(vn) < 5) || checkElementType(vn, 'B')) {			//右队列类型为TEXT且长度小于5或者右队列类型为BYTE
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
*获得元素类型
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
*字符串拼凑 还未进行优化处理
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
	k = 1;																													//codeword起点为1
	while(k < codeLength)
	{
		type = GetElementType(p->param->codewords[k]);
		if(type != '\0' || (type == '\0' && k == 1))																			//起始模式
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
			if(endN - startN >= 1)									//有发生模式转换或者抵达末尾?
				listAdd(list, type, startN, endN);
			k = endN;
			continue;
		}
	}
	if(p->param->error)
		return ;
}

/********************************************************
*汇总所有转换
********************************************************/
void assemble(pPdf417class p, pArrayList list) {
    int k;
    if (list->size == 0)
        return;
    p->cwPtr = 1;																										//codeword起点为1
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
*汇总所有逆向想转换 还未进行优化处理
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
        case 'b':																								//非整除6
            byteUncompress(p, v->start, v->end - v->start, 'b');
			break;
		case 'B':																								//整除6
			byteUncompress(p, v->start, v->end - v->start, 'B');
            break;
        }
        if (p->param->error)
            return;
	}
}

/********************************************************
*原始字符元素列表
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
*输出字符元素列表
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
*获得最大平方
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
*绘制码字 附带其它选择
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
		*查看数据的编码
		*****************************************************/
		/*for(i = 0; i < pp.cwPtr; i++)
		{
			printf("%2x ", pp.param->codewords[i]);
		}*/
        if (p->error)
            return;
        p->codewords[0] = p->lenCodewords = pp.cwPtr;																//预留第一位为码字长度
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
	*判断错误是否符合规范
	*****************************************/
    if (p->errorLevel < 0)
        p->errorLevel = 0;
    else if (p->errorLevel > maxErr)
        p->errorLevel = maxErr;
    if (p->codeColumns < 1)
        p->codeColumns = 1;
	/****************************************
	*判断码字是否符合规范
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
*从位图得到输出字节
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
	int firstPos = 0, lastPos = 0, curPos = 0, curCode = 0;																//起始位置,终止位置,当前字节位置,当前码字序号
	int i = 0, j = 0, k = 0, oldsize = 0, newsize = 0;
	BOOL haveFoundStartLine = FALSE, 
		haveFoundStartCol = FALSE, 
		haveFoundEndCol = FALSE, 
		haveFoundEndLine = FALSE;
	int startLine = 0, endLine = 0, startCol = 0, endCol = 0;
	if(pfileName == NULL)																								//文件名为空
	{
		//printf("file name is null\n");
		return 0;
	}
	hfile = CreateFile(pfileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,												//读出位图文件信息
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hfile == INVALID_HANDLE_VALUE)																					//打开位图失败
	{
		//printf("open file fail\n");
		return 0;
	}
	dwFileSize = GetFileSize(hfile, &dwHighSize);
	if(dwHighSize)																										//文件大于2G
	{
		//printf("can't deal with this size\n");
		CloseHandle(hfile);
		return 0;
	}
	pbmfh = (BITMAPFILEHEADER *)malloc(dwFileSize);
	if(pbmfh == NULL)																									//内存分配失败
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
	if(bitCounts == 1)																									//1位深
	{
		column = ((bmpWidth * bitCounts + 31) & ~31) >> 3;
		
		for(i = 0; i < bmpHeight && !haveFoundEndLine; i++)																//搜寻起始行,终止行,起始列,终止列
		{
			for(j = 0; j < column && !haveFoundStartLine; j++)															//扫描行直至起始行
			{
				if((pbits[i * column + j] & 0xFF) != 0xFF)
				{
					haveFoundStartLine = TRUE;
					startLine = i;
				}
			}
			for(j = 0; j < column && (!haveFoundEndCol || !haveFoundStartCol); j++)																//从起始列开始
			{
				for(k = 0; k < 8 && !haveFoundStartCol; k++)
				{
					int temp = 0x01;
					if((((pbits[i * column + j] & 0xFF) & (temp << k)) >> k) == 0)									//提取像素点
					{
						haveFoundStartCol = TRUE;
						startCol = j;
						firstPos = i * column + j + k;																	//第一个为黑的像素
						p->bitPtr = firstPos;
					}
				}
				
				if(haveFoundStartCol && (pbits[i * column + j] & 0xFF) == 0xFF)											//最后一列+1
				{
					haveFoundEndCol = TRUE;
					endCol = j - 1;
					p->param->codeColumns = endCol - startCol + 1;															//获得列的字节长度
					p->param->bitColumns = (endCol - startCol + 1) * 8;	
				}
			}

			for(j = startCol; j <= endCol && (pbits[i * column + j] & 0xFF) == 0xFF; j++);
			if(j == endCol + 1)																							//最后一行加1
			{
				haveFoundEndLine = TRUE;
				endLine = i - 1;																						//确定尾行号
				p->param->codeRows = endLine - startLine + 1;															//获得行高
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
			if(p->param->lenBits)																						//不断更新outBits的字节长
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
			for(j = startCol; j <= endCol; j++)																			//将数据写回到outBits中
			{
				p->param->outBits[p->param->lenBits++] = ~pbits[i * column + j];									//编码中1为黑 0为白,故需取反
				//printf("%2x ", (255 - pbits[i * column + j]) & 0xFF);
			}
			//printf("\n");
		}
	}
	else if(bitCounts == 4)																								//4位深
	{
	}
	else if(bitCounts == 8)																								//8位深
	{
	}
	else if(bitCounts == 24)																							//24位深
	{
	}
	else if(bitCounts == 32)
	{
		//字节列宽
		column = ((bmpWidth * bitCounts + 31) & ~31) >> 3;
		
		for(i = 0; i < bmpHeight && !haveFoundEndLine; i++)																//搜寻起始行,终止行,起始列,终止列
		{
			for(j = 0; j < column && !haveFoundStartLine; j += 4)															//扫描行直至起始行
			{
				if(pbits[i * column + j + 1] != char(0XFF)
					|| pbits[i * column + j + 2] != char(0XFF)
					|| pbits[i * column + j + 3] != char(0XFF))
				{
					haveFoundStartLine = TRUE;
					startLine = i;
				}
			}
			//从左往右选取
			for(j = 0; j < column && (!haveFoundEndCol || !haveFoundStartCol); j += 4)																//从起始列开始
			{
				if(pbits[i * column + j + 1] == 0
					&& pbits[i * column + j + 2] == 0
					&& pbits[i * column + j + 3] == 0)									//提取像素点
				{
					haveFoundStartCol = TRUE;
					startCol = j;
					firstPos = i * column / 32 + j;																	//第一个为黑的像素
					p->bitPtr = firstPos;
					break;
				}
			}
			
			//从右往左选取
			for(j = column; j >= 0 && !haveFoundEndCol; j -= 4)																//从起始列开始
			{
				if(pbits[i * column + j + 1] == 0
					&& pbits[i * column + j + 2] == 0
					&& pbits[i * column + j + 3] == 0)									//提取像素点
				{
					haveFoundEndCol = TRUE;
					endCol = j;
					p->param->codeColumns = (endCol - startCol + 1) / 32;												//获得列的字节长度
					p->param->bitColumns = p->param->codeColumns * 8;
					break;
				}
			}

			for(j = startCol; j <= endCol
				&& pbits[i * column + j + 1] == char(0xFF)
				&& pbits[i * column + j + 2] == char(0xFF)
				&& pbits[i * column + j + 3] == char(0xFF); j += 4);
			if(j > endCol)																						//最后一行加1
			{
				haveFoundEndLine = TRUE;
				endLine = i - 1;																						//确定尾行号
				p->param->codeRows = endLine - startLine + 1;															//获得行高
				p->param->bitColumns = p->param->codeColumns * 8;
				p->param->lenBits = p->param->codeColumns * p->param->codeRows;
				p->param->lenCodewords = p->param->codeRows * (p->param->codeColumns / 8 + 1);
				continue;
			}
			else if(i == bmpHeight - 1 && !haveFoundEndLine)
			{
				haveFoundEndLine = TRUE;
				endLine = i;																						//确定尾行号
				p->param->codeRows = endLine - startLine + 1;															//获得行高
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
			if(p->param->lenBits)																						//不断更新outBits的字节长
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
			for(j = startCol; j < endCol; j += 32, p->param->lenBits++)																			//将数据写回到outBits中
			{
				int k = 0;
				p->param->outBits[p->param->lenBits] = 0;
				for(k = 0; k < 8; k++)
				{
					//每一个像素1位 
					p->param->outBits[p->param->lenBits] |= (((((unsigned char)(pbits[i * column + j + k * 4 + 1]) + (unsigned char)(pbits[i * column + j + k * 4 + 2]) + (unsigned char)(pbits[i * column + j + k * 4 + 3])) / 765))  << (8 - (k + 1)));									//编码中1为黑 0为白,故需取反
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
*恢复字符串 未进行优化
********************************************************************/
void recoverString(pPdf417class p, char *pfileName, pArrayList list)
{
	GetBitsFromBMP(p, pfileName);																						//从像素数据恢复输出串
	inPaintCode(p);																										//反编码
	connectString(p, list);																								//拼凑字符串
	bitsList(p, list);
	unassemble(p, list);																								//恢复字符串
}


/*****************************************************************
*将码字写入数据位图 允许1、4、8、24位位图
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
	hFile = CreateFile(pfileName, GENERIC_READ, 0, NULL,																//读出位图文件信息
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		//printf("create file failed!\n");
		hFile = CreateFile(pfileName, GENERIC_READ, 0, NULL,																//读出位图文件信息
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			return 0;
	}
	dwFileSize = GetFileSize(hFile, &dwHighSize);																		//获得文件字节大小
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
	bmih = pbmf->bmiHeader;																								//获得文件信息头
	bmp_width = bmih.biWidth;																							//获得文件像素宽
	bmp_height = bmih.biHeight;																							//获得文件像素长
	bitCount = bmih.biBitCount;
	pbits = (char *)(pbmfh + pbmfh->bfOffBits);																			//获得像素数据起始地址	
	if(hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	column = ((p.bitColumns + 7)& ~7) >> 3;
	for(i = 0; i < bmp_height; i++)																		//清除原图像
	{
		for(j = 0; j < column ; j++)
		{
			pbits[i * column + j] = (char)0xFF;
		}
	}
	
	hFile = CreateFile(pfileName, GENERIC_WRITE, 0, NULL,											
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);																		//将数据写入图像
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		CloseHandle(hFile);
		printf("create file failed!\n");
		return 0;
	}
	
	bSuccess = WriteFile(hFile, pbmfh, dwFileSize, &dwBytesWritten, NULL);												//写入清除用的像素
	if(!bSuccess)
	{
		free(pbmfh);
		CloseHandle(hFile);
		printf("fail towrite file\n");
		return 0;
	}
	if(hFile)																											//关闭文件句柄
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	
	if(bitCount == 1)																									//1位深
	{

		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//编码按1字节补齐,所以长度应为长度((p.bitColumns + 7)& ~7) >> 3
			{
				temp = 255 - p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j;											//位图按4字节补齐,所以长度应为长度((bmp_width * bitCount + 31) & ~31) >> 3
				pbits[column] = temp;
				//printf("%2x ", (255 - temp) & 0xFF);
			}
			//printf("\n");
		}
	}
	else if(bitCount == 4)																								//4位深
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//编码按1字节补齐,所以长度应为长度((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 4;										//位图按4字节补齐,所以长度应为长度((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)																					//成对赋值
				{
					cur_color = (temp & (0x01 << k)) >> k;																//按位取出对应数值								
					cur_col = column + (k / 2);
					pbits[cur_col] &= (((((1 - cur_color) * 15) << (4 * (1 - k % 2)))) | ((1 - k % 2) ? 0x0f : 0xf0));	//赋值
				}
			}
		}
	}
	else if(bitCount == 8)																								//8位深
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//编码按1字节补齐,所以长度应为长度((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 8;										//位图按4字节补齐,所以长度应为长度((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)
				{
					cur_color = (temp & (0x01 << k)) >> k;																//按位取出对应数值								
					cur_col = column + k;
					pbits[cur_col] = (1 - cur_color) * 255;																//直接进行赋值
				}
			}
		}
	}
	else if(bitCount == 24)																								//24位深																				
	{
		for(i = 0; i < p.codeRows; i++)
		{
			for(j = 0; j < (((p.bitColumns + 7)& ~7) >> 3); j++)														//编码按1字节补齐,所以长度应为长度((p.bitColumns + 7)& ~7) >> 3
			{
				temp = p.outBits[i * (((p.bitColumns + 7)& ~7) >> 3) + j ] & 0xff;
				column = i * (((bmp_width * bitCount + 31) & ~31) >> 3) + j * 3 * 8;									//位图按4字节补齐,所以长度应为长度((bmp_width * bitCount + 31) & ~31) >> 3
				for(k = 0; k < 8; k++)
				{
					cur_color = (temp & (0x01 << k)) >> k;																//按位取出对应数值								
					cur_col = column + k * 3;
					pbits[cur_col] = (1 - cur_color) * 255;																//R值
					pbits[cur_col + 1] = (1 - cur_color) * 255;															//G值
					pbits[cur_col + 2] = (1 - cur_color) * 255;															//B值
				}
			}
		}
	}
	else
	{
		return 0;
	}
	
	hFile = CreateFile(pfileName, GENERIC_WRITE, 0, NULL,											
	CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);																		//将数据写入图像
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("");
		CloseHandle(hFile);
		printf("create file failed!\n");
		return 0;
	}
	
	bSuccess = WriteFile(hFile, pbmfh, dwFileSize, &dwBytesWritten, NULL);												//写入新数据

	if(hFile)																											//关闭文件句柄
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return 1;
}