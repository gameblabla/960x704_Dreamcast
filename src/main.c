/* KallistiOS ##version##

   examples/dreamcast/pvr/bumpmap/bump.c
   Copyright (C) 2014 Lawrence Sebald

   This example demonstrates the use of bumpmaps on a surface.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <kos.h>

#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/fmath.h>
#include <dc/maple/controller.h>

#include <kmg/kmg.h>

static pvr_poly_hdr_t hdr;
static pvr_ptr_t txr;

#define NOTEXTURE 1
//#define TEST 1

static pvr_ptr_t Tex_to_VRAM(char const* filename, uint32_t fsize) {
	pvr_ptr_t rv;
	
	FILE* fp;
	fp = fopen(filename, "rb");
	if (!fp)
	{
		return 0;
	}
	
	rv = pvr_mem_malloc(fsize);
	fread(rv, 1, fsize, fp);
	fclose(fp);

	return rv;
}

void Texture_to_Framebuffer(char const* filename, uint32_t BYTESTOUPDATE)
{
	uint32_t* tmp_mem;
	
	FILE* fp;
	fp = fopen(filename, "rb");
	if (!fp)
	{
		return;
	}
	
	tmp_mem = memalign(BYTESTOUPDATE, 32);
	fread(tmp_mem, 1, BYTESTOUPDATE, fp);
	fclose(fp);
	
	dcache_flush_range((uint32_t)tmp_mem,BYTESTOUPDATE);
	while (!pvr_dma_ready());
	pvr_dma_transfer(tmp_mem, (uint32_t)vram_l, BYTESTOUPDATE,PVR_DMA_VRAM32,-1,NULL,0);
	
}

static pvr_ptr_t load_kmg(char const* filename, uint32_t* w, uint32_t* h) {
	kos_img_t img;
	pvr_ptr_t rv;

	if(kmg_to_img(filename, &img)) {
		printf("Failed to load image file: %s\n", filename);
		return NULL;
	}

	if(!(rv = pvr_mem_malloc(img.byte_count))) {
		printf("Couldn't allocate memory for texture!\n");
		kos_img_free(&img, 0);
		return NULL;
	}

	pvr_txr_load_kimg(&img, rv, 0);
	kos_img_free(&img, 0);

	*w = img.w;
	*h = img.h;

	return rv;
}

/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
	((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )
#define MIN(a, b) ( (a)<(b)? (a):(b) )

static int init() {
	pvr_poly_cxt_t cxt;
	
#ifdef TEST
	uint32 w = 512, h = 512;

	if(!(txr = Tex_to_VRAM("/cd/640.tex", 786432)))
#else
	uint32 w = 1024, h = 1024;

	if(!(txr = Tex_to_VRAM("/cd/example.tex", 264192)))
#endif
		return 1;

#ifdef TEST
	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED | PVR_TXRFMT_STRIDE | PVR_TXRFMT_VQ_DISABLE, w, h, txr, PVR_FILTER_NONE);
#else
	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_YUV422 | PVR_TXRFMT_TWIDDLED |  PVR_TXRFMT_VQ_ENABLE, w, h, txr, PVR_FILTER_NONE);
#endif
	
	pvr_poly_compile(&hdr, &cxt);

	return 0;
}


static void draw() {
#ifdef TEST
	float const x1 = 0, x2 = 640, y1 = 0, y2 = 480;
#else
	float const x1 = 0, x2 = 768, y1 = 0, y2 = 576;
#endif
    pvr_vertex_t vert = {
		.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f),
		.oargb = 0,
		.flags = PVR_CMD_VERTEX,
		.z = 1
	};

	pvr_wait_ready();
	pvr_scene_begin();

	pvr_list_begin(PVR_LIST_OP_POLY);
	pvr_prim(&hdr, sizeof(hdr));

    vert.x = x1;
    vert.y = y1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x2;
    vert.y = y1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x1;
    vert.y = y2;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x2;
    vert.y = y2;
    vert.u = 1.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

	pvr_list_finish();

	pvr_scene_finish();
}

vid_mode_t mycustommode = {
        .generic = 0,  // Replace 0 with appropriate value for generic mode type
        .width = 768,
        .height = 576,
        .flags = VID_INTERLACE | VID_PAL,
        .cable_type = CT_VGA,
        .pm = PM_RGB888P,  // Replace 0 with appropriate value for pixel mode
        .scanlines = 624,
        .clocks = 863,
        .bitmapx = 88,
        .bitmapy = 16,
        .scanint1 = 24,
        .scanint2 = 260,
        .borderx1 = 54,
        .borderx2 = 843,
        .bordery1 = 44,
        .bordery2 = 620,
        .fb_curr = 0,
        .fb_count = 1,
        .fb_size = (768*576)*3
};


vid_mode_t DM_800x600 = {
        .generic = 0,  // Replace 0 with appropriate value for generic mode type
        .width = 800,
        .height = 600,
        .flags = VID_PAL,
        .cable_type = CT_VGA,
        .pm = PM_RGB888P,  // Replace 0 with appropriate value for pixel mode
        .scanlines = 600,
        .clocks = 863,
        .bitmapx = 88+32,
        .bitmapy = 16,
        .scanint1 = 24,
        .scanint2 = 260,
        .borderx1 = 54+32,
        .borderx2 = 843+32,
        .bordery1 = 44,
        .bordery2 = 620,
        .fb_curr = 0,
        .fb_count = 1,
        .fb_size = (800*800)*3
};

typedef struct {
  // Keeps track of the horizontal framebuffer scale factor for reference
  // Since division is slow, multiply by this number to divide by the video scale
  float video_scale_multiplier;
  // This is the inverse of video_scale_multiplier and tracks by how much the output
  // image will be stretched from the framebuffer.
  float video_scale;

  // Current resolution (in pixels) and color depth
  uint32_t video_width;
  uint32_t video_height;
  // In Hz
  uint32_t video_refresh_rate;
  // RGB0555 = 0, RGB565 = 1, RGB888 = 2, RGB0888 = 3
  uint32_t video_color_type;

  // Current framebuffer resolution (in pixels) and color depth
  uint32_t fb_width;
  uint32_t fb_height;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t fb_color_bytes;
} VIDEO_PARAMS_STRUCT;

VIDEO_PARAMS_STRUCT STARTUP_video_params;

// 16-bit
#define FB_RGB0555 0
// 16-bit
#define FB_RGB565 1
// 24-bit
#define FB_RGB888 2
// 32-bit
#define FB_RGB0888 3


#define POWERVR_FB_RENDER_MODULO (*(volatile uint32_t*)0xa05f804c)

#define POWERVR_HPOS (*(volatile uint32_t*)0xa05f80ec) // STARTX
#define POWERVR_VPOS (*(volatile uint32_t*)0xa05f80f0) // STARTY

#define POWERVR_HPOS_IRQ (*(volatile uint32_t*)0xa05f80c8) // HBLANK_INT
#define POWERVR_VPOS_IRQ (*(volatile uint32_t*)0xa05f80cc) // VBLANK_INT

#define POWERVR_BORDER_COLOR (*(volatile uint32_t*)0xa05f8040)
#define POWERVR_FB_DISPLAY_SIZE  (*(volatile uint32_t*)0xa05f805c)

#define POWERVR_SYNC_WIDTH  (*(volatile uint32_t*)0xa05f80e0)

#define POWERVR_SYNC_CFG (*(volatile uint32_t*)0xa05f80d0)

#define POWERVR_HBORDER (*(volatile uint32_t*)0xa05f80d4) //HBLANK
#define POWERVR_SYNC_LOAD (*(volatile uint32_t*)0xa05f80d8)
#define POWERVR_VBORDER (*(volatile uint32_t*)0xa05f80dc) // VBLANK

void STARTUP_832x480_VGA_CVT_RBv2(uint8_t fbuffer_color_mode)
{
  // Set global video output mode parameters
  STARTUP_video_params.video_width = 832;
  STARTUP_video_params.video_height = 480;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 928 wide scaled to 909 wide (field)
  // 848 wide scaled to 832 wide (visible frame)
  // Perfect for the DC because the DC does 32x32 tiles.
  uint32_t horiz_active_area = 832;
  uint32_t vert_active_area = 480;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  int cable = vid_check_cable();

  if(cable == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    POWERVR_FB_RENDER_MODULO = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    POWERVR_BORDER_COLOR = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    POWERVR_FB_DISPLAY_SIZE = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    POWERVR_HPOS = 0x00000045; // ok: 69 ? mod: 68. CVT-RBv2: 72. Using 10 (no, 20).. Using 7 (no, 23). 168, default: 0x000000a8 (horiz)
	POWERVR_VPOS = 0x000e000e; // ok: 14 & 14 ? new: 11 & 11.. CVT-RBv2: 14 & 14. Using 31 & 31.. 40 & 40, default: 0x00280028 (vert)
    POWERVR_HPOS_IRQ = 0x03850000; // ok: 901 ? mod: 900. CVT-RBv2: 920. Using 858 (no, 868; nah, 871).. Using 855. 837, default: 0x03450000 (horiz)
    POWERVR_VPOS_IRQ = 0x000e01ee; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 21, 511.. Using 21, 512. 21, 520, default: 0x00150208 (vert)
	POWERVR_SYNC_CFG = 0x00000100;
    POWERVR_HBORDER = 0x00450385; // ok: 69, 901 ? mod: 68, 900. CVT-RBv2: 72, 920. Using 10, 858 (no, 20, 868; nah, 23, 871).. Using 7, 855. 126, 837, default: 0x007e0345 (horiz)
    POWERVR_SYNC_LOAD = 0x01ee038c; // ok: 908x494 (909x495) ? new: 906x493 (907x494).. mod: 906x494 (907x495) CVT-RBv2: 927x494 (928x495). May need to fudge with horiz scale for 27MHz. VESA: 1088x517, our max is 870x516 (871x517). 857x524 (858x525), default: 0x020c0359
    POWERVR_VBORDER = 0x000e01ee; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 31, 511.. Using 40, 512. 40, 520, default: 0x00280208 (vert)
    POWERVR_SYNC_WIDTH = 0x01f6d81e; // ok: 7 (8,- 1), 877 (909 - 31,- 1), 8 (CVT), 30 (31,- 1) ? new: 7, 875 (907 - 31 - 1), 5 (for 16:9), 30.. mod: 7, 875 (907 - 31 - 1), 8, 30. CVT-RBv2: 15, 895 (928 - 32 - 1), 8, 31. Using 15, 854, 8, 15.. Using 15, 854, 3, 15. 15, 793, 3, 63, default: 0x03f1933f

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}

void STARTUP_720p_VGA_PVR(uint8_t fbuffer_color_mode)
{
  // Set global scale factors
  STARTUP_video_params.video_scale = 1280.0f / 1280.0f;
  STARTUP_video_params.video_scale_multiplier = 1280.0f / 1280.0f;

  // Set global video output mode parameters
  STARTUP_video_params.video_width = 1280;
  STARTUP_video_params.video_height = 720;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 1664 wide scaled to 603 wide (field)
  // 1280 wide scaled to 464 wide (visible frame)
  uint32_t horiz_active_area = 1280;
  uint32_t vert_active_area = 720;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  int cable = vid_check_cable();

  if(cable == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    *(volatile uint32_t*)0xa05f804c = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    *(volatile uint32_t*)0xa05f8040 = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    *(volatile uint32_t*)0xa05f805c = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    *(volatile uint32_t*)0xa05f80ec = 0x00000074; // ok: 116 ? 168, default: 0x000000a8 (horiz)
    *(volatile uint32_t*)0xa05f80f0 = 0x00190019; // ok: 25 & 25 ? 40 & 40, default: 0x00280028 (vert)
    *(volatile uint32_t*)0xa05f80c8 = 0x02440000; // ok: 580 ? 837, default: 0x03450000 (horiz)
    *(volatile uint32_t*)0xa05f80cc = 0x001902e9; // ok: 25, 745 ? 21, 520, default: 0x00150208 (vert)
    *(volatile uint32_t*)0xa05f80d0 = 0x00000100;
    *(volatile uint32_t*)0xa05f80d4 = 0x00740244; // ok: 116, 580 ? 126, 837, default: 0x007e0345 (horiz)
    *(volatile uint32_t*)0xa05f80d8 = 0x02eb025a; // ok: 602x747 (603x748) ? 857x524 (858x525), default: 0x020c0359
    *(volatile uint32_t*)0xa05f80dc = 0x001902e9; // ok: 25, 745 ? 40, 520, default: 0x00280208 (vert)
    *(volatile uint32_t*)0xa05f80e0 = 0x02e2c52d; // ok: 11 (12,- 1), 556 (603 - 46,- 1), 5 (16:9), 45 (46,- 1) ? 15, 793, 3, 63, default: 0x03f1933f

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}


void MOD_STARTUP_832x704_VGA_CVT_RBv2(uint8_t fbuffer_color_mode)
{
  // Set global video output mode parameters
  STARTUP_video_params.video_width = 832;
  STARTUP_video_params.video_height = 704;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 928 wide scaled to 909 wide (field)
  // 848 wide scaled to 832 wide (visible frame)
  // Perfect for the DC because the DC does 32x32 tiles.
  uint32_t horiz_active_area = 832;
  uint32_t vert_active_area = 704;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  int cable = vid_check_cable();

  if(cable == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    POWERVR_FB_RENDER_MODULO = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    POWERVR_BORDER_COLOR = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    POWERVR_FB_DISPLAY_SIZE = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    POWERVR_HPOS = 0x00000045; // ok: 69 ? mod: 68. CVT-RBv2: 72. Using 10 (no, 20).. Using 7 (no, 23). 168, default: 0x000000a8 (horiz)
    POWERVR_VPOS = 0x00190019; // ok: 14 & 14 ? new: 11 & 11.. CVT-RBv2: 14 & 14. Using 31 & 31.. 40 & 40, default: 0x00280028 (vert)
    POWERVR_HPOS_IRQ = 0x03850000; // ok: 901 ? mod: 900. CVT-RBv2: 920. Using 858 (no, 868; nah, 871).. Using 855. 837, default: 0x03450000 (horiz)
    POWERVR_VPOS_IRQ = 0x001902eb; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 21, 511.. Using 21, 512. 21, 520, default: 0x00150208 (vert)
    POWERVR_SYNC_CFG = 0x00000100;
    POWERVR_HBORDER = 0x00450385; // ok: 69, 901 ? mod: 68, 900. CVT-RBv2: 72, 920. Using 10, 858 (no, 20, 868; nah, 23, 871).. Using 7, 855. 126, 837, default: 0x007e0345 (horiz)
    POWERVR_SYNC_LOAD = 0x02eb038c; // ok: 908x494 (909x495) ? new: 906x493 (907x494).. mod: 906x494 (907x495) CVT-RBv2: 927x494 (928x495). May need to fudge with horiz scale for 27MHz. VESA: 1088x517, our max is 870x516 (871x517). 857x524 (858x525), default: 0x020c0359
    POWERVR_VBORDER = 0x001902eb; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 31, 511.. Using 40, 512. 40, 520, default: 0x00280208 (vert)
    POWERVR_SYNC_WIDTH = 0x02e2d81e; // ok: 7 (8,- 1), 877 (909 - 31,- 1), 8 (CVT), 30 (31,- 1) ? new: 7, 875 (907 - 31 - 1), 5 (for 16:9), 30.. mod: 7, 875 (907 - 31 - 1), 8, 30. CVT-RBv2: 15, 895 (928 - 32 - 1), 8, 31. Using 15, 854, 8, 15.. Using 15, 854, 3, 15. 15, 793, 3, 63, default: 0x03f1933f

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}



void MOD_STARTUP_960x704_VGA_CVT_RBv2(uint8_t fbuffer_color_mode)
{
  // Set global video output mode parameters
  STARTUP_video_params.video_width = 960;
  STARTUP_video_params.video_height = 704;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 1360 wide scaled to 1280/1360 wide (field)
  // 960 wide scaled to 960 wide (visible frame)
  // Perfect for the DC because the DC does 32x32 tiles.
  uint32_t horiz_active_area = 960;
  uint32_t vert_active_area = 704;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  int cable = vid_check_cable();

  if(cable == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    POWERVR_FB_RENDER_MODULO = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    POWERVR_BORDER_COLOR = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    POWERVR_FB_DISPLAY_SIZE = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    POWERVR_HPOS = 0x0000003F; // ok: 63
    POWERVR_HPOS_IRQ = 0x03FF0000; // ok: 1023
    POWERVR_HBORDER = 0x003F03FF; // ok: 63, 1008 

    POWERVR_SYNC_LOAD = 0x02eb03fF; // 1024x704
    POWERVR_SYNC_WIDTH = 0x02e2d30f; // ok: 7 (8,- 1), 877 (909 - 31,- 1), 3 (DMT legacy), 15 (16,- 1)
    
    POWERVR_VPOS = 0x00190019; // ok: 14 & 14 ? new: 11 & 11.. CVT-RBv2: 14 & 14. Using 31 & 31.. 40 & 40, default: 0x00280028 (vert)
    POWERVR_VPOS_IRQ = 0x001902eb; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 21, 511.. Using 21, 512. 21, 520, default: 0x00150208 (vert)
    POWERVR_SYNC_CFG = 0x00000100;
    POWERVR_VBORDER = 0x001902eb; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 31, 511.. Using 40, 512. 40, 520, default: 0x00280208 (vert)
    

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}


void MOD_STARTUP_960x720_VGA_CVT_RBv2(uint8_t fbuffer_color_mode)
{
  // Set global video output mode parameters
  STARTUP_video_params.video_width = 960;
  STARTUP_video_params.video_height = 720;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 1360 wide scaled to 1280/1360 wide (field)
  // 960 wide scaled to 960 wide (visible frame)
  // Perfect for the DC because the DC does 32x32 tiles.
  uint32_t horiz_active_area = 960;
  uint32_t vert_active_area = 720;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  int cable = vid_check_cable();

  if(cable == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    POWERVR_FB_RENDER_MODULO = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    POWERVR_BORDER_COLOR = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    POWERVR_FB_DISPLAY_SIZE = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    POWERVR_HPOS = 0x0000003F; // ok: 63
    POWERVR_HPOS_IRQ = 0x03FF0000; // ok: 1023
    POWERVR_HBORDER = 0x003F03FF; // ok: 63, 1008 

    POWERVR_SYNC_LOAD = 0x02ed03fF; // 1024x749
    POWERVR_SYNC_WIDTH = 0x02e2d30f; // ok: 7 (8,- 1), 877 (909 - 31,- 1), 3 (DMT legacy), 15 (16,- 1)
    
    POWERVR_VPOS = 0x00190019; // ok: 14 & 14 ? new: 11 & 11.. CVT-RBv2: 14 & 14. Using 31 & 31.. 40 & 40, default: 0x00280028 (vert)
    POWERVR_VPOS_IRQ = 0x001902e9; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 21, 511.. Using 21, 512. 21, 520, default: 0x00150208 (vert)
    POWERVR_SYNC_CFG = 0x00000100;
    POWERVR_VBORDER = 0x001902e9; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 31, 511.. Using 40, 512. 40, 520, default: 0x00280208 (vert)
    

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}

void MOD_STARTUP_960x960_VGA_CVT_RBv2(uint8_t fbuffer_color_mode)
{
  // Set global video output mode parameters
  STARTUP_video_params.video_width = 960;
  STARTUP_video_params.video_height = 960;
  STARTUP_video_params.video_color_type = fbuffer_color_mode;
  STARTUP_video_params.video_refresh_rate = 60;

  // 1360 wide scaled to 1280/1360 wide (field)
  // 960 wide scaled to 960 wide (visible frame)
  // Perfect for the DC because the DC does 32x32 tiles.
  uint32_t horiz_active_area = 960;
  uint32_t vert_active_area = 960;
  // {RGB0555, RGB565} = 2Bpp, {RGB888} = 3Bpp, {RGB0888} = 4Bpp
  uint32_t bpp_mode_size = fbuffer_color_mode + 1 + (0x1 ^ ((fbuffer_color_mode & 0x1) | (fbuffer_color_mode >> 1))); // Add another 1 only if 0b00

  // Set global framebuffer parameters
  STARTUP_video_params.fb_width = horiz_active_area;
  STARTUP_video_params.fb_height = vert_active_area;
  STARTUP_video_params.fb_color_bytes = bpp_mode_size;

  if(vid_check_cable() == CT_VGA)
  {
    *(volatile uint32_t*)0xa05f80e8 = 0x00160008;
    *(volatile uint32_t*)0xa05f8044 = 0x00800000 | (fbuffer_color_mode << 2);

    POWERVR_FB_RENDER_MODULO = (horiz_active_area * bpp_mode_size) / 8; // for PVR to know active area width
    POWERVR_BORDER_COLOR = 0x00000000; // Border color in RGB0888 format (this mode has no border)
    POWERVR_FB_DISPLAY_SIZE = (1 << 20) | ((vert_active_area - 1) << 10) | (((horiz_active_area * bpp_mode_size) / 4) - 1); // progressive scan has a 1 since no lines are skipped

    POWERVR_HPOS = 0x0000003F; // ok: 63
    POWERVR_HPOS_IRQ = 0x03FF0000; // ok: 1023
    POWERVR_HBORDER = 0x003F03FF; // ok: 63, 1023

    POWERVR_SYNC_LOAD = 0x03e703FF; // 1024x999
    POWERVR_SYNC_WIDTH = 0x029a530f; // ok: 2, 421, 3 (DMT legacy), 15 (16,- 1)
    
    POWERVR_VPOS = 0x00270027; // ok: 14 & 14 ? new: 11 & 11.. CVT-RBv2: 14 & 14. Using 31 & 31.. 40 & 40, default: 0x00280028 (vert)
    POWERVR_VPOS_IRQ = 0x002703e7; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 21, 511.. Using 21, 512. 21, 520, default: 0x00150208 (vert)
    POWERVR_VBORDER = 0x002703e7; // ok: 14, 494 ? new: 11, 491.. CVT-RBv2: 14, 494. Using 31, 511.. Using 40, 512. 40, 520, default: 0x00280208 (vert)
    
	POWERVR_SYNC_CFG = 0x00000100;

    uint32_t scan_area_size = horiz_active_area * vert_active_area;
    uint32_t scan_area_size_bytes = scan_area_size * bpp_mode_size; // This will always be divisible by 4

    // Reset framebuffer address
    *(volatile uint32_t*)0xa05f8050 = 0x00000000; // BootROM sets this to 0x00200000 (framebuffer base is 0xa5000000 + this)
    *(volatile uint32_t*)0xa05f8054 = 0x00000000; // Same for progressive, resetting the offset gets us 2MB VRAM back after BootROM is done with it

    // zero out framebuffer area
    for(uint32_t pixel_or_two = 0; pixel_or_two < scan_area_size_bytes; pixel_or_two += 4)
    {
      *(uint32_t*)(0xa5000000 + pixel_or_two) = 0;
    }

    // re-enable video
    *(volatile uint32_t*)0xa05f80e8 &= ~8;
    *(volatile uint32_t*)0xa05f8044 |= 1;
  }
}


void pvr_setup()
{
	pvr_init_params_t params = {
		/* Enable Opaque, Translucent, and Punch-Thru polygons with binsize 16 */
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0,
		PVR_BINSIZE_16 },
		
		/* Vertex buffer size 128K */
		128*1024,
		
		0, // Disable Vertex DMA
		
		0, // Disable FSAA
		
		0
	};

    /* init kos  */
#ifdef TEST
    vid_set_mode(DM_640x480, PM_RGB565);
#elif defined(NOTEXTURE)
    //vid_set_mode_ex(&mycustommode);
	printf("RGB888\n");
    //STARTUP_832x480_VGA_CVT_RBv2(FB_RGB888);
    //MOD_STARTUP_832x704_VGA_CVT_RBv2(FB_RGB888);
    MOD_STARTUP_960x704_VGA_CVT_RBv2(FB_RGB888);
#else
    vid_set_mode(DM_640x480, PM_RGB565);
#endif


	pvr_init(&params);
    pvr_dma_init();
}

int main(int argc, char *argv[]) {
	//pvr_init_defaults();
	pvr_setup();
	
#ifdef NOTEXTURE
	Texture_to_Framebuffer("/cd/43_960x704.rgb",
	/* 884736*/ 
	/*798720*/ 
	2027520
	/*1198080*/
	/*1221120*/
	/*1465344*/
	
	/*1327104*/ 
	/*1440000*/ 
	/*1769472*/);

	while(1)
	{
		vid_waitvbl();
	}
#else
	if(init()) {
		puts("Init error.");
		return 1;
	}

	for(;;) {
		draw();
	}

	pvr_mem_free(txr);
#endif
	return 0;
}
