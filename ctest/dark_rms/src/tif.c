
#include "../include/tif.h"

struct tiff_info *info;

/* tif load function */
int tif_load(char **argv)
{
	int r, c;	// height index, width index
	uint16 s;
	int dircount = 0;
	int bound;	
	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));
	
	uint32 data_size;
	unsigned long image_offset;
	uint16 *scanline;	
	TIFF *tif;
	
	tif = TIFFOpen(argv[0], "r");
	if(tif == NULL){
		fprintf(stderr, "ERROR: Could not open input image!\n");
		exit(1);
	}
	printf("\topening tiff file...\n");

	do {
		dircount++;
	} while (TIFFReadDirectory(tif));
	printf("\t%d images in %s\n", dircount, argv[0]);
	
	/* global variable */
	pages = dircount;
	
	// input image paramters
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &info->length);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &info->width);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &info->bps);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &info->spp);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &info->photo_metric);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &info->config);

	/* here to set image size
	info->length = LENGTH;
	info->width = WIDTH;
	*/
	
	/* global variable */
	buffer_length = info->length/NUM_PROCESS_THREADS;
	buffer_width = info->width;
	page_size = info->length * info->width;
	
	printf("\tlength = %d, width = %d\n", info->length, info->width);
	printf("\tbit per sample = %d\n", info->bps);
	printf("\tsample per pixel = %d\n", info->spp);
	printf("\tphoto metirc = %d\n", info->photo_metric);
	printf("\tplanar config = %d\n", info->config);
	
	printf("\tImage is splitted into %d blocks for multiple threads, each block has %d lines\n", NUM_PROCESS_THREADS, buffer_length);

	info->line_size = TIFFScanlineSize(tif);
	info->image_size = info->line_size * info->length;
	data_size = info->image_size * dircount;	

	if(info->spp != 1){
		info->spp = 1;
		printf("\tWarnning:sample per pixel value automatically set to 1!\n");
	}
	
	if((input_image = (uint16 *)_TIFFmalloc(data_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the uncompressed image!\n");
		exit(42);
	}

	if((scanline = (uint16 *)_TIFFmalloc(info->line_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the scan buffer!\n");
		exit(42);
	}
	
	image_offset = 0;
	printf("\tthe line size is %d\n", info->line_size);
	printf("\tthe image size is %d\n", info->image_size);
	printf("\timage offset is %ld\n", image_offset);
	printf("\tthe total sequence image size is %d\n", data_size);
	printf("\tloading tif files ... \n");		

	TIFFSetDirectory(tif, 0);
	
	do {
		if (info->config == PLANARCONFIG_CONTIG){
			for(r = 0; r < info->length; r++){
				TIFFReadScanline(tif, scanline, r, s);
			
				for(c = 0; c < info->width; c++)
				{		
					input_image[image_offset + info->width * r + c] = *(scanline + c);
				}

			}
		} else if (info->config == PLANARCONFIG_SEPARATE){
			for(s = 0; s < info->spp; s++){
				for(r = 0; r < info->length; r++){
					TIFFReadScanline(tif, scanline, r, s);
					for(c = 0; c < info->width; c++)
	                        	{
        	                        	input_image[image_offset + info->width * r + c] = *(scanline + c);
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

int tif_syn(){
	int r, c;	// height index, width index
	uint16 s;

	uint16 *scanline;

	TIFF *tif;
		
	if((tif = TIFFOpen(output_filename[0], "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}	
	
	printf("\tsetting %s tif parameters\n", output_filename[0]);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, info->length);
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, info->width);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, info->bps);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	
	if((scanline = (uint16 *)_TIFFmalloc(info->width * info->length * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}
	
	printf("\twriting %s images...\n", output_filename[0]);
	
	for (r = 0; r < info->length; r++){
		for (c = 0; c < info->width; c++){	
			*(scanline + c) = output_image_avg[info->width * r + c];
		}
		TIFFWriteScanline(tif, scanline, r, s);
	}

	printf("\twriting %s image done!\n", output_filename[0]);
	_TIFFfree(scanline);
	TIFFClose(tif);

	if((tif = TIFFOpen(output_filename[1], "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}

	printf("\tsetting %s parameters\n", output_filename[1]);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, info->length);
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, info->width);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, info->bps);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	if((scanline = (uint16 *)_TIFFmalloc(info->width * info->length * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}

	printf("\twriting %s tif images...\n", output_filename[1]);

	for (r = 0; r < info->length; r++){
		for (c = 0; c < info->width; c++){
			*(scanline + c) = output_image_std[info->width * r + c];
		}
		TIFFWriteScanline(tif, scanline, r, s);
	}

	printf("\twriting %s image done!\n", output_filename[1]);
	_TIFFfree(scanline);
	TIFFClose(tif);

	return 0;
}

void tif_release(uint16 *image)
{
	_TIFFfree(image);
}

