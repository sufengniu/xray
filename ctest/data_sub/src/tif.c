
#include "../include/tif.h"

/*
 *
 *
 */

struct tiff_info *info;

int tif_info(char **argv)
{
	int dircount = 0;
	int bound;	
	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));	
	
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
	
	/* global variable */
	buffer_length = info->length/NUM_PROCESS_THREADS;
	buffer_width = info->width;
	buffer_size = buffer_length * buffer_width;
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
	
}

/* tif load function */
int tif_load(uint16 *image)
{
	int r, c;	// height index, width index
	uint16 s;
	uint16 *scanline;
		
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

int tif_syn(){
	int r, c;	// height index, width index
	uint16 s;
	int np;

	uint16 *scanline;

	TIFF *avg_tif, *rms_tif, *sub_tif;
		
	if((avg_tif = TIFFOpen(output_filename[0], "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}	
	
	printf("\tsetting %s tif parameters\n", output_filename[0]);
	TIFFSetField(avg_tif, TIFFTAG_IMAGELENGTH, info->length);
	TIFFSetField(avg_tif, TIFFTAG_IMAGEWIDTH, info->width);
	TIFFSetField(avg_tif, TIFFTAG_BITSPERSAMPLE, info->bps);
	TIFFSetField(avg_tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
	TIFFSetField(avg_tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
	TIFFSetField(avg_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	
	if((scanline = (uint16 *)_TIFFmalloc(info->width * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}
	
	printf("\twriting %s images...\n", output_filename[0]);
	
	for (r = 0; r < info->length; r++){
		for (c = 0; c < info->width; c++){	
			*(scanline + c) = output_image_avg[info->width * r + c];
		}
		TIFFWriteScanline(avg_tif, scanline, r, s);
	}

	printf("\twriting %s image done!\n", output_filename[0]);
	TIFFClose(avg_tif);

	if((rms_tif = TIFFOpen(output_filename[1], "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}

	printf("\tsetting %s parameters\n", output_filename[1]);
	TIFFSetField(rms_tif, TIFFTAG_IMAGELENGTH, info->length);
	TIFFSetField(rms_tif, TIFFTAG_IMAGEWIDTH, info->width);
	TIFFSetField(rms_tif, TIFFTAG_BITSPERSAMPLE, info->bps);
	TIFFSetField(rms_tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
	TIFFSetField(rms_tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
	TIFFSetField(rms_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	printf("\twriting %s tif images...\n", output_filename[1]);

	for (r = 0; r < info->length; r++){
		for (c = 0; c < info->width; c++){
			*(scanline + c) = output_image_std[info->width * r + c];
		}
		TIFFWriteScanline(rms_tif, scanline, r, s);
	}

	printf("\twriting %s image done!\n", output_filename[1]);
	TIFFClose(rms_tif);

	// under testing
	if((sub_tif = TIFFOpen(output_filename[2], "w")) == NULL){
                printf("Open output tif file for writing failed!\n");
                exit(0);
        }

        printf("\twriting %s tif images...\n", output_filename[2]);
	
	image_offset = 0;
	for (np = 0; np < pages; np++){
		TIFFSetField(sub_tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
		TIFFSetField(sub_tif, TIFFTAG_IMAGELENGTH, info->length);
		TIFFSetField(sub_tif, TIFFTAG_IMAGEWIDTH, info->width);
		TIFFSetField(sub_tif, TIFFTAG_BITSPERSAMPLE, info->bps);
		TIFFSetField(sub_tif, TIFFTAG_SAMPLESPERPIXEL, info->spp);
		TIFFSetField(sub_tif, TIFFTAG_PHOTOMETRIC, info->photo_metric);
		TIFFSetField(sub_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(sub_tif, TIFFTAG_PAGENUMBER, np, pages);
		
		for (r = 0; r < info->length; r++){
			for (c = 0; c < info->width; c++){
				*(scanline + c) = res_image[image_offset + info->width * r + c];
			}
			TIFFWriteScanline(sub_tif, scanline, r, s);
		}
		image_offset += info->image_size/sizeof(uint16);
		TIFFWriteDirectory(sub_tif);
	}
	
	printf("\twriting %s image done!\n", output_filename[2]);
	_TIFFfree(scanline);
	TIFFClose(sub_tif);

	return 0;
}

void tif_release(uint16 *image)
{
	_TIFFfree(image);
}

