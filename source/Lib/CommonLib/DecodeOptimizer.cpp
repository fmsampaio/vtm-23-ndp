#include "DecodeOptimizer.h"

FILE* DecodeOptimizer::mvsFile;

void DecodeOptimizer::openMvsFile(std::string fileName) {
    printf("AAA\n");
    mvsFile = fopen(fileName.c_str(), "r");
    int v1, v2, v3, v4, v5;
    int res = fscanf(mvsFile, "%d;%d;%d;%d;%d", &v1, &v2, &v3, &v4, &v5);
    printf("[MVS FILE] %d %d %d %d %d %d\n\n", res, v1, v2, v3, v4, v5);
}