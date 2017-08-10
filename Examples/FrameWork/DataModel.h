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

#ifndef DATAMODEL_H_INCLUDED
#define DATAMODEL_H_INCLUDED

#include "Program.h"
#include "System.h"

#include <cstdlib>
#include <string>
#include <vector>

/*
 * Namespace for datamodel implementation(s)
 */
namespace datamodel {
/**
 * DataModel implementation which contains applications and system data for the measurements
 */
class DataModel
{
    std::vector<Program> m_programs; ///< the list of the apps
    System m_system; ///< the used system
public:
    /**
     * \brief Constructor, which initialize the datamodel with the data from the config file
     * \param config file which contains information about the measurement
     */
    DataModel(const std::string& configFile);
    /**
     * This function will return with the container of the measured apps
     */
    const std::vector<Program>& programs() const;
    /**
     * This function will return with the data of the used system
     */
    const System& systemData() const;
};

} // namespace datamodel

#endif // DATAMODEL_H_INCLUDED
