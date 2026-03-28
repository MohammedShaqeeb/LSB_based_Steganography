#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"

// Text color macros
#define RESET   "\033[0m"
#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"


//Opens the data encode image file
Status open_image_file(DecodeInfo *decInfo)
{
    // Decode Image file
    decInfo->fptr_dec_image = fopen(decInfo->dec_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_dec_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->dec_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status open_secret_file(DecodeInfo *decInfo)
{
    //Opens the file to store secret data
    decInfo->fptr_out_file=fopen(decInfo->output_fname,"w");
    if (decInfo->fptr_out_file == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

//Skips the bmp header part of image
Status skip_bmp_header(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_dec_image,54,SEEK_SET);

    return e_success;
}

//Read and validate the args passed as input
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Checks whether the image file is of type .bmp
    if(strstr(argv[2],".bmp"))
    {
        decInfo->dec_image_fname=argv[2];
        //Checks whether the output file is given
        if(argv[3]!=NULL)
        {
            //If provided it omits the extention and extracts only filename
            decInfo->output_fname = malloc(50);
            strcpy(decInfo->output_fname,argv[3]);
            strtok(decInfo->output_fname,".");
            return e_success;
        }
        else
        {
            //If not provided it takes "output" as filename
            decInfo->output_fname = malloc(50);
            strcpy(decInfo->output_fname, "output");
            return e_success;
        }
    }
    return e_failure;
}

//Decodes a single integer data from image
uint decode_size_from_lsb(char * str_size)
{
    uint size=0;
    for(int i=0;i<32;i++)
    {
        uint temp=str_size[i]&1;
        temp=temp<<i;
        size=size|temp;
    }

    return size;
}

//Decodes a single byte of data from image
char decode_byte_from_lsb(char *data)
{
    unsigned char ch=0;
    for(int i=0;i<8;i++)
    {
        unsigned char temp=data[i]&1;
        temp=temp<<i;
        ch=ch|temp;   
    }

    return ch;
}

//Decoding Magic string size
Status decode_magic_string_size(DecodeInfo *decInfo)
{

    char arr[32];
    //Read 32 bytes from encoded image
    fread(arr,32,1,decInfo->fptr_dec_image);

    //Decoding size from encoded data
    decInfo->magic_string_size=decode_size_from_lsb(arr);
    return e_success;
}

//Function to decode a single character from data
Status data_from_image(uint size, FILE *fptr_dec_image,char *data_str)
{
    char arr[8]; 
    for(uint i=0;i<size;i++)
    {
        fread(arr,8,1,fptr_dec_image);

        data_str[i]=decode_byte_from_lsb(arr);
    }   
    data_str[size] = '\0';
    return e_success;
}

//Decoding magic string from encoded data
Status decode_magic_string(DecodeInfo *decInfo)
{
    //Decoding magic string here
    data_from_image(decInfo->magic_string_size,decInfo->fptr_dec_image,decInfo->magic_str);
    char mg_str[50];

    //Getting Magic string from user for authentication 
    printf(BLUE"Enter the Magic String: "RESET);
    scanf(" %s",mg_str);

    //Comparing bothe magic string to confirm access
    if(strcmp(decInfo->magic_str,mg_str)==0)
    {
        return e_success;
    }
    return e_failure;
}

//Decoding file extention size
Status decode_file_extn_size(DecodeInfo *decInfo)
{

    char arr[32];
    //Read 32 bytes from encoded image
    fread(arr,32,1,decInfo->fptr_dec_image);

    //Decoding extention size from encoded data
    decInfo->extn_size=decode_size_from_lsb(arr);
    return e_success;

}

//Decoding file extention from image
Status decode_file_extn(DecodeInfo *decInfo)
{
    //Decoding file extention
    data_from_image(decInfo->extn_size,decInfo->fptr_dec_image,decInfo->file_extn_str);

    //Concatinating the extention with secret filename
    strcat(decInfo->output_fname,decInfo->file_extn_str);

    return e_success;
}

//Decoding secret data size from image
Status decode_secret_data_size(DecodeInfo *decInfo)
{
    char arr[32];
    //Read 32 bytes from encoded image
    fread(arr,32,1,decInfo->fptr_dec_image);

    //Decoding secret data size from encoded data
    decInfo->secret_data_size=decode_size_from_lsb(arr);

    return e_success;

}

//Decoding secret data from image
Status decode_secret_data(DecodeInfo *decInfo)
{
    for(int i=0;i<(decInfo->secret_data_size);i++)
    {
        //Decoding from image
        data_from_image(1,decInfo->fptr_dec_image,decInfo->secret_data_decode);\

        //Writing the data to the file of extention type 
        fwrite(decInfo->secret_data_decode,1,1,decInfo->fptr_out_file);
    }
    return e_success;
}

//Decoding happens here
Status do_decoding(DecodeInfo *decInfo)
{
    //Opening the image file to decode the encoded data
    if(open_image_file(decInfo)==e_success)
    {
        //success msg
        printf(GREEN"SUCCESSFULLY OPEN ENCODED FILE\n"RESET);
    }
    else
    {
        printf(RED"FILE COULD NOT BE OPENED\n"RESET);
        return e_failure;
    }

    //Skipping the bmp image header
    if(skip_bmp_header(decInfo)==e_success)
    {
        printf(CYAN"Decoding is going to start...\n"RESET);
    }

    //Decoding the magic string and its size
    decode_magic_string_size(decInfo);
    if(decode_magic_string(decInfo)==e_success)
    {
        printf(GREEN"ACCESS GRANTED\n"RESET);
    }
    else
    {
        printf(YELLOW"UNAUTHORIZED ACCESS\n"RESET);
        return e_failure;
    }
    printf(GREEN"MAGIC STRING DECODED\n"RESET);

    //Decoding the file extention and its size
    decode_file_extn_size(decInfo);
    decInfo->file_extn_str = malloc(decInfo->extn_size + 1);
    decode_file_extn(decInfo);
    printf(GREEN"FILE EXTENTION EXTRACTED\n"RESET);

    //Opening secret file once extention is extracted
    if(open_secret_file(decInfo)==e_success)
    {
        //success msg
        printf(GREEN"SUCCESSFULLY OPENED SECRET FILE\n"RESET);
    }
    else
    {
        printf(RED"FILE COULD NOT BE OPENED\n"RESET);
        return e_failure;
    }

    //Decoding the Secret data and its size 
    decode_secret_data_size(decInfo);
    decode_secret_data(decInfo);
    printf(GREEN"DECODING COMPLETED..;)\n"RESET);

    //Closing all the files once decoding completed
    fclose(decInfo->fptr_dec_image);
    fclose(decInfo->fptr_out_file);
    free(decInfo->output_fname);

    return e_success;
}