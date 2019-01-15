/*
 * recttools.h
 *
 *  Created on: 2016-11-13
 *      Author: ZQ
 */
//#include<common.h>
#ifndef RECTTOOLS_H_
#define RECTTOOLS_H_

void getBorder( const Rect_int original, const Rect_int limited, Rect_int *res);
void limit(Rect_int *rect, int y,int x);
//inline  x2(const Rect rect);


#endif /* RECTTOOLS_H_ */
