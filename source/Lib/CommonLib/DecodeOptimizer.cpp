#include "DecodeOptimizer.h"

FILE* DecodeOptimizer::mvsFile;

void DecodeOptimizer::openMvsFile(std::string fileName) {
    mvsFile = fopen(fileName.c_str(), "r");

    int currFramePoc;
    PosType xPU;
    PosType yPU;
    SizeType wPU;
    SizeType hPU;
    int refList;
    int refFramePoc;
    int xMV;
    int yMV;
    int fracPosition;

    while(!feof(mvsFile)) {
        int res = fscanf(mvsFile, "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n", &currFramePoc, &xPU, &yPU, &wPU, &hPU, &refList, &refFramePoc, &xMV, &yMV, &fracPosition);
        if(res == 0) {
            break;
        }
        printf("[M] %d %d %d %d %d %d %d %d %d %d\n", currFramePoc, xPU, yPU, wPU, hPU, refList, refFramePoc, xMV, yMV, fracPosition);
    }
   
}
