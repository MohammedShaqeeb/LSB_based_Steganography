#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
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

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

//validation of args
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Checking whether the input file is of type .bmp
    if(strstr(argv[2],".bmp"))
    {
        encInfo->src_image_fname=argv[2];
        char * pos;
        //Checking whether secret file is provided as input
        if(argv[3]==NULL)
        {
            return e_failure;
        }
        //Getting the position of extention 
        if(pos=strchr(argv[3],'.'))
        {
            //Checking whether the extention is there
            if((pos+1)!=NULL)
            {
                encInfo->secret_fname=argv[3];

                //Copying the extention of secret file
                strcpy(encInfo->extn_secret_file,strstr(argv[3],pos));
                if(argv[4]!=NULL)
                {
                       if(strstr(argv[4],".bmp"))
                    {
                        encInfo->stego_image_fname=argv[4];
                        return e_success;
                    }
                }
                else
                {
                    encInfo->stego_image_fname="stego.bmp";
                    return e_success;
                }
            }
        }
    }
    return e_failure;
}

//Checking whether secret data can be encoded in source image
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image);

    //Getting the Magic string for authentication
    printf(BLUE"Enter the magic string : "RESET);
    scanf(" %s",encInfo->magic_string);

    //Finding the size of secret data
    fseek(encInfo->fptr_secret,0,SEEK_END);
    int size_secret_data=ftell(encInfo->fptr_secret);

    encInfo->size_secret_file=size_secret_data;

    fseek(encInfo->fptr_secret,0,SEEK_SET);

    //Calculating whether secret data can be encode or not
    int size_of_info=sizeof(int)+strlen(encInfo->magic_string)+sizeof(int)+strlen(encInfo->extn_secret_file)+sizeof(int)+size_secret_data;
    if((encInfo->image_capacity) > (size_of_info * 8))
    {
        //Exact amount of remaining data after encoding
        encInfo->image_capacity=encInfo->image_capacity-(size_of_info * 8);
        return e_success;
    }
    
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //Move the src_fptr to 0th position
    char arr[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(arr,54,1,fptr_src_image);

    //write array to fptr_dest_image
    fwrite(arr,54,1,fptr_dest_image);
    return e_success;
}

Status encode_size_to_lsb(int size,char *image_data)
{
    //Encode a single integer value into image
    unsigned int temp=0;
    for(int i=0;i<32;i++)
    {
        temp=size&(1<<i);
        temp=temp>>i;
        image_data[i]=image_data[i]&(~1);
        image_data[i]=image_data[i]|temp;
    }
}

//Encode magic string size
Status encode_magic_string_size(EncodeInfo *encInfo)
{
    char arr[32];
    //Read 32 bytes of data from image
    fread(arr,32,1,encInfo->fptr_src_image);

    encode_size_to_lsb(strlen(encInfo->magic_string),arr);

    //Write 32 bytes of encoded data to image 
    fwrite(arr,32,1,encInfo->fptr_stego_image);
}


Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //Encode a single character value to image
    unsigned char temp=0;
    for(int i=0;i<8;i++)
    {
        temp=data&(1<<i);
        temp=temp>>i;
        image_buffer[i]=image_buffer[i]&(~1);
        image_buffer[i]=image_buffer[i]|temp;
    }

}

//Encoding data into image
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i=0;i<size;i++)
    {
        //read 8 bytes from source image
        fread(arr,8,1,fptr_src_image);

        encode_byte_to_lsb(data[i],arr);

        //write 8 bytes to destination
        fwrite(arr,8,1,fptr_stego_image);
    }
}

//Encoding Magic string into the image
Status encode_magic_string(EncodeInfo *encInfo)
{
    
    encode_data_to_image(encInfo->magic_string, strlen(encInfo->magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);

}

//Encode Secret file extention size into image
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char arr[32];
    //Read 32 bytes of data from image
    fread(arr,32,1,encInfo->fptr_src_image);

    encode_size_to_lsb(strlen(encInfo->extn_secret_file),arr);

    //Write 32 bytes of encoded data to image
    fwrite(arr,32,1,encInfo->fptr_stego_image);
}

//Encode secret file extention to image
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    encode_data_to_image(encInfo->extn_secret_file, strlen(encInfo->extn_secret_file),encInfo->fptr_src_image,encInfo->fptr_stego_image);   
}

//Encode Secret file size to image
Status encode_secret_file_size(EncodeInfo *encInfo)
{
    char arr[32];
    //Read 32 bytes of data from image
    fread(arr,32,1,encInfo->fptr_src_image);

    encode_size_to_lsb(encInfo->size_secret_file,arr);

    //Write 32 bytes of encoded data to image
    fwrite(arr,32,1,encInfo->fptr_stego_image);
} 

//Encode secret data into image
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    for(int i=0;i<(encInfo->size_secret_file);i++)
    {

        fread(encInfo->secret_data,1,1,encInfo->fptr_secret);
        encode_data_to_image(encInfo->secret_data, 1,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    }
}

//Copying remaining unused data of image
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    uint n=encInfo->image_capacity;
    char *arr=malloc(n);
    if (arr == NULL)
    {
        return e_failure;
    }
    fread(arr,n,1,encInfo->fptr_src_image);
    fwrite(arr,n,1,encInfo->fptr_stego_image);
}

//Encoding of secret happens here
Status do_encoding(EncodeInfo *encInfo)
{
    //Opening the files required for encoding
    if(open_files(encInfo)==e_success)
    {
        //success msg
        printf(GREEN"SUCCESSFULLY OPENED ALL FILES\n"RESET);
    }
    else
    {
        printf(RED"FILES COULD NOT BE OPENED\n"RESET);
        return e_failure;
    }

    //Checking whether secret data can be encoded in file or not
    if(check_capacity(encInfo)==e_success)
    {
        //success msg
        printf(CYAN"The Data can be encoded\n"RESET);
    }
    else
    {
        //Error msg
        printf(YELLOW"The Size is not enough to store the Secret Data\n"RESET);
        return e_failure;
    }
    
    //Copying the bmp file header fully
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
    {
        //success msg
        printf(GREEN"The Meta data is copied successfully!\n"RESET);
    }

    //Encoding of Magic string and its size
    encode_magic_string_size(encInfo);
    encode_magic_string(encInfo);
    printf(GREEN"Magic string is encoded\n"RESET);

    //Encoding of Secret File extention and its size
    encode_secret_file_extn_size(encInfo);
    encode_secret_file_extn(encInfo);
    printf(GREEN"File extention is encoded\n"RESET);

    //Encoding of Secret data and its size
    encode_secret_file_size(encInfo);
    encode_secret_file_data(encInfo);
    printf(GREEN"Secret data is encoded\n"RESET);

    //Copy remaining unused data of image
    copy_remaining_img_data(encInfo);  
    printf(GREEN"Remaining data is encoded\n"RESET);  

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

}