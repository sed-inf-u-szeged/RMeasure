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

#ifndef SCOPECONTROLSERVER_H_INCLUDED
#define SCOPECONTROLSERVER_H_INCLUDED

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "PicoScope.h"

/*
 * A Method class to handle that function which opens the scope device
 */
class PicoOpenMethod : public xmlrpc_c::method {
public:
    PicoOpenMethod();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to handle that function which close the scope device
 */
class PicoCloseMethod : public xmlrpc_c::method {
public:
    PicoCloseMethod();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to ensure information about the specified scope device
 */
class PicoGetDeviceInfo : public xmlrpc_c::method {
public:
    PicoGetDeviceInfo();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to ensure information about the scope channels characteristics
 */
class PicoChannelSettings : public xmlrpc_c::method {
public:
    PicoChannelSettings();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to handle streaming function
 */
class PicoStartStreaming : public xmlrpc_c::method {
public:
    PicoStartStreaming();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to stop streaming function
 */
class PicoStopStreaming : public xmlrpc_c::method {
public:
    PicoStopStreaming();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to ensure results from the measurements
 */
class PicoGetValues : public xmlrpc_c::method {
public:
    PicoGetValues();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to ensure raw data from the measurements
 */
class PicoRawData : public xmlrpc_c::method {
public:
    PicoRawData();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

/*
 * A Method class to ensure the configuration of the sample rating in streaming mode
 */
class PicoSetSample : public xmlrpc_c::method {
public:
    PicoSetSample();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP);
};

class ScopeControlServer {
    ScopeControlServer();
    ~ScopeControlServer();
    ScopeControlServer(const ScopeControlServer&) = delete;
    void operator=(const ScopeControlServer&)  = delete;

    unsigned int m_portNumber;
    std::string m_logFile;
    unsigned int m_keepaliveTimeout;
    unsigned int m_keepaliveMaxConn;
    unsigned int m_timeout;
    bool m_dontAdvertise;
    xmlrpc_c::registry m_registry; ///< registry object for the server
    xmlrpc_c::serverAbyss* m_abyssServer;
    ps4000a::PicoScope* m_picoscope;

   static ScopeControlServer* s_instance;

public:
    static ScopeControlServer* instance();
    static void deleteInstance();


    const std::string& logFile();
    bool create(const std::string& configName = "");
    void runOnce();

    bool openScope();
    bool closeScope();
    bool streaming(const bool run);
    bool setSampleData(const int sampleInterval, const std::string& sampleUnit);

    const ps4000a::PicoScope* picoScope() const;

};

#endif //SCOPECONTROLSERVER_H_INCLUDED
