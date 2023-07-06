#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include "cachelab.h"


void usage();
int parse_arg(int argc, char* argv[]);


int op = -1, verbose = 0, s = -1, E = -1, b = -1;
char* file_input = NULL;


int main(int argc, char* argv[])
{
    //printSummary(0, 0, 0);
    int res = parse_arg(argc, argv);
    if(res == -1) return -1;

    return 0;
}

void usage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("-h: Optional help flag that prints usage info\n");
    printf("-v: Optional verbose flag that displays trace info\n");
    printf("-s <s>: Number of set index bits (S = 2s is the number of sets)\n");
    printf("-E <E>: Associativity (number of lines per set)\n");
    printf("-b <b>: Number of block bits (B = 2b is the block size)\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n");
}

int parse_arg(int argc, char* argv[])
{
    while((op = getopt(argc, argv, "h::v::s:E:b:t:")) != -1)
    {
        switch(op)
        {
            case 'h':
                usage();
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                file_input = optarg;
                break;
            default:
                usage();
                return -1;
        }
    }
    return 0;
}




