
#include "tif.h"

/*
 *
 *
 */

void *tif_info(char **argv)
{
	int dircount = 0;
	int bound;	
	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));	
	
	tif = TIFFOpen(argv[0], "r");
	if(tif == NULL){
		fprintf(stderr, "ERROR: Could not open input image!\n");
		exit(1);
	}
	printf("-- opening tiff file...\n");

	do {
		dircount++;
	} while (TIFFReadDirectory(tif));
	printf("-- %d images in %s\n", dircount, argv[0]);
	
	/* global variable */
	image_info->page_num = dircount;
	
	// input image paramters
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &info->length);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &info->width);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &info->bps);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &info->spp);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &info->photo_metric);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &info->config);
	
	/* global variable */
	image_info->buffer_length = info->length/NUM_PROCESS_THREADS;
	image_info->buffer_width = info->width;
	image_info->buffer_size = image_info->buffer_length * image_info->buffer_width;
	image_info->image_size = info->length * info->width;
	image_info->length = info->length;
	image_info->width = info->width;
		
	printf("-- length = %d, width = %d\n", info->length, info->width);
	printf("-- bit per sample = %d\n", info->bps);
	printf("-- sample per pixel = %d\n", info->spp);
	printf("-- photo metirc = %d\n", info->photo_metric);
	printf("-- planar config = %d\n", info->config);
	
	printf("-- Image is splitted into %d blocks for multiple threads, each block has %d lines\n", NUM_PROCESS_THREADS, image_info->buffer_length);

	info->line_size = TIFFScanlineSize(tif);
	info->image_size = info->line_size * info->length;

	if(info->spp != 1){
		info->spp = 1;
		printf("-- Warnning:sample per pixel value automatically set to 1!\n");
	}

	return (void *)info;
	
}

/* tif load function */
int tif_load(uint16 *image)
{
	int r, c;	// height index, width index
	uint16 s;
	uint16 *scanline;
	long int image_offset;
	
	if((scanline = (uint16 *)_TIFFmalloc(info->line_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the scan buffer!\n");
		exit(42);
	}
	
	image_offset = 0;	
	printf("-- loading tif files ... \n");		

	TIFFSetDirectory(tif, 0);

	do {
		if (info->config == PLANARCONFIG_CONTIG){
			for(r = 0; r < info->length; r++){
				TIFFReadScanline(tif, scanline, r, s);

				for(c = 0; c < info->width; c++)
				{		
					image[image_offset + info->width * r + c] = *(scanline + c);
				}

			}
		} else if (info->config == PLANARCONFIG_SEPARATE){
			for(s = 0; s < info->spp; s++){
				for(r = 0; r < info->length; r++){
					TIFFReadScanline(tif, scanline, r, s);
					for(c = 0; c < info->width; c++)
					{
						image[image_offset + info->width * r + c] = *(scanline + c);
					}

				}
			}
		}
		image_offset += info->image_size/sizeof(uint16);
	} while (TIFFReadDirectory(tif));

	_TIFFfree(scanline);
	
	TIFFClose(tif);

	return 0;
}

int tif_syn(uint16 *image, char *filename){
	TIFF *op_tif;
	int r, c;	// height index, width index
	uint16 s;

	uint16 *scanline;
		
	if((op_tif = TIFFOpen(filename, "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}	
	
	printf("-- setting %s tif parameters\n", filename);
	TIFFSetField(op_tif, TIFFTAG_IMAGELENGTH, info->length);
	TIFFSetField(op_tif, TIFFTAG_IMAGEWIDTH, info->width);
	TIFFSetField(op_tif, TIFFTAG_BITSPERSAMPLE, info->bps);
	TIFFSetField(op_tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
	TIFFSetField(op_tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
	TIFFSetField(op_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		
	if((scanline = (uint16 *)_TIFFmalloc(image_info->width * image_info->length * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}
	
	printf("-- writing %s images...\n", filename);
	
	for (r = 0; r < info->length; r++){
		for (c = 0; c < info->width; c++){	
			*(scanline + c) = image[info->width * r + c];
		}
		TIFFWriteScanline(op_tif, scanline, r, s);
	}

	printf("-- writing %s image done!\n", filename);
	
	_TIFFfree(scanline);
	TIFFClose(op_tif);
}

int tif_multi_syn(uint16 *image, char *filename)
{
	TIFF *op_tif;
	int image_offset;
	int r, c;	// height index, width index
	int np;
	uint16 s;

	uint16 *scanline;

	if((op_tif = TIFFOpen(filename, "w")) == NULL){
                printf("Open output tif file for writing failed!\n");
                exit(0);
        }

        printf("-- writing %s tif images...\n", filename);
	
	image_offset = 0;
	
	TIFFSetField(op_tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	TIFFSetField(op_tif, TIFFTAG_IMAGELENGTH, image_info->length);
	TIFFSetField(op_tif, TIFFTAG_IMAGEWIDTH, image_info->width);
	TIFFSetField(op_tif, TIFFTAG_BITSPERSAMPLE, 16);
	TIFFSetField(op_tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(op_tif, TIFFTAG_PHOTOMETRIC, 1);
	TIFFSetField(op_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(op_tif, TIFFTAG_PAGENUMBER, np, image_info->page_num);

	if((scanline = (uint16 *)_TIFFmalloc(info->width * info->length * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}

	for (np = 0; np < image_info->page_num; np++){
		TIFFSetField(op_tif, TIFFTAG_PAGENUMBER, np, image_info->page_num);
		
		for (r = 0; r < info->length; r++){
			for (c = 0; c < info->width; c++){
				*(scanline + c) = image[image_offset + info->width * r + c];
			}
			TIFFWriteScanline(op_tif, scanline, r, s);
		}
		image_offset += info->image_size/sizeof(uint16);
		TIFFWriteDirectory(op_tif);
	}
	
	printf("-- writing %s image done!\n", filename);
	_TIFFfree(scanline);
	TIFFClose(op_tif);

	return 0;
}

void tif_release(uint16 *image)
{
	_TIFFfree(image);
	free(info);
}

