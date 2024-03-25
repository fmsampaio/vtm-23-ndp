#include "DecodeOptimizer.h"

FILE* DecodeOptimizer::mvsFile;
std::map<std::string, MvLogData*> DecodeOptimizer::mvsDataMap;
std::map<std::string, std::list<MvLogData*> > DecodeOptimizer::mvsDataMapPerCTUWindow;
std::map<std::string, std::pair<int, double> > DecodeOptimizer::prefFracMap;

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

    for(auto it = mvsDataMapPerCTUWindow.begin(); it != mvsDataMapPerCTUWindow.end(); ++it) {
        std::pair<int, double> result = calculatePrefFrac(it->second);
        prefFracMap.insert({it->first, result});
    }   
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
    int yCoord = (yMV << 2) | yMask;

    return std::pair<int,int>(xCoord, yCoord);
}

std::pair<int, double> DecodeOptimizer::calculatePrefFrac(std::list<MvLogData*> list) {
    int countFracPos[16];

    for (int i = 0; i < 16; i++) {
        countFracPos[i] = 0;
    }
    
   
    for(std::list<MvLogData*>::iterator it = list.begin(); it != list.end(); ++ it) {
        countFracPos[(*it)->fracPosition] ++;
    }
 
    int countFracs = 0;
    int prefFrac = -1;
    int maxOcc = -1;

    for (int frac = 1; frac < 16; frac++) {
        countFracs += countFracPos[frac];
        if(countFracPos[frac] > maxOcc) {
            maxOcc = countFracPos[frac];
            prefFrac = frac;
        }
    }

    if(countFracs != 0) {
        double percentFrac = (maxOcc * 1.0) / countFracs;
        return std::pair<int, double>(prefFrac, percentFrac);
    }
    else {
        return std::pair<int, double>(-1, -1);
    }
}

void DecodeOptimizer::modifyMV(int currFramePoc, PosType yPU, int refList, int* xMV, int* yMV) {
    std::string ctuWindowKey = generateKeyPerCTUWindow(currFramePoc, yPU, refList);
    std::pair<int, double> prefFracResult = prefFracMap.at(ctuWindowKey);

    if(prefFracResult.first == -1)
        return;

    int xMask = prefFracResult.first >> 2;
    int yMask = prefFracResult.first & 0x3;

    printf("[%d] (%d,%d) -> ",prefFracResult.first , *xMV, *yMV);

    (*xMV) = ((*xMV) & 0xFFFFFFFC) | xMask;
    (*yMV) = ((*yMV) & 0xFFFFFFFC) | yMask;

    printf("(%d,%d)\n", *xMV, *yMV);    
}