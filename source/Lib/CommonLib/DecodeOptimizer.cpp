#include "DecodeOptimizer.h"

FILE *DecodeOptimizer::mvsFile, *DecodeOptimizer::optLogFile;
std::map<std::string, MvLogData*> DecodeOptimizer::mvsDataMap;
std::map<std::string, std::list<MvLogData*> > DecodeOptimizer::mvsDataMapPerCTUWindow;
std::map<std::string, std::pair<int, double> > DecodeOptimizer::prefFracMap;
std::map<std::string, std::pair<int, double> > DecodeOptimizer::avgMvMap;
long long int DecodeOptimizer::countAdjustedMVs, DecodeOptimizer::totalDecodedMVs;

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
    optLogFile = fopen("decoder-opt.log", "w");

    countAdjustedMVs = 0;
    totalDecodedMVs = 0;

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

        //printf("[I]%s\n", key.c_str());

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

    fprintf(optLogFile, "ctu-window-id;cus-count;avg-mv;pref-frac;avg-mv-hit;pref-frac-hit\n");
    for(auto it = mvsDataMapPerCTUWindow.begin(); it != mvsDataMapPerCTUWindow.end(); ++it) {
        std::pair<int, double> resultPrefFrac = calculatePrefFrac(it->second);
        prefFracMap.insert({it->first, resultPrefFrac});

        std::pair<int, double> resultAvgMV = calculateAvgMV(it->second);
        avgMvMap.insert({it->first, resultAvgMV});

        int prefFrac = resultPrefFrac.first;
        double prefFracHit = resultPrefFrac.second;
        int avgMv = resultAvgMV.first;
        double avgMvHit = resultAvgMV.second;

        int cusWithinWindow = it->second.size();
        std::string ctuWindowKey = it->first;

        fprintf(optLogFile, "%s;%d;%d;%.3f;%d;%.3f\n", ctuWindowKey.c_str(), cusWithinWindow, avgMv, avgMvHit, prefFrac, prefFracHit);
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
        countFracPos[(*it)->fracPosition] += (*it)->wPU * (*it)->hPU;
    }
 
    int countFracArea = 0;
    int prefFrac = -1;
    int maxOcc = -1;

    for (int frac = 1; frac < 16; frac++) {
        countFracArea += countFracPos[frac];
        if(countFracPos[frac] > maxOcc) {
            maxOcc = countFracPos[frac];
            prefFrac = frac;
        }
    }

    if(countFracArea != 0) {
        double percentFrac = (maxOcc * 1.0) / countFracArea;
        return std::pair<int, double>(prefFrac, percentFrac);
    }
    else {
        return std::pair<int, double>(-1, -1);
    }
}

std::pair<int, double> DecodeOptimizer::calculateAvgMV(std::list<MvLogData*> list) {
    int yCount = 0;
    int accumFracPUs = 0;
    
    for(std::list<MvLogData*>::iterator it = list.begin(); it != list.end(); ++ it) {
        bool isFrac = (*it)->fracPosition != 0;
        if(isFrac) {
            accumFracPUs ++;
            yCount += (*it)->yMV;
        }
    }

    
    bool isNeg = yCount < 0;

    double yAvgDouble = abs(yCount) * 1.0 / accumFracPUs;
    int yAvg = (isNeg ? -round(yAvgDouble) : round(yAvgDouble));
    
    //yAvg = 0; //for debug!

    int yTop = yAvg;
    int yBottom = yAvg + 128;

    int accumMVsInsideInterpWindow = 0;
    for(std::list<MvLogData*>::iterator it = list.begin(); it != list.end(); ++ it) {
        int yMV = (*it)->yMV;
        SizeType hPU = (*it)->hPU;
        bool isFrac = (*it)->fracPosition != 0;

        if(isFrac) {
            if(yMV >= yTop && (yMV + hPU) < yBottom) {
                accumMVsInsideInterpWindow ++;
            }
        }
    }

    if(accumMVsInsideInterpWindow == 0) {
        return std::pair<int, double>(-1, -1);
    }
    else {
        double percentInsideInterpWindow = (accumMVsInsideInterpWindow * 1.0) / accumFracPUs;
        return std::pair<int, double>(yAvg, percentInsideInterpWindow);
    }

}

int DecodeOptimizer::extractIntegAndFrac(int *xCoord, int *yCoord) {
    int xMasked = *(xCoord) & 0x3;
    int yMasked = *(yCoord) & 0x3;
    int fracPosition = (xMasked << 2) | yMasked;

    // for debug
    // std::cout << *xCoord << " " << *yCoord << " --> ";

    *xCoord = *xCoord >> 2;
    *yCoord = *yCoord >> 2;

    // for debug
    // std::cout << *xCoord << " " << *yCoord << " " << fracPosition << "\n";

    return fracPosition;
}

void DecodeOptimizer::modifyMV(int currFramePoc, PosType xPU, PosType yPU, SizeType hPU, int refList, int refFramePoc, int* xMV, int* yMV) {
    int xMVInteg = *xMV, yMVInteg = *yMV;

    int fracPosition = extractIntegAndFrac(&xMVInteg, &yMVInteg);
       
    std::string ctuWindowKey = generateKeyPerCTUWindow(currFramePoc, yPU, refList);

    if(prefFracMap.find(ctuWindowKey) == prefFracMap.end() || avgMvMap.find(ctuWindowKey) == avgMvMap.end()) {
        return;
    }

    std::pair<int, double> prefFracResult = prefFracMap.at(ctuWindowKey);
    std::pair<int, double> avgMVResult = avgMvMap.at(ctuWindowKey);

    bool isFrac = fracPosition != 0;

    int xMVBkp = *xMV;
    int yMVBkp = *yMV;

    totalDecodedMVs ++;
    
    if(prefFracResult.first == -1 || avgMVResult.first == -1 || !isFrac)
        return;

    int yTop = avgMVResult.first;
    int yBottom = avgMVResult.first + 128;

    bool adjustMV = false;    
    int adjustedYMV = 0;

    if(yMVInteg < yTop) {
        adjustMV = true;
        adjustedYMV = yTop;
    }
    else {
        if((yMVInteg + hPU) > yBottom) {
            adjustMV = true;
            adjustedYMV = yBottom - hPU;
        }
    }

    if(adjustMV) {
        int yIntegMask = adjustedYMV << 2;
        (*yMV) = yIntegMask;
    }

    // Adjusting frac position to the prefFrac of the current CTU Window
    int xFracMask = prefFracResult.first >> 2;
    int yFracMask = prefFracResult.first & 0x3;

    (*xMV) = ((*xMV) & 0xFFFFFFFC) | xFracMask;
    (*yMV) = ((*yMV) & 0xFFFFFFFC) | yFracMask;

    if((*xMV) != xMVBkp || (*yMV) != yMVBkp) {
        countAdjustedMVs ++;

        // for debug
        // int xMasked = *(xMV) & 0x3;
        // int yMasked = *(yMV) & 0x3;
        // int fracPositionAdjusted = (xMasked << 2) | yMasked;

        // std::cout << "[" << prefFracResult.first << " x " << fracPosition << " x " << fracPositionAdjusted << "] ";
        // std::cout << "[" << yTop << "," << yBottom << "] [" << yMVInteg << "," << hPU << " x " << adjustedYMV <<  "] "; 
        // std::cout << "(" << xMVBkp << "," << yMVBkp << ") -> \t (" << (*xMV) << "," << (*yMV) << ")\n";
        
    }
}

void DecodeOptimizer::logDecoderOptSummary() {
    double adjustedMvsPercent = (countAdjustedMVs * 1.0) / totalDecodedMVs;
        
    fprintf(optLogFile, "adj-mvs-pctg;%.2f\n", adjustedMvsPercent);

    fclose(optLogFile);

}