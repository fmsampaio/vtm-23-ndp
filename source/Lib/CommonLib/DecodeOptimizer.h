#include <cstdio>
#include <string>

class DecodeOptimizer {
    private:
        static FILE* mvsFile;

    public:
        static void openMvsFile(std::string fileName);

};