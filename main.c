/*DOCUMENTATION:
  NAME: N.MOHAMMED SHAQEEB
  REG_NO: 25031_124
  START_DATE: 28-01-2025
  END_DATE: 02-02-2026
  DESC: This is a LSB based Steganography which encodes and decodes secret data using ".bmp" type images.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
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


//Function for Checking operation
OperationType check_operation_type(char *argv[])
{
    //Checks for encoding operation
    if(strcmp(argv[1],"-e")==0)
    {
        return e_encode;
    }
    if(strcmp(argv[1],"-d")==0)//Checks for decoding operation
    {
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}


int main(int argc, char *argv[])
{
    if(argc<3)//checking the required number arguments passed or not
    {
        printf("INCOMPLETE INPUT! GIVE ALL REQUIRED INPUTS\n");
        return 1;
    }

    if(check_operation_type(argv)==e_encode)//Function call of encoding
    {
        //Encoding function call
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv,&encInfo)==e_success)//Validation of encoding
        {
            do_encoding(&encInfo);
        }
        else
        {
            printf("INVALID OPERATION INPUT\n");
            return 1;
        }
    }
    else if(check_operation_type(argv)==e_decode)//Function call of decoding
    {
        //decode
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv,&decInfo)==e_success)//Validating the args
        {
            do_decoding(&decInfo);
        }
        else
        {
            printf("INVALID OPERATION INPUT\n");
            return 1;
        }
    }
    else
    {
        printf("INVALID OPERATION INPUT\n");
        return 1;
    }

    return 0;
}
