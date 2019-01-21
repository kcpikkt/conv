///  Name:       conv
/// Info:       Simple cli app converting numbers between number systems. 
///             It supports number systems with bases up to 36 due to character limitaton.
/// Language:   C
/// Author:     Kacper Kokot
/// Repo:       github.com/kcpikkt/conv
/// Note:       If you are using windows cmd or powershell, colored text is not going to be rendered properly.
//#define NO_COLOR //uncomment this line to use conv without ANSI colors 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define STR_HELPER(ARG) #ARG
#define STR(ARG) STR_HELPER(ARG)

#ifdef _WIN32
    #define APP_NAME "conv.exe"
#else
    #define APP_NAME "conv"
#endif

#define MAX_BASE 36
#define MIN_BASE 2 

#define DEFAULT_BUFLEN 1024
#define DEFAULT_FROM_BASE 10
#define DEFAULT_TO_BASE 2
#define DEFAULT_CASE_SIZE 0 //0 - minuscule, 1 -majuscule
#define DEFAULT_ENDIANNESS 1 //1 - big endian, 0 - little endian

#ifdef NO_COLOR
    #define RED(STR) STR
    #define YELLOW(STR) STR
    #define CYAN(STR) STR
    #define PURPLE(STR) STR
#else
    #define COL_NC "\e[0m"
    #define COL_RED "\e[1;31m"
    #define COL_YELLOW "\e[1;33m"
    #define COL_CYAN "\e[0;36m"
    #define COL_PURPLE "\e[0;35m"
    #define RED(STR) COL_RED STR COL_NC
    #define YELLOW(STR) COL_YELLOW STR COL_NC
    #define CYAN(STR) COL_CYAN STR COL_NC
    #define PURPLE(STR) COL_PURPLE STR COL_NC
#endif

#define FLAG_RESIZE_BUF "-r"
#define FLAG_OUTPUT_FILE "-o"
#define FLAG_ENDIANNESS "-e"
#define FLAG_FROM_BASE "-f"
#define FLAG_TO_BASE "-t"
#define FLAG_CASE_SIZE "-c"
#define FLAG_HELP "-h"

#define FLAGCMP(A,B) (A[0]==B[0] && A[1]==B[1])

#define ERR_HELP 1 
#define HELP_TEXT \
    "Usage:\n"\
    "   " APP_NAME " [[option]... [number]... ]...\n"\
    "   Every option is applied only to numbers following it.\n"\
    "   Syntax for options: "YELLOW("-option=value" )"\n\n"\
    "Options:\n"\
    "   " YELLOW(FLAG_FROM_BASE)        "\tBase from which numbers are going to be converted (" \
        PURPLE(STR(MIN_BASE))" - "PURPLE(STR(MAX_BASE))", " PURPLE(STR(DEFAULT_FROM_BASE))" by default).\n"\
    "   " YELLOW(FLAG_TO_BASE)          "\tBase to which numbers are going to be converted (" \
        PURPLE(STR(MIN_BASE))" - "PURPLE(STR(MAX_BASE))", " PURPLE(STR(DEFAULT_TO_BASE))" by default).\n"\
    "   " YELLOW(FLAG_ENDIANNESS)   "\tEndianness of the output, can be set either to "\
        "(" YELLOW("l")")ittle endian or ("YELLOW("b")")ig endian.\n"\
    "   " YELLOW(FLAG_OUTPUT_FILE) "\tOutput file to which result going to be written.\n"\
    "   " YELLOW(FLAG_RESIZE_BUF)  "\tOutput buffer length/maximal number of digits" \
        " (by default " PURPLE(STR( DEFAULT_BUFLEN )) ").\n"\
    "   " YELLOW(FLAG_CASE_SIZE)   "\tSize of letter cases representing digits greater than 9, "\
        PURPLE("0")" - minuscule, " PURPLE("1")" - majuscule.\n"\
    "   " YELLOW(FLAG_HELP)        "\tView this help.\n\n"\
    "If you are using windows cmd or powershell, colored text is not going to be rendered properly.\n"\
    "Recompile " APP_NAME " with NO_COLOUR macro or use terminal emulator that supports ANSI escape characters.\n"

#define ERR_BAD_NUMBER -1
#define ERR_BAD_NUMBER_MESSAGE\
    "   Typed number is incorrect.\n"
#define ERR_TOO_BIG -2
#define ERR_TOO_BIG_MESSAGE\
    "   Output has more digits than provided buffer can accomodate,\n"\
    "   this is by default " PURPLE( STR( DEFAULT_BUFLEN )) " and can be changed with "\
    YELLOW(FLAG_RESIZE_BUF)" flag\n"\
    "   You can also write output to a file with " YELLOW(STR(FLAG_OUTPUT_FILE))" flag.\n"\
    "   Optionally you could recompile "APP_NAME" with "CYAN("DEFAULT_BUFLEN")" adjusted to your choice.\n"
#define ERR_INCOMPATIBILE_BASE -3 
#define ERR_INCOMPATIBILE_BASE_MESSAGE \
    "   Base you are trying to set is out of supported range ("PURPLE("2")" - "PURPLE("36")").\n"
#define ERR_BAD_ENDIAN -4 
#define ERR_BAD_ENDIAN_MESSAGE \
    "   Enidiannes can be set either to (" YELLOW("l")")ittle endian or ("YELLOW("b")")ig endian.\n"
#define ERR_BAD_FILE -5 
#define ERR_BAD_FILE_MESSAGE \
    "   Output file opening failed.\n"
#define ERR_ALLOC -6 
#define ERR_ALLOC_MESSAGE \
    "   Resizing buffer failed.\n"
#define ERR_BAD_CASE_SIZE -7 
#define ERR_BAD_CASE_SIZE_MESSAGE \
    "   Bad case size given, " PURPLE("0")" - minuscule and " PURPLE("1")" - majuscule are correct values.\n"
#define PRINT_ERR(num, message) \
    printf(RED("Error")" while converting "PURPLE("%s")" from base %i to base %i.\n", num, from_base, to_base); printf(message);
#define PRINT_FLAG_ERR(flag,message) \
    printf(RED("Error")" while processing " YELLOW("%s") " flag.\n", flag ); printf(message);

    
size_t bufsz, buflen = DEFAULT_BUFLEN;
char _buffer[DEFAULT_BUFLEN];
char * buffer = &_buffer[0];
FILE * output;

int to_base     =DEFAULT_TO_BASE;
int from_base   =DEFAULT_FROM_BASE;
int endianness  =DEFAULT_ENDIANNESS;
int case_size   =DEFAULT_CASE_SIZE;
void error(int error_code, char * num);
int convert(char * n, int from_base, int to_base);
uint64_t str_to_uint64(uint64_t * num,char * n, int base);
int process_flag(char * flag, char * arg);

int ascii_to_linear(char n, int base){
    if(n > 47 && n < 58) return (n-48 < base ? n-48 : -1);
    if(n > 64 && n < 91) return (n-55 < base ? n-55 : -1);
    if(n > 96 && n < 123) return (n-87 < base ? n-87 : -1); 
    return -1;
}
char linear_to_ascii(int n, int base){ 
    if(n > -1 && n < 10) return (n < base ? n+48 : -1);
    if(n > 9  && n < 36) return (n < base ? n+87-(case_size*32) : -1);
    return -1;
}

int main(int argc, char * argv[]){
    output = stdout;
    if(argc == 1){printf(HELP_TEXT); return 0;}
    for(int i=1; i<argc; i++){
        if(argv[i][0] == '-' && argv[i][2] == '=') {
            argv[i][2] = '\0';
            error(process_flag(argv[i], &argv[i][3]), argv[i]);
            continue;
        }
        error(convert( argv[i], from_base, to_base ), argv[i]);
        if(output == stdout){ fprintf(output, PURPLE("%s")"\t",argv[i]);}
        else                { fprintf(output, "%s\t",argv[i]);}

        if(!endianness){ for(int k=0;  (size_t)k<bufsz; k++){ fputc(buffer[k], output);}}
        else           { for(int k=bufsz; k>=0; k--){ fputc(buffer[k], output);}}
        fputc('\n', output);
        fflush(output);
    }
}
int convert(char * n, int from_base, int to_base){
    memset(buffer, '0', buflen);
    uint64_t rep, c, temp, mp;
    rep = c = temp = mp = 0;
    int negative = 0;

    if(n[0] == '-'){negative = 1; n++;}
    if(str_to_uint64(&rep, n, from_base) != 0) { return ERR_BAD_NUMBER;}
    mp = floor(log((double)rep)/log((double)to_base));
    if(!((mp+2) < buflen)) { return ERR_TOO_BIG; }
    bufsz = mp+1+negative;
    buffer[bufsz] = '\0';
    if(negative) {buffer[0] = '-';}
    mp = 1;
    while(mp != 0 && rep > 0){
        mp = floor(log((double)rep)/log((double)to_base));
        c = floor(rep/pow(to_base, mp));
        buffer[mp+negative] = linear_to_ascii(c, to_base);
        rep -= c*pow(to_base, mp);
    }
    return 0;
}
uint64_t str_to_uint64(uint64_t * num,char * n, int base){
    (*num) = 0;
    int nsz = 0, c;
    while(n[nsz]!='\0'){ nsz++;}
    for(int i=0; i<nsz; i++){
        if((c = ascii_to_linear(n[i], base)) == -1 ) { return ERR_BAD_NUMBER;}
        (*num) += pow(base, (nsz-i-1))*(c); //TODO: input endianness
    }
    return 0;
}
int process_flag(char * flag, char * arg){
    if(FLAGCMP(FLAG_RESIZE_BUF, flag)) { 
        uint64_t newbuflen; 
        if(str_to_uint64(&newbuflen, arg, 10) != 0) { return ERR_ALLOC;};
        buffer = (char*)malloc((size_t)newbuflen);
        if(buffer == NULL) { buffer = _buffer; return ERR_ALLOC; }
    } 
    if(FLAGCMP(FLAG_OUTPUT_FILE, flag)) {
        output = fopen(arg, "a");
        if(!output){ output = stdout; return ERR_BAD_FILE;}
        return 0;
    } 
    if(FLAGCMP(FLAG_ENDIANNESS, flag)) {
        if(FLAGCMP(arg, "l\0")){ endianness = 0; return 0;}
        if(FLAGCMP(arg, "b\0")){ endianness = 1; return 0;}
        return ERR_BAD_ENDIAN;
    } 
    if(FLAGCMP(FLAG_FROM_BASE, flag)) {
        uint64_t newbase; str_to_uint64(&newbase,arg, 10);
        if(newbase == 0 || newbase > MAX_BASE){
            return ERR_INCOMPATIBILE_BASE;
        }
        from_base = newbase; return 0;
    } 
    if(FLAGCMP(FLAG_TO_BASE, flag)) {
        uint64_t newbase; str_to_uint64(&newbase,arg, 10);
        if(newbase < MIN_BASE || newbase > MAX_BASE){
            return ERR_INCOMPATIBILE_BASE;
        }
        to_base = newbase; return 0;
    } 
    if(FLAGCMP(FLAG_CASE_SIZE, flag)){
        uint64_t newcasesize; str_to_uint64(&newcasesize, arg, 10);
        if(newcasesize == 1 || newcasesize == 0){
            case_size = newcasesize; return 0;}
        else { return ERR_BAD_CASE_SIZE;}
    }
    if(FLAGCMP(FLAG_HELP, flag)) {
        error(ERR_HELP, arg);
    }
    return 1;
}
void error(int error_code, char * num){
    switch(error_code){
        case 0: { return; }
        case ERR_BAD_NUMBER: { PRINT_ERR(num, ERR_BAD_NUMBER_MESSAGE); }break;
        case ERR_TOO_BIG: { PRINT_ERR(num,ERR_TOO_BIG_MESSAGE); }break;
        case ERR_INCOMPATIBILE_BASE: { PRINT_FLAG_ERR(num,ERR_INCOMPATIBILE_BASE_MESSAGE); }break;
        case ERR_BAD_ENDIAN: { PRINT_FLAG_ERR(num,ERR_BAD_ENDIAN_MESSAGE); }break;
        case ERR_BAD_FILE: { PRINT_FLAG_ERR(num,ERR_BAD_FILE_MESSAGE); }break;
        case ERR_BAD_CASE_SIZE: { PRINT_FLAG_ERR(num,ERR_BAD_CASE_SIZE_MESSAGE); }break;
        case ERR_HELP: { printf( HELP_TEXT ); }break;
    }
    exit(-1);
}
