#include "DecMVsLogger.h"

FILE* DecMVsLogger::logger_file;
bool DecMVsLogger::decoding = false;

void DecMVsLogger::init() {  
    logger_file = fopen("vectors.log", "w"); 
    setDecoding(true);
}

void DecMVsLogger::close() {  
    fclose(logger_file);
}

void DecMVsLogger::setDecoding(bool isDecoding) {
    decoding = true;
}

bool DecMVsLogger::isDecoding() {
    return decoding;
}

void DecMVsLogger::logMotionVector(
    int currFramePoc,
    PosType xPU, 
    PosType yPU,
    SizeType wPU,
    SizeType hPU,
    int refList,
    int refFramePoc,
    int xMV,
    int yMV          
) {

    fprintf(logger_file, "%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
        currFramePoc, xPU, yPU, wPU, hPU, refList, refFramePoc, xMV, yMV);
    
}