#include <stdio.h>
#include <tiffio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000L;

#define BIT_PER_SAMPLE		16	// defined by camera property

typedef struct tiff_info{
	unsigned short type;	/* little or big endian */
	int width;
	int length;
	int depth;
	unsigned short photo_metric;
	uint32 spp;	// sample per pixel, 16 bits per pixel for ANL camera
	uint16 bps;	// bit per sample, default is 1
	int line_size;
	int image_size;
	uint16 config;
} tiff_info;

uint16 *input_image;

int main(int argc, char **argv)
{
	// clock_t start, end;
        struct timespec start, stop;
        double accum;

	int r, c;	// height index, width index
	uint16 s;
	int dircount = 0;
	
	struct tiff_info *info;	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));
	
	uint32 data_size;
	unsigned long image_offset;
	//uint16 *input_image;
	uint16 *scanline;	
	TIFF *tif;

	// verify purpose
	uint32 count;
	FILE *output_file;	

	tif = TIFFOpen(argv[1], "r");
	if(tif == NULL){
		fprintf(stderr, "ERROR: Could not open input image!\n");
		exit(1);
	}
	printf("opening tiff file...\n");

	do {
		dircount++;
	} while (TIFFReadDirectory(tif));
	printf("%d images in %s\n", dircount, argv[1]);
	
	// input image paramters
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &info->length);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &info->width);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &info->bps);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &info->spp);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &info->photo_metric);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &info->config);
		
	printf("length = %d, width = %d\n", info->length, info->width);
	printf("bit per sample = %d\n", info->bps);
	printf("sample per pixel = %d\n", info->spp);
	printf("photo metirc = %d\n", info->photo_metric);
	printf("planar config = %d\n", info->config);

	info->line_size = TIFFScanlineSize(tif);
	info->image_size = info->line_size * info->length;
	data_size = info->image_size * dircount;	

	if(info->spp != 1){
		info->spp = 1;
		printf("Warnning:sample per pixel value automatically set to 1!\n");
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
	printf("the line size is %d\n", info->line_size);
	printf("the image size is %d\n", info->image_size);
	printf("image offset is %ld\n", image_offset);
	printf("the total sequence image size is %d\n", data_size);
	printf("loading tif files ... \n");

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
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
		image_offset += info->image_size/2;
	} while (TIFFReadDirectory(tif));
	printf("reading done.\n");

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        accum = (stop.tv_sec - start.tv_sec)+(double)(stop.tv_nsec-start.tv_nsec)/(double)BILLION;
        printf("done in %lf second\n", accum);

	printf("writing to loadded_image_line.dat file...\n");
	output_file = fopen("loaded_image_line.dat", "w");

        for(count = 0; count < (info->image_size/2)*dircount; count++){
                fprintf(output_file, "%04x", (uint16) input_image[count]);
                if((count + 1) % info->width == 0) fprintf(output_file, "\n");
                else fprintf(output_file, " ");
        }

        fclose(output_file);
	printf("done!\n");
	
	_TIFFfree(input_image);
	_TIFFfree(scanline);

	TIFFClose(tif);	
	
	return 0;
}
