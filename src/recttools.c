/*
 * recttools.c
 *
 *  Created on: 2016-10-21
 *      Author: ZQ
 */
#include "KCF_Track.h"

inline  x2(const Rect_int rect)
{
    return rect.x + rect.width;
}

inline  y2(const Rect_int rect)
{
    return rect.y + rect.height;
}

void limit(Rect_int *rect, int x,int y)
{
	Rect_int limit;
//	Rect rect=*rect1;
	limit.width=x;
	limit.height=y;
	limit.x=0;
	limit.y=0;
	if (rect->x + rect->width > limit.x + limit.width)
		rect->x = (limit.width - rect->width);
    if (rect->y + rect->height > limit.y + limit.height)
		rect->y = (limit.height - rect->height);
    if (rect->x < limit.x)
    {
        //rect->width -= (limit.x - rect->x);
        rect->x = limit.x;
    }
    if (rect->y < limit.y)
    {
        //rect->height -= (limit.y - rect->y);
        rect->y = limit.y;
    }
    if(rect->width<0)
		rect->width=0;
    if(rect->height<0)
		rect->height=0;
}

void getBorder( const Rect_int original,const  Rect_int limited, Rect_int *res)
{
//    Rect res;
    res->x = limited.x - original.x;
    res->y = limited.y - original.y;
    res->width = x2(original) - x2(limited);
    res->height = y2(original) - y2(limited);
//    assert(res.x >= 0 && res.y >= 0 && res.width >= 0 && res.height >= 0);
//    return res;
}
