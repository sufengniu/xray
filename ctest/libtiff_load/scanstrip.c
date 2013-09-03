#include <stdio.h>
#include <tiffio.h>
#include <stdlib.h>
#include <string.h>

#define BIT_PER_SAMPLE		16	// defined by camera property
#define SAMPLE_PER_PIXEL	1	// sample per pixel default is 1

typedef struct tiff_info{
	unsigned short type;	/* little or big endian */
	int width;
	int length;
	int depth;
	unsigned short photo_metric;
	unsigned short spp;	// sample per pixel, 16 bits per pixel for ANL camera
	unsigned short bps;	// bit per sample, default is 1
	int line_size;
	int image_size;
} tiff_info;

int main(int argc, char **argv)
{
	int r, c;	// height index and width index
	
	struct tiff_info *info;	
	info = (struct tiff_info *)malloc(sizeof(struct tiff_info));
	
	uint16 *input_image;
	uint16 *output_image;
	uint16 *stripbuffer;	
	TIFF *input_file;
	int io_status;	// status of io operation
	
	/* according to tiffio library tsize_t is int32, tstrip_t is uint32 */	
	tsize_t strip_size, buffer_size;
	tstrip_t strip_max, strip_count, strip_num;
	unsigned long image_offset, result;
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
	
	printf("length = %d, width = %d\n", info->length, info->width);
	printf("bit per sample = %d\n", info->bps);
	printf("sample per pixel = %d\n", info->spp);
	printf("photo metirc is %d\n", info->photo_metric);

	// Read by multiple strips
	strip_size = TIFFStripSize (input_file);
	strip_max = TIFFNumberOfStrips (input_file);
	image_offset = 0;

	buffer_size = TIFFNumberOfStrips (input_file) * strip_size;
	
	printf("the strip_size is %u\n", strip_size);
	printf("the strip_max is %u\n", strip_max);
	printf("the image size if %u\n", buffer_size);
	printf("reading tif files...\n");

	if((stripbuffer = (uint16 *)_TIFFmalloc(buffer_size)) == NULL){
		fprintf(stderr, "Could not allocate enough memory for uncompressed image!\n");
		exit(42);
	}
	
	for (strip_count = 0; strip_count < strip_max; strip_count++){
		/*
		if((result = TIFFReadEncodedStrip( input_file, strip_count,
						stripbuffer + image_offset,
						strip_size)) == -1){
			fprintf(stderr, "reading error occurs on input strip number %d\n", strip_count);
			exit(42);
		}*/
		result = TIFFReadEncodedStrip( input_file, strip_count,
						stripbuffer + image_offset,
						strip_size);

		image_offset += result/2;
	}

	/*	
	if(info->photo_metric != PHOTOMETRIC_MINISWHITE){
		// Flip bits
		printf("Fixing the photometric interpretation\n");

		for(count = 0; count < buffer_size; count++)
		stripbuffer[count] = ~stripbuffer[count];
	}*/

	output_file = fopen("loaded_image_strip.dat", "w");

        for(count = 0; count < buffer_size/2; count++){
                fprintf(output_file, "%04x", (uint16) stripbuffer[count]);
                if((count + 1) % info->width == 0) fprintf(output_file, "\n");
                else fprintf(output_file, " ");
        }

        fclose(output_file);

	// _TIFFfree(input_image);
	_TIFFfree(stripbuffer);

	TIFFClose(input_file);	
	
	return 0;
}
