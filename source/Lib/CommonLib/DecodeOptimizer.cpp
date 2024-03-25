#include "DecodeOptimizer.h"

FILE* DecodeOptimizer::mvsFile;
std::map<std::string, MvLogData*> DecodeOptimizer::mvsDataMap;
std::map<std::string, std::list<MvLogData*> > DecodeOptimizer::mvsDataMapPerCTUWindow;

std::string DecodeOptimizer::generateMvLogMapKey(int currFramePoc, PosType xPU, PosType yPU, int refList, int refFramePoc) {
    std::string key = std::to_string(currFramePoc) + "_" +
                          std::to_string(xPU) + "_" +
                          std::to_string(yPU) + "_" +
                          std::to_string(refList) + "_" +
                          std::to_string(refFramePoc);
    
    return key;
}

std::string DecodeOptimizer::generateKeyPerCTUWindow(int currFramePoc, PosType yPU, int refList) {
    int ctuLine = yPU / 128;

    std::string key = std::to_string(currFramePoc) + "_" +
                          std::to_string(ctuLine) + "_" +
                          std::to_string(refList);
    
    return key;
}

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
        MvLogData* mvData = new MvLogData();
        
        mvData->currFramePoc = currFramePoc;
        mvData->xPU = xPU;
        mvData->yPU = yPU;
        mvData->wPU = wPU;
        mvData->hPU = hPU;
        mvData->refList = refList;
        mvData->refFramePoc = refFramePoc;
        mvData->xMV = xMV;
        mvData->yMV = yMV;
        mvData->fracPosition = fracPosition;

        std::string key = generateMvLogMapKey(currFramePoc, xPU, yPU, refList, refFramePoc);
        mvsDataMap.insert({key, mvData});

        std::string keyPerWindow = generateKeyPerCTUWindow(currFramePoc, yPU, refList);
        if(mvsDataMapPerCTUWindow.find(keyPerWindow) != mvsDataMapPerCTUWindow.end()) {
            mvsDataMapPerCTUWindow.at(keyPerWindow).push_back(mvData);
        }
        else {
            std::list<MvLogData*> list;
            list.push_back(mvData);
            mvsDataMapPerCTUWindow.insert({keyPerWindow, list});
        }

    }

    for(auto it = mvsDataMapPerCTUWindow.begin(); it != mvsDataMapPerCTUWindow.end(); it ++) {
        printf("CTU Window (%s) - %lu\n", it->first.c_str(), it->second.size());
    }


    printf("MV data: %lu\n", mvsDataMap.size());
   
}

MvLogData* DecodeOptimizer::getMvData(int currFramePoc, PosType xPU, PosType yPU, int refList, int refFramePoc) {
    std::string key = generateMvLogMapKey(currFramePoc, xPU, yPU, refList, refFramePoc);
    if(mvsDataMap.find(key) != mvsDataMap.end()) {
        MvLogData* mvData = mvsDataMap.at(key);
        return mvData;
    }
    else {
        return NULL;
    }
    
}

std::pair<int, int> DecodeOptimizer::restoreMv(int xMV, int yMV, int fracPosition) {
    int xMask = fracPosition >> 2;
    int yMask = fracPosition & 0x3;

    int xCoord = (xMV << 2) | xMask;
    int yCoord = yMV << 2 | yMask;

    return std::pair<int,int>(xCoord, yCoord);

    

}