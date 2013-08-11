#ifndef TIF_H_
#define TIF_H_

#include <tiffio.h>
#include "sys_config.h"

typedef struct tiff_info{
        unsigned short type;    /* little or big endian */
        int width;
        int length;
        int depth;
        unsigned short photo_metric;
        uint32 spp;     // sample per pixel, 16 bits per pixel for ANL camera
        uint16 bps;     // bit per sample, default is 1
        int line_size;
        int image_size;
        uint16 config;
	int page_num;
} tiff_info;
static struct tiff_info *info;

TIFF *tif;
TIFF *op_tif;

void *tif_info(char **argv);
int tif_multi_load(uint16 *image);
int tif_single_load(uint16 *image);
void tif_release(uint16 *image);

int tif_single_syn(struct image_info_type *image_info, uint16 *image, char *filename);
int tif_multi_syn(struct image_info_type *image_info, uint16 *image, char *filename);

#endif /* tif.h */
