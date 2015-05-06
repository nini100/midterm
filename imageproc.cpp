/*
 * imageproc.cpp
 *
 *  Created on: 2015. 5. 1.
 *      Author: nini
 */

#include "imageproc.h"

// image read method
char *readImage(const char *file_name, unsigned long *length){
    FILE *pgmFile;	// file pointer
    char version[3];
    int i, j, cols, rows, levels;   //image height, width, color levels
    int pixel;
    struct image_header *ih;
    char *image_data;
    char *pix;

    pgmFile = fopen(file_name, "rb");	// binary mode (only read)
    if (pgmFile == NULL) {	// if file doesn't open
        fprintf(stderr, "cannot open file to read");	//standard error
        exit(EXIT_FAILURE);
    }

    fgets(version, sizeof(version), pgmFile);	// read image version
    // if image is not pgm or ppm file
    if (strcmp(version, "P5") != 0 && strcmp(version, "P6") != 0) {
        fprintf(stderr, "Wrong file type!\n");
        exit(EXIT_FAILURE);
    }

    fscanf(pgmFile, "%d", &cols);	//  read image width
    fscanf(pgmFile, "%d", &rows);	// read image height
    fscanf(pgmFile, "%d", &levels);	// read image levels
    fgetc(pgmFile);	//read character one by one


    if (strcmp(version, "P5") == 0){	// allocate memories
        image_data = (char *)malloc(rows * cols + sizeof(struct image_header));
    }else{
        image_data = (char *)malloc(3 * rows * cols + sizeof(struct image_header));
    }

    ih = (struct image_header *) image_data;
    memcpy(ih, version, 3);	//copy memory (image version)
    ih->cols = cols;	//image width
    ih->rows = rows;	//image height
    ih->levels = levels;	//image levels

    if (strcmp(version, "P5") == 0){ //if image is PGM file
        for (j = 0; j < rows; ++j)
            for (i = 0; i < cols; ++i) {
                pixel = (char)fgetc(pgmFile);	//read pixel one by one
                image_data[sizeof(struct image_header) + j*cols + i] = pixel;	// put the value of pixels
            }
    }else{	// if image is PPM file
        for (j = 0; j < rows; ++j)
            for (i = 0; i < cols; ++i) {
                pix = &image_data[sizeof(struct image_header) + j*(cols * 3) + i * 3]; //change the value which pix pointer indicated
                pix[0] = (char)fgetc(pgmFile);	// read the value of pixels and put it in image data array
                pix[1] = (char)fgetc(pgmFile);
                pix[2] = (char)fgetc(pgmFile);
            }
    }


    if (strcmp(version, "P5") == 0){ // if image version is P5(PPM Binary)
        *length = sizeof(struct image_header) + rows * cols;  //image length
    }else{	//if image version is P6(PGM binary)
        *length = sizeof(struct image_header) + 3 * rows * cols; // R,G,B -> multiply 3 times
    }
    fclose(pgmFile); //close the file
    return image_data;
}
//-----------------------------------------------------------------------------
void writeImage(const char *filename, const char *image_data){	//image write method
    FILE *pgmFile; //file pointer
    int i, j;
    const char *pix;
    struct image_header *ih;

    pgmFile = fopen(filename, "wb");	//open the image file in binary mode(only write)
    if (pgmFile == NULL) {	//if image file doesn't open
        perror("cannot open file to write"); //print error value in error output stream
        exit(EXIT_FAILURE);
    }

    ih = (struct image_header *) image_data; //ih pointer indicates image_data

    fprintf(pgmFile, "%s ", ih->format);	//print the value of image version(string type) to pgmFile
    fprintf(pgmFile, "%d %d ", ih->cols, ih->rows);	//print the value of image width and height(integer type) to pgmFile
    fprintf(pgmFile, "%d ", ih->levels);  //print the value of image levels(integer type) to pgmFile

    if(strcmp(ih->format, "P5") == 0){	//if image version is P5(PGM binary)
        for (j = 0; j < ih->rows; ++j)
            for (i = 0; i < ih->cols; ++i) { //write the value of pixels one by one to pgmFile
                pix = &image_data[sizeof(struct image_header) + j*ih->cols + i];
                fputc(pix[0], pgmFile);
            }
    }else{	//if image version is P6(PPM binary)
        for (j = 0; j < ih->rows; ++j)
            for (i = 0; i < ih->cols; ++i) { //write the 3 values of pixels(R, G, B) to pgmFile
                pix = &image_data[sizeof(struct image_header) + j * ih->cols * 3 + i * 3];
                fputc(pix[0], pgmFile);
                fputc(pix[1], pgmFile);
                fputc(pix[2], pgmFile);
            }
    }

    fclose(pgmFile); //close the image file
}

//-----------------------------------------------------------------------------
char *getBlocks(const char *image, int cols, int rows, int bytes_per_pix, int block_size, unsigned long *data_length){
    unsigned num_blocks =  (cols/block_size) * (rows/block_size);
    int i, k;
    const char *image_data = image + sizeof(image_header);
    *data_length = (block_size * block_size * bytes_per_pix + sizeof(unsigned)) * num_blocks;
    char *blocks_data = (char *) malloc(*data_length);

    unsigned *ptr_block_sq_number, block_sq_number = 0;
    unsigned x_bl, y_bl;
    int block_bytes = 0;
    char **blocks = (char **) malloc(num_blocks * (sizeof(char *)));
    for (i = 0; i < num_blocks; i++) {
        blocks[i] = blocks_data + i * (block_size * block_size * bytes_per_pix + sizeof(unsigned));
    }

    for (i = 0; i < num_blocks; i++) {
        ptr_block_sq_number = (unsigned *) blocks[block_sq_number];
        *ptr_block_sq_number = block_sq_number++;
        x_bl = (i * block_size) % cols;
        y_bl = block_size * ((i * block_size) / cols);
        block_bytes += sizeof(unsigned);
        for (k = 0; k < block_size; k++) {
            block_bytes += block_size * bytes_per_pix;
            memcpy(blocks[*ptr_block_sq_number] + sizeof(unsigned) + k * block_size * bytes_per_pix,
                   image_data + y_bl * cols * bytes_per_pix + x_bl * bytes_per_pix + k * cols * bytes_per_pix, block_size * bytes_per_pix);
        }
        printf("%d\n", block_bytes);
        block_bytes = 0;
    }
    //free(blocks);
    return blocks_data;
}

void rgb2ycbcr(char *image, int cols, int rows){ //convert rgb file to ycbcr file

    float T[3][3] = {{ .299,   .587,    .114}, //Y = 0.299*R + 0.587*G + 0.114*B
        {-.169,  -.331,    .500},	// Cb = -0.169*R -0.331*G + 0.5*B
        {.500,      .419,    -.081}}; //Cr = 0.5*R -0.419*G -0.081*B
    float offset[3] = {0, 128, 128};  //added/subtracted in order to bring the values into the range[0,255]
    float temp_pixel[3];

    int i,j;
    char *pix;
    for (j = 0; j < rows; ++j){
        for (i = 0; i < cols; ++i){
            pix = &image[j * cols * 3 + i * 3];
            temp_pixel[0] =      T[0][0] * pix[0] + //the value of Y
            T[0][1] * pix[1] +
            T[0][2] * pix[2] +
            offset[0];
            temp_pixel[1] =  T[1][0] * pix[1] +	//	the value of Cb
            T[1][1] * pix[1] +
            T[1][2] * pix[2] +
            offset[1];
            temp_pixel[2] =  T[2][0] * pix[2] +	//the value of Cr
            T[2][1] * pix[1] +
            T[2][2] * pix[2] +
            offset[2];

            pix[0] = (char)temp_pixel[0];
            pix[1] = (char)temp_pixel[1];
            pix[2] = (char)temp_pixel[2];
        }
    }

}


void ycbcr2rgb(char *input, int cols, int rows){	//convert ycbcr file to rgb file

    float Ti[3][3] = {{ 1.000,   0.000,     1.403},	//R = 1.000*Y +0.000*(Cb-128) +1.403*(Cr-128)
        { 1.000,  -0.344,  -0.714},	//G = 1.000*Y -0.344*(Cb-128) -0.714*(Cr-128)
        { 1.000,   1.773,   0.000}};   //B = 1.000*Y + 1.773*(Cb-128) + 0.000*(Cr-128)
    float offset[3] = {0, 128, 128};	//added/subtracted in order to bring the values into the range[0,255]
    float temp_pixel[3];

    int i,j;
    char *pix;
    for (j = 0; j < rows; ++j){
        for (i = 0; i < cols; ++i){
            pix = &input[j * cols * 3 + i * 3];
            temp_pixel[0] =      Ti[0][0] * (pix[0] - offset[0]) +	//the value of R
            Ti[0][1] * (pix[1] - offset[1]) +
            Ti[0][2] * (pix[2] - offset[2]);
            temp_pixel[1] =  Ti[1][0] * (pix[1] - offset[1]) +	//the value of G
            Ti[1][1] * (pix[1] - offset[1]) +
            Ti[1][2] * (pix[2] - offset[2]);
            temp_pixel[2] =  Ti[2][0] * (pix[2] - offset[2]) +	//the value of B
            Ti[2][1] * (pix[1] - offset[1]) +
            Ti[2][2] * (pix[2] - offset[2]);

            pix[0] = (char)temp_pixel[0];
            pix[1] = (char)temp_pixel[1];
            pix[2] = (char)temp_pixel[2];
        }
    }
}
