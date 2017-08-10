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

#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <string>

/*
 * Namespace for datamodel implementation(s)
 */
namespace datamodel {

class System
{
private:
    std::string m_name; ///< name of the used system
    std::string m_hppdl; ///< contains the HPP-DL description about the system
public:
    System();
    System(const std::string& name, const std::string& hppdl);

    /**
     * \brief A setter function which fill the system information
     * \param name is the name of the used system
     * \param hppdl contains the HPP-DL description about the system
     */
    void setSystem(const std::string& name, const std::string& hppdl);

    /**
     * This function will return with the name of the used system
    */
    const std::string& name() const;

    /**
     * This function will return with the HPP-DL description about the system
    */
    const std::string& hppdl() const;
};

} // namespace datamodel

#endif // SYSTEM_H_INCLUDED
