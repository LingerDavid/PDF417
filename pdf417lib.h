/*
 * Copyright 2003 by Paulo Soares.
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
#ifndef __PDF417LIB_H__
#define __PDF417LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PDF417_USE_ASPECT_RATIO     0
#define PDF417_FIXED_RECTANGLE      1
#define PDF417_FIXED_COLUMNS        2
#define PDF417_FIXED_ROWS           4
#define PDF417_AUTO_ERROR_LEVEL     0
#define PDF417_USE_ERROR_LEVEL      16
#define PDF417_USE_RAW_CODEWORDS    64
#define PDF417_INVERT_BITMAP        128

#define PDF417_ERROR_SUCCESS        0
#define PDF417_ERROR_TEXT_TOO_BIG   1
#define PDF417_ERROR_INVALID_PARAMS 2


typedef struct _pdf417param {
    char *outBits;			/*输出码字结果*/
    int lenBits;			/*数据的总字节长度*/
    int bitColumns;			/*二维码总比特宽*/
    int codeRows;			/*二维码行数*/
    int codeColumns;		/*二维码列数*/	
    int codewords[928];		/*数据编码结果*/
    int lenCodewords;		/*码字表长度*/
    int errorLevel;			/*纠错等级*/
    char *text;				/*原始数据*/
    int lenText;			/*数据长度*/
    int options;			/*编码选项*/
    float aspectRatio;		/*缩放比例*/
    float yHeight;			/*行高*/
    int error;				/*错误*/
} pdf417param, *pPdf417param;


typedef struct _listElement {
    char type;				/*队列类型*/
    int start;				/*队列起始位置,不包含断点*/
    int end;				/*队列终止位置,包含终点*/
} listElement, *pListElement;

typedef struct _arrayList {
    pListElement array;		/*不同类型的队列*/
    int size;				/*数组长度*/
    int capacity;			/*数组容量*/
} arrayList, *pArrayList;

typedef struct _pdf417class {
    int bitPtr;				/*bitouts输出字符尾指针*/
    int cwPtr;				/*codewords码字尾指针*/
    pdf417param *param;
} pdf417class, *pPdf417class;


//FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
/**************************************************************
*绘制编码
**************************************************************/
void paintCode(pPdf417param p);

/**************************************************************
*初始化PDF417结构体
**************************************************************/
void pdf417init(pPdf417param param);

/**************************************************************
*释放PDF417内存
**************************************************************/
void pdf417free(pPdf417param param);


int PaintBMP(pdf417param p, char *pfileName);

void listInit(pArrayList list);

void inPaintCode(pPdf417class p);

void connectString(pPdf417class p, pArrayList list);

void unassemble(pPdf417class p, pArrayList list);


void recoverString(pPdf417class p, char *pfileName, pArrayList list);
#ifdef __cplusplus
}
///////////////////////////////////////////////////////////////////////////////////////////
#endif

#endif
