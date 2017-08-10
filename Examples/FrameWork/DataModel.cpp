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

#include "DataModel.h"

#include <iostream>
#include <libconfig.h++>
#include <fstream>

using namespace libconfig;

namespace datamodel {

DataModel::DataModel(const std::string& configFile)
{
    try {
        Config cfg;
        // Read the file. If there is an error, report it.
        cfg.readFile(configFile.c_str());

        std::string systemName = cfg.lookup("system.name");
        std::string systemHPPDL = cfg.lookup("system.hppdlFile");

#ifdef SCOPE
        std::string scopeService = cfg.lookup("scopeService");
        setenv("SCOPESERVICE",scopeService.c_str(),1);
#endif

#if defined (SCOPE) || defined (RAPL)
        std::string rmeasureService = cfg.lookup("rmeasureService");
        setenv("RMEASURESERVICE",rmeasureService.c_str(),1);
#endif

        std::ifstream ifs(systemHPPDL.c_str());
        std::string hppdlInfo( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()) );

        m_system.setSystem(systemName, hppdlInfo);

        const Setting& root = cfg.getRoot();
        const Setting &programs = root["programs"];
        int count = programs.getLength();

        for(int i = 0; i < count; ++i) {

            std::string programName, inputName;
            std::string envName, envVersion, envParams;
            std::string inputTarget, runPath, targetPath, deviceType;

            const Setting& programSetting = programs[i];

            if(!( programSetting.lookupValue("name", programName)
                && programSetting.lookupValue("inputTarget", inputTarget)
                && programSetting.lookupValue("inputName", inputName)
                && programSetting.lookupValue("envName", envName)
                && programSetting.lookupValue("envVersion", envVersion)
                && programSetting.lookupValue("envParams", envParams)
                && programSetting.lookupValue("targetDeviceType", deviceType)
                && programSetting.lookupValue("runPath", runPath)
                && programSetting.lookupValue("targetPath", targetPath)))
                continue;

            Program program;
            program.setProgramName(programName);
            program.setInputName(inputName);
            program.setEnviroment(envName, envVersion, envParams);
            program.setTargets(runPath, targetPath, inputTarget, deviceType);
            m_programs.push_back(program);

        }
    }
    catch(const FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
    }
    catch(const SettingNotFoundException &nfex)
    {
        std::cerr << "Settings not found!" << std::endl;
    }
}

const std::vector<Program>& DataModel::programs() const
{
    return m_programs;
}

const System& DataModel::systemData() const
{
    return m_system;
}

} // namespace datamodel
