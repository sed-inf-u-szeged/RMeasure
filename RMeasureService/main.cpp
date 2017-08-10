/*
Copyright (c) 2014-2017 University of Szeged

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include "RMeasureServer.h"

static bool runServer = false;

/**
  a signal handler for the Linux signals sent to daemon process,
*/
void signal_handler(int sig)
{
    switch(sig) {
        case SIGHUP:
            runServer = false;
            break;
        case SIGTERM:
            runServer = false;
            break;
        default:
            break;
    }
}

void printUsage()
{
    std::cout << "Usage: rMeasureService --config rMeasureService.cfg" << std::endl;
}

int main(int argc, char *argv[]) {
try {
    std::string configFile;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg.compare("-c")) == 0 || (arg.compare("--config")) == 0) {
            if (i + 1 < argc) {
                configFile = argv[++i];
            } else {
                std::cerr << "--config option requires one argument." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    if (configFile.empty()) {
        std::cerr << "Need to set a config file!" << std::endl;
        printUsage();
        return EXIT_FAILURE;
    }

   pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // child (daemon) continues
    sid = setsid(); // obtain a new process group
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signal_handler); /* catch hangup signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */

    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    runServer = rMeasureServer->create(configFile);

    while (runServer) {
        // server executes no more than one RPC at a time
        rMeasureServer->runOnce();
    }

    rMeasureServer->deleteInstance();
} catch (std::exception const& e) {
    std::cerr << "Something failed. " << e.what() << std::endl;
}
    return EXIT_SUCCESS;
}
