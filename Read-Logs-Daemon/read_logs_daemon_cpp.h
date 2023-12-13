#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <log/log.h>
#include <log/log_read.h>
#include <regex>
#include <cstring>
#include <string>
#include <sstream>
#include <thread>


namespace devtitans::logs {

class ReadLogsDaemonCpp {
public:
    void readMainLogs();
    void readKernelLogs();
    void startDaemon();
};

} // namespace devtitans::logs
