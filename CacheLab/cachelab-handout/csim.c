#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include "cachelab.h"


void usage();
int parse_arg(int argc, char* argv[]);
void init();
void calculate();
void update(int, int);
void free_cache();

typedef struct
{
    int tag, valid, timestamp;
}line;

typedef struct
{
    int S, E, B;
    line** ptr;
}cache;

cache set;
int op = -1, verbose = 0, s = -1, E = -1, b = -1;
char* file_input = NULL;
int hits, misses, evictions;


int main(int argc, char* argv[])
{

    int res = parse_arg(argc, argv);
    if(res == -1) return -1;

    init();
    calculate();
    //free_cache();
    printSummary(hits, misses, evictions);    
    return 0;
}

void free_cache()
{
    for(int i = 0; i < set.S; i ++)
        free(set.ptr[i]);
    free(set.ptr);
}

void calculate()
{
    FILE* fp = fopen(file_input, "r");
    char file_line[20], op;
    int addr, size;
    while(fgets(file_line, 20, fp) != NULL)
    {
        if(file_line[0] == 'I') continue;
        //我们假设内存自动对齐，因此可以忽略size
        sscanf(file_line, " %c %x,%d", &op, &addr, &size);
        int group = (addr >> b) & (((unsigned)-1) >> (8 * sizeof(unsigned) - s));
        int tag = addr >> (b + s);

        switch(op)
        {
            case 'M': update(tag, group);
            case 'L':
            case 'S': update(tag, group);
        }
    }
    fclose(fp);
}

int get_idx(int tag, int group)
{
    for(int i = 0; i < set.E; i ++)
    {
        if(set.ptr[group][i].valid && set.ptr[group][i].tag == tag)
            return i;
    }
    return -1;
}

void update(int tag, int group)
{
    //printf("begin:  tag == %d  group == %d\n", tag, group);
    //idx表示某一行，即当前地址所对应的行是否存在
    int idx = get_idx(tag, group);
    if(idx == -1)//miss
    {
        misses++;
        if(verbose) printf("miss\n");
        int full = 1;
        //该组中是否存在行没有满
        for(int i = 0; i < set.E; i ++)
        {
            if(set.ptr[group][i].valid == 0)
                full = 0;
        }
        if(full)
        {
            evictions++;
            if(verbose) printf("eviction\n");
            int mx = -1;
            for(int i = 0; i < set.E; i ++)
            {
                if(set.ptr[group][i].timestamp > mx)
                {
                    mx = set.ptr[group][i].timestamp;
                    idx = i;
                }
            }
        }
        else
        {
            for(int i = 0; i < set.E; i ++)
            {
                if(set.ptr[group][i].valid == 0)
                {
                    idx = i;
                    break;
                }
            }
        }
    }
    else
    {
        hits++;
        if(verbose) printf("hit\n");
    }
    for(int i = 0; i < set.E; i ++)
    {
        if(set.ptr[group][i].valid)
            set.ptr[group][i].timestamp++;
    }
    //printf("end:  idx == %d  tag == %d  group == %d\n", idx, tag, group);
    set.ptr[group][idx].timestamp = 0;
    set.ptr[group][idx].tag = tag;
    set.ptr[group][idx].valid = 1;
}

void init()
{
    set.S = 1 << s;
    set.B = 1 << b;
    set.E = E;      //E表示标记个数
    set.ptr = (line**)malloc(set.S * sizeof (line*));
    for(int i = 0; i < set.S; i ++)
    {
        set.ptr[i] = (line*)malloc(set.E * sizeof (line));
        for(int j = 0; j < set.E; j ++)
        {
            set.ptr[i][j].tag = -1;
            set.ptr[i][j].valid = 0;
            set.ptr[i][j].timestamp = 0;
        }
    }
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
    while((op = getopt(argc, argv, "hvs:E:b:t:")) != -1)
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
    if(file_input == NULL) 
    {
        usage();
        return -1;
    }
    return 0;
}




