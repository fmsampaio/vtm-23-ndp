#include <cstdio>
#include <string>
#include <map>
#include <list>
#include <iostream>
#include <cmath>

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
        static FILE *mvsFile, *optLogFile;
        static std::map<std::string, MvLogData*> mvsDataMap;
        static std::map<std::string, std::list<MvLogData*> > mvsDataMapPerCTUWindow;
        static std::map<std::string, std::pair<int, double> > prefFracMap;
        static std::map<std::string, std::pair<int, double> > avgMvMap;

        static long long int countAdjustedMVs;
        static long long int totalDecodedMVs;

        static bool isFracOnly;

    public:
        static void openMvsFile(std::string fileName);
        static void setOptMode(int cfgFracOnly);
        static std::string generateMvLogMapKey(int currFramePoc, PosType xPU, PosType yPU, int refList, int refFramePoc);
        static std::string generateKeyPerCTUWindow(int currFramePoc, PosType yPU, int refList);
        static MvLogData* getMvData(int currFramePoc, PosType xPU, PosType yPU, int refList, int refFramePoc);
        static std::pair<int, int> restoreMv(int xMV, int yMV, int fracPosition);
        static std::pair<int, double> calculatePrefFrac(std::list<MvLogData*> list);
        static std::pair<int, double> calculateAvgMV(std::list<MvLogData*> list);
        static void modifyMV(int currFramePoc, PosType xPU, PosType yPU, SizeType hPU, int refList, int refFramePoc, int* xMV, int* yMV);
        static void logDecoderOptSummary();
        static int extractIntegAndFrac(int *xCoord, int *yCoord);
};