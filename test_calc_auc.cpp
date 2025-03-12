//no precompiled header to keep this project simple
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include "calc_auc.h"

/*Input file should be an ANSI text file with lines containing:

        ground truth value (integer 0 or 1), floating point prediction

for example:

        0, 0.1
        0, 3
        1, 0.8

return numbers of valid lines read, 0 if error.*/
static unsigned int LoadInputFile(const char* pf,AUCDatum*p,unsigned int bufLen)
{
    FILE* fp = fopen(pf, "rt");
    if (!fp) {
        fprintf(stderr, "couldn't open input file");
        return 0;
    }

    unsigned int n=0;
    for (n = 0; n < bufLen; n++) {
        if (2 != fscanf(fp, "%u ,%f\n", &p[n].ground_truth, &p[n].prediction)) break;
        if (p[n].ground_truth != 0 && p[n].ground_truth != 1) {
            fclose(fp);
            fprintf(stderr, "ground truth value must be either 0 or 1");
            return 0;
        }
    }
    fclose(fp);
    printf("data points read from input file: %u\n", n);

    if (n < 2) {
        fprintf(stderr,"need at least two data points to calculate AUC");
        return 0;
    }
    return n;
}

int main(int argc,char**argv) //ANSI version only
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s input_file", argv[0]);
        return -1;
    }

    const unsigned long long MAX_LINES_TO_READ = 1048576;
    AUCDatum* pd = new(std::nothrow) AUCDatum[MAX_LINES_TO_READ];
    if (!pd) {
        fprintf(stderr, "Memory allocation failed.");
        return -2;
    }
    unsigned int n = LoadInputFile(argv[1],pd, MAX_LINES_TO_READ);
    if (n>=2)
        printf("CalculateAUC() returned %f\n", CalculateAUC(pd, n));

    delete[]pd;
    return 0;
}
