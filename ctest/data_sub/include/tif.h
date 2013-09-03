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
} tiff_info;

TIFF *tif;
uint32 data_size;
unsigned long image_offset;

int tif_info(char **argv);
int tif_load();
void tif_release(uint16 *image);
int tif_syn();

#endif /* tif.h */
