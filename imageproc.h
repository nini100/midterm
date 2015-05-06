/*
 * imageproc.h
 *
 *  Created on: 2015. 5. 1.
 *      Author: nini
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 8				//define BLOCK_SIZE to 8
#define IMAGE_WIDTH 640		    //define IMAGE_WIDTH to 640
#define IMAGE_HEIGHT 400			//define IMAGE_HEIGHT 400
#define CHANNELS 3

struct image_header{
    char format[3]; //Image format, example: P5
    int rows;       //Image height
    int cols;       //Image width
    int levels;     //Number of gray levels
};

// image read method
char *readImage(const char *file_name, unsigned long *data_length);
// image write method
void writeImage(const char *filename, const char *image_data);

char *getBlocks(const char *image, int cols, int rows, int bytes_per_pix, int block_size, unsigned long *data_length);

void rgb2ycbcr(char *image, int cols, int rows);	// rgb -> ycbcr
void ycbcr2rgb(char *input, int cols, int rows);	// ycbcr -> rgb

