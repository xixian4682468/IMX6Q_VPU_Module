#ifndef _DFT_H_
#define _DFT_H_
#include "KCF_Track.h"
#ifndef MY_SWAP
#define MY_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))
#endif

#ifndef MY_PI
#define MY_PI   3.1415926535897932384626433832795
#endif

//#ifndef uchar
//#define uchar  unsigned char
//#endif

typedef struct
{
	double re;
	double im;
}my_complex_double;

typedef struct
{
	float re;
	float im;
}my_complex_float;

typedef struct
{
	unsigned char dims;
	int rows;
	int cols;
	int step;
	unsigned char channels;
	unsigned char *data;
}my_mat;

#ifndef my_size_t
#define my_size_t unsigned int
#endif


#define my_bool char


#define my_false 0

#define my_true 1

#define MY_MAX_LOCAL_DFT_SIZE  (1 << 15)

static void my_CCSIDFT(const float* src, float* dst, int n, int nf, int* factors, const int* itab,
	const my_complex_float* wave, int tab_size,const void* spec, my_complex_float* buf,
	int flags, double _scale);

static void my_CCSIDFT_32f(const float* src, float* dst, int n, int nf, int* factors,
	const int* itab, const my_complex_float* wave, int tab_size, const void* spec,
	my_complex_float* buf, int flags, double scale);

static void  my_CopyColumn(const uchar* _src, my_size_t src_step,
	uchar* _dst, my_size_t dst_step,
	int len, my_size_t elem_size);

static void my_CopyFrom2Columns(const uchar* _src, my_size_t src_step,
	uchar* _dst0, uchar* _dst1,
	int len, my_size_t elem_size);

static void
	my_CopyTo2Columns(const uchar* _src0, const uchar* _src1,
	uchar* _dst, my_size_t dst_step,
	int len, my_size_t elem_size);

void* my_cvAlignPtr(const void* ptr, int align);


void my_dft(my_mat * _src0, my_mat * _dst, int flags, int nonzero_rows);

static void my_DFT(const my_complex_float* src, my_complex_float* dst, int n,
	int nf, const int* factors, const int* itab,
	const my_complex_float* wave, int tab_size,
	const void* spec,my_complex_float* buf,
	int flags, double _scale);

static void my_DFT_32f(const my_complex_float* src, my_complex_float* dst, int n,
	int nf, const int* factors, const int* itab,
	const my_complex_float* wave, int tab_size,
	const void* spec, my_complex_float* buf,
	int flags, double scale);

static int my_DFTFactorize(int n, int* factors);

static void my_DFTInit(int n0, int nf, int* factors, int* itab,
	   int elem_size, void* _wave, int inv_itab);

static void
	my_ExpandCCS(uchar* _ptr, int n, int elem_size);

static void my_RealDFT(const float* src, float* dst, int n, int nf, int* factors, const int* itab,
	const my_complex_float* wave, int tab_size,const void* spec,
	my_complex_float* buf, int flags, double _scale);

static void my_RealDFT_32f(const float* src, float* dst, int n, int nf, int* factors,
	const int* itab, const my_complex_float* wave, int tab_size, const void* spec,
	my_complex_float* buf, int flags, double scale);

#endif
