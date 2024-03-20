#include <cstdio>
#include <string>
#include <map>
#include <iostream>

using PosType  = int32_t;
using SizeType = uint32_t;

class MvLogData {
    public:
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
};

class DecodeOptimizer {
    private:
        static FILE* mvsFile;
        static std::map<std::string, MvLogData*> mvsDataMap;

    public:
        static void openMvsFile(std::string fileName);

};