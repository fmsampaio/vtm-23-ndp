#include "DecMVsLogger.h"

FILE* DecMVsLogger::logger_file;
bool DecMVsLogger::isDecoding = false;

void DecMVsLogger::init() {  
    logger_file = fopen("vectors.log", "w"); 
    isDecoding = true;
}

void DecMVsLogger::close() {  
    fclose(logger_file);
}