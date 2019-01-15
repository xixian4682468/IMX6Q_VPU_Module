
/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <linux/videodev2.h>
#include "vpu_test.h"
#include "Communication.h"


#define TEST_BUFFER_NUM 3
#define MAX_CAPTURE_MODES   10

static int cap_fd = -1;
struct capture_testbuffer cap_buffers[TEST_BUFFER_NUM];
static int sizes_buf[MAX_CAPTURE_MODES][2];

static int getCaptureMode(int width, int height)
{
	int i, mode = -1;

	for (i = 0; i < MAX_CAPTURE_MODES; i++) {
		if (width == sizes_buf[i][0] &&
		    height == sizes_buf[i][1]) {
			mode = i;
			break;
		}
	}

	return mode;
}

int
v4l_start_capturing(void)
{
	unsigned int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(cap_fd, VIDIOC_QUERYBUF, &buf) < 0) {
			err_msg("VIDIOC_QUERYBUF error\n");
			return -1;
		}

		cap_buffers[i].length = buf.length;
		cap_buffers[i].offset = (size_t) buf.m.offset;
		
//*************************参考自主添加*******************************************
        cap_buffers[i].start = mmap (NULL, cap_buffers[i].length,  PROT_READ | PROT_WRITE, MAP_SHARED, 
			 	cap_fd, cap_buffers[i].offset);
		memset(cap_buffers[i].start, 0xFF, cap_buffers[i].length);
//*****************************************************************************
	}

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.m.offset = cap_buffers[i].offset;
		if (ioctl(cap_fd, VIDIOC_QBUF, &buf) < 0) {
			err_msg("VIDIOC_QBUF error\n");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(cap_fd, VIDIOC_STREAMON, &type) < 0) {
		err_msg("VIDIOC_STREAMON error\n");
		return -1;
	}

	return 0;
}

void
v4l_stop_capturing(void)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(cap_fd, VIDIOC_STREAMOFF, &type);
	close(cap_fd);
	cap_fd = -1;
}

int
v4l_capture_setup(struct encode *enc, int width, int height, int fps)
{
	char v4l_device[80], node[8];
	struct v4l2_format fmt = {0};
	struct v4l2_streamparm parm = {0};
	struct v4l2_requestbuffers req = {0};
	struct v4l2_control ctl;
	struct v4l2_crop crop;
	struct v4l2_dbg_chip_ident chip;
	struct v4l2_frmsizeenum fsize;
	int i, g_input = 1, mode = 0;

	if (cap_fd > 0) {
		warn_msg("capture device already opened\n");
		return -1;
	}

	sprintf(node, "%d", enc->cmdl->video_node_capture);
	strcpy(v4l_device, "/dev/video");
	strcat(v4l_device, node);

	if ((cap_fd = open(v4l_device, O_RDWR, 0)) < 0) {
		err_msg("Unable to open %s\n", v4l_device);
		return -1;
	}

	if (ioctl(cap_fd, VIDIOC_DBG_G_CHIP_IDENT, &chip)) {
		printf("VIDIOC_DBG_G_CHIP_IDENT failed.\n");
		return -1;
	}
	info_msg("sensor chip is %s\n", chip.match.name);

	memset(sizes_buf, 0, sizeof(sizes_buf));
	for (i = 0; i < MAX_CAPTURE_MODES; i++) {
		fsize.index = i;
		if (ioctl(cap_fd, VIDIOC_ENUM_FRAMESIZES, &fsize))
			break;
		else {
			sizes_buf[i][0] = fsize.discrete.width;
			sizes_buf[i][1] = fsize.discrete.height;
		}
	}

	ctl.id = V4L2_CID_PRIVATE_BASE;
	ctl.value = 0;
	if (ioctl(cap_fd, VIDIOC_S_CTRL, &ctl) < 0)
	{
		printf("set control failed\n");
		return -1;
	}

	mode = getCaptureMode(width, height);
	if (mode == -1) {
		printf("Not support the resolution in camera\n");
		return -1;
	}
	info_msg("sensor frame size is %dx%d\n", sizes_buf[mode][0],
					       sizes_buf[mode][1]);

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = fps;
	parm.parm.capture.capturemode = mode;
	if (ioctl(cap_fd, VIDIOC_S_PARM, &parm) < 0) {
		err_msg("set frame rate failed\n");
		close(cap_fd);
		cap_fd = -1;
		return -1;
	}

	if (ioctl(cap_fd, VIDIOC_S_INPUT, &g_input) < 0) {
		printf("VIDIOC_S_INPUT failed\n");
		close(cap_fd);
		return -1;
	}

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c.width = width;
	crop.c.height = height;
	crop.c.top = 0;
	crop.c.left = 0;
	if (ioctl(cap_fd, VIDIOC_S_CROP, &crop) < 0) {
		printf("VIDIOC_S_CROP failed\n");
		close(cap_fd);
		return -1;
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.bytesperline = width;
	fmt.fmt.pix.priv = 0;
	fmt.fmt.pix.sizeimage = 0;
	if (enc->cmdl->chromaInterleave == 0)
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	else
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	if (ioctl(cap_fd, VIDIOC_S_FMT, &fmt) < 0) {
		err_msg("set format failed\n");
		close(cap_fd);
		return -1;
	}

	memset(&req, 0, sizeof(req));
	req.count = TEST_BUFFER_NUM;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(cap_fd, VIDIOC_REQBUFS, &req) < 0) {
		err_msg("v4l_capture_setup: VIDIOC_REQBUFS failed\n");
		close(cap_fd);
		cap_fd = -1;
		return -1;
	}

	return 0;
}

//**************************变量*************************
unsigned char Frame_NUM_FPGA = 0;
unsigned char Alarm_Begin_Flag = 0;

extern unsigned char *gSubImageData;
extern unsigned char gSubImageData_QS[960*540*3/2];
extern unsigned short VPU_Camera_Resolution;
extern unsigned short VPU_Camera_Format;

extern unsigned char PAL_BeginTrack_flag ;

extern unsigned char IR_Track_Begin_Flag ;
extern unsigned char CCD_Track_Begin_Flag ;

extern unsigned short CCD_Target_x ,CCD_Target_y ;  //CCD高清 目标位置
extern unsigned short IR_Target_x ,IR_Target_y ;    //IR标清 目标位置
extern unsigned short KCF_Target_x,KCF_Target_y;
extern volatile unsigned char Frame_Capture;
extern volatile unsigned char Frame_Capture_OK;

unsigned char Picture_1080p[1920*1080*3/2] = {0};
unsigned char Picture_544p[960*544*3/2] = {0};
extern unsigned char gSubImageData_QS_544[960*544*3/2];

unsigned char Picture_576p[720*576*3/2] = {0};
unsigned char Picture_512p[640*512*3/2] = {0};

unsigned char gSubImageData_QS_1080[1920*1080*3/2] = {0};
unsigned char gSubImageData_KCF[720*576] = {0};
//*******************************************************

int
v4l_get_capture_data(struct v4l2_buffer *buf)
{
	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = V4L2_MEMORY_MMAP;
	if (ioctl(cap_fd, VIDIOC_DQBUF, buf) < 0) {
		err_msg("VIDIOC_DQBUF failed\n");
		return -1;
	}

	if(VPU_Camera_Resolution == 576)//PAL
	{
		memcpy(Picture_576p, cap_buffers[buf->index].start, 720*576*3/2);
		if(Frame_Capture == 1)
		{
			memcpy(gSubImageData_KCF, Picture_576p, 720*576);
			Frame_Capture_OK = 1;
		}
		//DisPlay_Target_Bomen(360, 288, Picture_576p, VPU_Camera_Resolution);

		//************************跟踪框***************************
		if((VPU_Camera_Format == 2)&&(PAL_BeginTrack_flag == 1))
			DisPlay_Target_Bomen(KCF_Target_x, KCF_Target_y, Picture_576p, VPU_Camera_Resolution);
		
		memcpy(cap_buffers[buf->index].start, Picture_576p, 720*576*3/2);
	}	
	else if(VPU_Camera_Resolution == 512) //640*512
	{
		memcpy(Picture_512p, cap_buffers[buf->index].start, 640*512*3/2);
		Frame_NUM_FPGA = Picture_512p[640];

		memcpy(gSubImageData, Picture_512p, 640*512);
		//************************搜索*****************************
		if(VPU_Camera_Format == 7)
			Alarm_Begin_Flag = 1;
		//************************跟踪框***************************
		if((VPU_Camera_Format == 2)&&(IR_Track_Begin_Flag == 1))
			DisPlay_Target_Bomen(IR_Target_x, IR_Target_y, Picture_512p, VPU_Camera_Resolution);
		//DisPlay_Target_Bomen(320, 256, Picture_512p, VPU_Camera_Resolution);
		memcpy(cap_buffers[buf->index].start, Picture_512p, 640*512*3/2);
	}
	else if(VPU_Camera_Resolution == 544) //960*544
	{
		memcpy(Picture_544p, cap_buffers[buf->index].start, 960*544*3/2);
		Frame_NUM_FPGA = Picture_544p[964];

		memcpy(gSubImageData_QS_544, Picture_544p, 960*544*3/2);
		//************************搜索*****************************
		if(VPU_Camera_Format == 7)
			Alarm_Begin_Flag = 1;
		//************************跟踪框***************************
		if((VPU_Camera_Format == 2)&&(CCD_Track_Begin_Flag == 1))
			DisPlay_Target_Bomen(CCD_Target_x, CCD_Target_y, Picture_544p, VPU_Camera_Resolution);

		memcpy(cap_buffers[buf->index].start, Picture_544p, 960*544*3/2);
	}
	else if(VPU_Camera_Resolution == 1080)
	{
		
		int x,y;
		memcpy(Picture_1080p, cap_buffers[buf->index].start, 1920*1080*3/2);
		//**********************将NV12 1080P压缩成960*540**********************
		/*
		unsigned long kkk = 0;
		for(y=0; y<1080; y+=2)
			for(x=0; x<1920; x+=2)
			{
				gSubImageData_QS_1080[kkk] = Picture_1080p[1920*y + x];
				kkk++;
			}			
		for(y=1080; y<1620; y+=2)
			for(x=0; x<1920; x+=4)
			{
				gSubImageData_QS_1080[kkk +1]   = Picture_1080p[y*1920 + x];      //UV调换了位置
				gSubImageData_QS_1080[kkk]      = Picture_1080p[y*1920 + x + 1];
				kkk+=2;
			}	
		*/
		memcpy(gSubImageData_QS_1080, Picture_1080p, 1920*1080*3/2);
		DisPlay_Target_Bomen(10, 10, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(960, 540, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(1900, 20, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(0, 1060, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(16, 1060, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(14, 100, gSubImageData_QS_1080, VPU_Camera_Resolution);
		DisPlay_Target_Bomen(216, 400, gSubImageData_QS_1080, VPU_Camera_Resolution);
		memcpy(cap_buffers[buf->index].start,gSubImageData_QS_1080, 1920*1080*3/2);
	}
	
	return 0;
}

void
v4l_put_capture_data(struct v4l2_buffer *buf)
{
	ioctl(cap_fd, VIDIOC_QBUF, buf);
}

