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

#include "SourceCapability.h"

namespace repara {
namespace measurement {

SourceCapability::SourceCapability(Type v) : _value(v)
{
}

bool SourceCapability::operator==(SourceCapability that) const
{
    return _value == that._value;
}

bool SourceCapability::operator!=(SourceCapability that) const
{
    return !(*this == that);
}

bool SourceCapability::operator<(SourceCapability that) const
{
    return _value < that._value;
}

/*
 * The values behind the objects. Not to be relied on, they may change in
 * later versions of the API.
 */
const SourceCapability SourceCapability::Energy(1 << 0);
const SourceCapability SourceCapability::MinimumPower(1 << 1);
const SourceCapability SourceCapability::AveragePower(1 << 2);
const SourceCapability SourceCapability::MaximumPower(1 << 3);
const SourceCapability SourceCapability::ElapsedTime(1 << 4);
const SourceCapability SourceCapability::KernelTime(1 << 5);
const SourceCapability SourceCapability::UserTime(1 << 6);

SourceCapabilities::SourceCapabilities(Type s) : _set(s)
{
}

SourceCapabilities::SourceCapabilities() : _set(0)
{
}

SourceCapabilities::SourceCapabilities(SourceCapability c) : _set(c._value)
{
}

SourceCapabilities::SourceCapabilities(const SourceCapabilities &s) : _set(s._set)
{
}

bool SourceCapabilities::operator==(SourceCapabilities that) const
{
    return _set == that._set;
}

bool SourceCapabilities::operator!=(SourceCapabilities that) const
{
    return !(*this == that);
}

SourceCapabilities SourceCapabilities::operator|(SourceCapabilities s)
{
    return SourceCapabilities(_set | s._set);
}

SourceCapabilities& SourceCapabilities::operator|=(SourceCapabilities s)
{
    _set |= s._set;
    return *this;
}

bool SourceCapabilities::check(SourceCapabilities pattern) const
{
    return  (_set & pattern._set) == pattern._set;
}

/* The empty set of capabilities. */
const SourceCapabilities SourceCapabilities::None;

} // namespace repara::measurement
} // namespace repara
