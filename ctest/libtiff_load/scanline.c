#include <stdio.h>
#include <tiffio.h>
#include <stdlib.h>

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

int main(int argc, char **argv)
{
	int r, c;	// height index and width index
	uint16 s;

	struct tiff_info *info;	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));
	
	uint16 *input_image;
	uint16 *output_image;
	uint16 *scanline;	
	TIFF *input_file;
	int io_status;	// status of io operation
	
	unsigned long count;	
	FILE *output_file;	

	input_file = TIFFOpen(argv[1], "r");
	if(input_file == NULL){
		fprintf(stderr, "ERROR: Could not open input image!\n");
		exit(1);
	}
	printf("opening tiff file...\n");

	// input image paramters
	TIFFGetField(input_file, TIFFTAG_IMAGELENGTH, &info->length);
	TIFFGetField(input_file, TIFFTAG_IMAGEWIDTH, &info->width);
	TIFFGetField(input_file, TIFFTAG_BITSPERSAMPLE, &info->bps);
	TIFFGetField(input_file, TIFFTAG_SAMPLESPERPIXEL, &info->spp);
	TIFFGetField(input_file, TIFFTAG_PHOTOMETRIC, &info->photo_metric);
	TIFFGetField(input_file, TIFFTAG_PLANARCONFIG, &info->config);
		
	printf("length = %d, width = %d\n", info->length, info->width);
	printf("bit per sample = %d\n", info->bps);
	printf("sample per pixel = %d\n", info->spp);
	printf("photo metirc = %d\n", info->photo_metric);
	printf("planar config = %d\n", info->config);

	info->line_size = TIFFScanlineSize(input_file);
	info->image_size = info->line_size * info->length;
	
	if(info->spp != 1){
		info->spp = 1;
		printf("Warnning:sample per pixel value automatically set to 1!\n");
	}
	
	if((input_image = (uint16 *)_TIFFmalloc(info->image_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the uncompressed image!\n");
		exit(42);
	}

	if((scanline = (uint16 *)_TIFFmalloc(info->line_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for the scan buffer!\n");
		exit(42);
	}

	printf("the line size is %d\n", info->line_size);
	printf("the image size is %d\n", info->image_size);
	printf("reading tif files ... \n");

	if (info->config == PLANARCONFIG_CONTIG){
		for(r = 0; r < info->length; r++){
			TIFFReadScanline(input_file, scanline, r, s);
			
			for(c = 0; c < info->width; c++)
			{
				input_image[info->width * r + c] = *(scanline + c);
			}
		}
		
	} else if (info->config == PLANARCONFIG_SEPARATE){
		for(s = 0; s < info->spp; s++){
			for(r = 0; r < info->length; r++){
				TIFFReadScanline(input_file, scanline, r, s);
				for(c = 0; c < info->width; c++)
				{
					input_image[info->width * r + c] = *(scanline + c);
				}
			}
		}
	}

	output_file = fopen("loaded_image_line.dat", "w");

        for(count = 0; count < (info->image_size/2); count++){
                fprintf(output_file, "%04x", (uint16) input_image[count]);
                if((count + 1) % info->width == 0) fprintf(output_file, "\n");
                else fprintf(output_file, " ");
        }

        fclose(output_file);
	
	_TIFFfree(input_image);
	_TIFFfree(scanline);

	TIFFClose(input_file);	
	
	return 0;
}
