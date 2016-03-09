#include "log.h"

namespace NDnn {


    TLog& TLog::Instance() {
        static TLog _inst;
        return _inst;
    }


}
