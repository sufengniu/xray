
#include "tif.h"

/* Description: This is tif read and tif write the API are shown below
 * tif_info:		load the tif image parameters for external function 
 *			memory allocation
 *			input: pointer of image on disk
 *			output: image parameters as struct tiff_info format
 * tif_single_load:	load a single tif images
 *			input: allocated memory space starting addr
 * tif_multi_load:	load multiple tif images at one time
 *			input: allocated memory space starting addr
 * tif_single_syn: 	synthesis multiple chunks from each threads together
 *			and write it into the disk
 *	 		input: TIFF, tif image parameters, synthesised image 
 *			starting addr, output file name
 * 			output: synthesised image is written on the disk
 * tif_multi_syn:	syntesis mutliple chunks from each threads and 
 * 			write it into the disk, the image on disk is multiple
 *			images
 * 			input: same as tif_single_syn
 * 			output: same as tif_multi_syn
 * tif_release:	free tif memory space
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
	
	info->page_num = dircount;
	
	// input image paramters
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &info->length);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &info->width);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &info->bps);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &info->spp);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &info->photo_metric);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &info->config);
	
	printf("-- length = %d, width = %d\n", info->length, info->width);
	printf("-- bit per sample = %d\n", info->bps);
	printf("-- sample per pixel = %d\n", info->spp);
	printf("-- photo metirc = %d\n", info->photo_metric);
	printf("-- planar config = %d\n", info->config);
	
	info->line_size = TIFFScanlineSize(tif);
	info->image_size = info->line_size * info->length;

	if(info->spp != 1){
		info->spp = 1;
		printf("-- Warnning:sample per pixel value automatically set to 1!\n");
	}

	return (void *)info;
	
}

/* single tif load function */
int tif_single_load(uint16 *image)
{
	int r, c;       // height index, width index
	uint16 s;
	uint16 *scanline;
	long int image_offset;

	if((scanline = (uint16 *)_TIFFmalloc(info->line_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the scan buffer!\n");
		exit(42);
	}

	image_offset = 0;
	printf("-- loading tif files ... \n");

	if (info->config == PLANARCONFIG_CONTIG){
		for(r = 0; r < info->length; r++){
			TIFFReadScanline(tif, scanline, r, s);

			for(c = 0; c < info->width; c++)
			{
				image[info->width * r + c] = *(scanline + c);
			}

		}
	} else if (info->config == PLANARCONFIG_SEPARATE){
		for(s = 0; s < info->spp; s++){
			for(r = 0; r < info->length; r++){
				TIFFReadScanline(tif, scanline, r, s);
				for(c = 0; c < info->width; c++)
				{
					image[info->width * r + c] = *(scanline + c);
				}
			}
		}
	}

	_TIFFfree(scanline);
	
	free(info);
	TIFFClose(tif);
	
	return 0;
}

/* multiple tif load function */
int tif_multi_load(uint16 *image)
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
	
	free(info);
	TIFFClose(tif);

	return 0;
}

int tif_single_syn(struct image_info_type *image_info, uint16 *image, char *filename){
	int r, c;	// height index, width index
	uint16 s;

	uint16 *scanline;
		
	if((op_tif = TIFFOpen(filename, "w")) == NULL){
		printf("Open output tif file for writing failed!\n");
		exit(0);
	}	
			
	TIFFSetField(op_tif, TIFFTAG_IMAGELENGTH, image_info->length);
	TIFFSetField(op_tif, TIFFTAG_IMAGEWIDTH, image_info->width);
	TIFFSetField(op_tif, TIFFTAG_BITSPERSAMPLE, 16);
	TIFFSetField(op_tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(op_tif, TIFFTAG_PHOTOMETRIC, 1);
	TIFFSetField(op_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	
	if((scanline = (uint16 *)_TIFFmalloc(image_info->width * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}
	
	for (r = 0; r < image_info->length; r++){
		for (c = 0; c < image_info->width; c++){	
			*(scanline + c) = image[image_info->width * r + c];
		}
		TIFFWriteScanline(op_tif, scanline, r, s);
	}

	printf("-- writing %s image done!\n", filename);
	
	_TIFFfree(scanline);
	TIFFClose(op_tif);
	return 0;
}

int tif_multi_syn(struct image_info_type *image_info, uint16 *image, char *filename)
{
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

	if((scanline = (uint16 *)_TIFFmalloc(image_info->width * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for the scan buffer!\n");
		exit(0);
	}

	for (np = 0; np < image_info->page_num; np++){
		TIFFSetField(op_tif, TIFFTAG_PAGENUMBER, np, image_info->page_num);
		
		for (r = 0; r < image_info->length; r++){
			for (c = 0; c < image_info->width; c++){
				*(scanline + c) = image[image_offset + image_info->width * r + c];
			}
			TIFFWriteScanline(op_tif, scanline, r, s);
		}
		image_offset += image_info->image_size/sizeof(uint16);
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
}

