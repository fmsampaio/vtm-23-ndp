#include <cstdio>
#include "CommonDef.h"

using PosType  = int32_t;
using SizeType = uint32_t;

class DecMVsLogger {
    private:
        static FILE* logger_file;
        static bool decoding;
    
    public:
        static void init();
        static void close();

        static void setDecoding(bool isDecoding);
        static bool isDecoding();

        static void logMotionVector(
            int currFramePoc,
            PosType xPU, 
            PosType yPU,
            SizeType wPU,
            SizeType hPU,
            int refList,
            int refFramePoc,
            int xMV,
            int yMV         
        );

        static int extractIntegAndFrac(int *xCoord, int *yCoord);

};