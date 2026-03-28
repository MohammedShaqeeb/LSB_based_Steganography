#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Decoded Image info */
    char *dec_image_fname;
    FILE *fptr_dec_image;

    /* Output File Info */
    char *output_fname;
    FILE *fptr_out_file;

    //Magic string Info
    uint magic_string_size;
    char *magic_str;

    //File Extention Info
    uint extn_size;
    char *file_extn_str;
    
    //Secret Data Info
    uint secret_data_size;
    char secret_data_decode[MAX_SECRET_BUF_SIZE];

} DecodeInfo;

/* Encoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the Decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointer for i/p file */
Status open_image_file(DecodeInfo *decInfo);

/* Get File pointer o/p file*/
Status open_secret_file(DecodeInfo *decInfo);

/* Skip bmp image header */
Status skip_bmp_header(DecodeInfo *decInfo);

//Decode Magic String size
Status decode_magic_string_size(DecodeInfo *decInfo);

//Decode Magic String 
Status decode_magic_string(DecodeInfo *decInfo);

//Decode File Extention size
Status decode_file_extn_size(DecodeInfo *decInfo);

//Decode File Extention
Status decode_file_extn(DecodeInfo *decInfo);

//Decode Secret Data size
Status decode_secret_data_size(DecodeInfo *decInfo);

//Decode Secret Data
Status decode_secret_data(DecodeInfo *decInfo);

/* Decode function, which does the real decoding */
Status data_from_image(uint size, FILE *fptr_dec_image,char *magic_str);

//Decode a single integer
uint decode_size_from_lsb(char * str_size);

//Decode a single character
char decode_byte_from_lsb(char *data);

#endif