#include <cstdio>
#include <string>

using PosType  = int32_t;
using SizeType = uint32_t;

class DecodeOptimizer {
    private:
        static FILE* mvsFile;

    public:
        static void openMvsFile(std::string fileName);

};