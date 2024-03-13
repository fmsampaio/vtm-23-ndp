#include <cstdio>

class DecMVsLogger {
    private:
        static FILE* logger_file;
        static bool isDecoding;
    
    public:
        static void init();
        static void close();

};