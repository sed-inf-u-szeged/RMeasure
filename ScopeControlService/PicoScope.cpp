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

#include "PicoScope.h"
#include <cstring>
#include <thread>

/**
 * Namespace for the PicoScope implementation
 */

namespace ps4000a {

// global variables for CallBackStreaming and getStreamingValues functions
bool    g_ready = false; ///< can be ready to collect data
int32_t     g_sampleCount = 0; ///< collected number of samples
uint32_t     g_startIndex = 0; ///< the current index to the first valid sample in the buffer
int16_t    g_autoStop = 0; ///< a flag to specify if the streaming should stop when all of maxSamples have been taken

/**
 * Callback
 * used by PS4000 data streaimng collection calls, on receipt of data.
 * used to set global flags etc checked by user routines
 */
static void CallBackStreaming
(
    int16_t handle, ///< the handle of device returning the samples
    int32_t noOfSamples, ///< the number of samples to collect
    uint32_t startIndex, ///< an index to the first valid sample in the buffer
    int16_t overflow, ///< returns a set of flags that indicate whether an overvoltage has occure on any of the channels
    uint32_t triggerAt, ///< an index to the buffer indicating the location of the trigger point
    int16_t triggered, ///< a flag indicating whether a trigger occured
    int16_t autoStop, ///< the flag that was set in the call to ps4000aRunStreaming
    void* pParameter ///< a void pointer passed from ps4000aGetStreamingLatestValues.
)
{
    // used for streaming
    g_sampleCount = noOfSamples;
    g_startIndex = startIndex;
    g_autoStop = autoStop;

    // flag to say done reading data
    g_ready = true;
}

PicoScope::PicoScope(const ChannelVector& channels) :
    m_channels(channels),
    m_scopeUnit(NULL)
{
}

PicoScope::~PicoScope()
{
    if (m_scopeUnit) {
        closeUnit();
    }
}

PICO_STATUS PicoScope::openUnit()
{
    // close unit if it was opened earlier
    if (m_scopeUnit)
        closeUnit();

    int16_t handle = 0;
    PICO_STATUS status = ps4000aOpenUnit(&handle, NULL);

    // switch device into non-USB 3.0-power mode
    if (status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
        status = ps4000aChangePowerSource(handle, PICO_USB3_0_DEVICE_NON_USB3_0_PORT);

    if (status == PICO_OK)
    {
        m_scopeUnit = new ScopeUnit(handle, m_channels);
    }

    return status;
}

PICO_STATUS PicoScope::closeUnit()
{
    PICO_STATUS status =  PICO_INVALID_HANDLE;
    if (m_scopeUnit)
    {
        status = ps4000aCloseUnit(m_scopeUnit->handle());
        delete m_scopeUnit;
        m_scopeUnit = NULL;
    }
    return status;
}

PICO_STATUS PicoScope::startStreaming()
{
    PICO_STATUS status =  PICO_INVALID_HANDLE;
    if (m_scopeUnit)
    {
        status = m_scopeUnit->runStreaming(m_channels);
    }
    return status;
}

bool PicoScope::stopStreaming()
{
    if (m_scopeUnit)
    {
        m_scopeUnit->setStreamingMode(false);
        return true;
    }
    return false;
}

const ChannelVector& PicoScope::channels() const
{
    return m_channels;
}

bool PicoScope::setSampleData(const int& sampleInterval, const std::string& sampleUnit)
{
    if (m_scopeUnit) {
        PS4000A_TIME_UNITS timeUnit;

        if (sampleUnit.compare("TIME_FS") == 0)
            timeUnit = PS4000A_FS;
        else if (sampleUnit.compare("TIME_PS") == 0)
            timeUnit = PS4000A_PS;
        else if (sampleUnit.compare("TIME_NS") == 0)
            timeUnit = PS4000A_NS;
        else if(sampleUnit.compare("TIME_US") == 0)
            timeUnit = PS4000A_US;
        else if(sampleUnit.compare("TIME_MS") == 0)
            timeUnit = PS4000A_MS;
        else if (sampleUnit.compare("TIME_S") == 0)
            timeUnit = PS4000A_S;
        else
            return false;

        m_scopeUnit->setSampleData(sampleInterval, timeUnit);
        return true;
    }
    return false;
}

PicoScope::ScopeUnit::ScopeUnit(int16_t handle, const ChannelVector& channels) :
    m_handle(handle),
    m_isStreaming(false),
    m_sampleInterval(1),
    m_timeUnit(PS4000A_MS),
    m_sampleCount(BUFFER_SIZE),
    m_scaleVoltages(true),
    m_measurementList(),
    m_measuredValues(),
    m_chParPort()
{
    short r = 0;
    char line [80];

    // write various information about the specified scope device to a character string (fill UnitInfo)
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_DRIVER_VERSION);
    m_deviceInfo.driverVersion = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_USB_VERSION);
    m_deviceInfo.usbVersion = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_HARDWARE_VERSION);
    m_deviceInfo.hardwareVersion = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_VARIANT_INFO);
    m_deviceInfo.variant = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_BATCH_AND_SERIAL);
    m_deviceInfo.batchAndSerial = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_CAL_DATE);
    m_deviceInfo.calDate = line;
    ps4000aGetUnitInfo(m_handle, (int8_t*)line, sizeof (line), &r, PICO_KERNEL_VERSION);
    m_deviceInfo.kernelVersion = line;

    // set the specific data of the current model (only MODEL_PS4824 is specified)
    switch (atoi(m_deviceInfo.variant.c_str())) {
        case MODEL_PS4824:
            m_modelType = MODEL_PS4824;
            m_external = true;
            m_firstRange = PS4000A_100MV;
            m_lastRange = PS4000A_20V;
            m_signalGenerator = true;
            m_channelNumber = OCTO_SCOPE;
            break;
        default:
            //std::cout << "Unsupported model" << std::endl;
            m_modelType = MODEL_UNKNOWN;
            m_external = false;
            m_firstRange = PS4000A_1V;
            m_lastRange = PS4000A_1V;
            m_signalGenerator = false;
            m_channelNumber = UNKNOWN;
            break;
    }

    if (m_channelNumber == channels.size()) {
        // This function is reserved for future used
        ps4000aSetEts(m_handle, PS4000A_ETS_OFF, 0, 0, NULL); // Turn off ETS
        ChannelVector::const_iterator channelIt = channels.begin();
        int16_t i = 0;
        for (; channelIt != channels.end(); ++channelIt, ++i) {

            ps4000aSetChannel(
                m_handle,
                channelIt->channelType(),
                channelIt->isEnabled(),
                channelIt->coupling(),
                channelIt->range(),
                channelIt->analogOffset()
            );
            if (channelIt->isEnabled() && channelIt->isParport()) {
                m_chParPort.first = channelIt->channelType();
                m_chParPort.second = channelIt->rangeInt();
            }
            //create data buffers
            m_buffers[i * 2] = new int16_t[m_sampleCount];
            m_buffers[i * 2 + 1] = new int16_t[m_sampleCount];
        }
    }
    else {
        for (int i = 0; i < m_channelNumber; ++i) {
             //create data buffers
            m_buffers[i * 2] = new int16_t[m_sampleCount];
            m_buffers[i * 2 + 1] = new int16_t[m_sampleCount];
        }
    }

}

PicoScope::ScopeUnit::~ScopeUnit()
{
    for (int i = 0; i < m_channelNumber; ++i) {
        delete[] m_buffers[i * 2];
        delete[] m_buffers[i * 2 + 1];
    }
}

const int16_t& PicoScope::ScopeUnit::handle() const
{
    return m_handle;
}

const DeviceInfo& PicoScope::ScopeUnit::deviceInfo() const
{
    return m_deviceInfo;
}

const bool& PicoScope::ScopeUnit::isStreaming() const
{
    return m_isStreaming;
}

void PicoScope::ScopeUnit::setStreamingMode(const bool& isStreaming)
{
    m_isStreaming = isStreaming;
}

const MeasuredValuesList& PicoScope::ScopeUnit::measurementList() const
{
    return m_measurementList;
}

const PicoScope::ScopeUnit* PicoScope::scopeUnit() const
{
    return m_scopeUnit;
}

PICO_STATUS PicoScope::ScopeUnit::runStreaming(const ChannelVector& channels, unsigned long preTrigger)
{
    if (m_channelNumber != channels.size())
        return PICO_INVALID_CHANNEL;

    g_autoStop = false;

    m_measurementList.clear();

    MeasurementMap markedMeasurement;
    std::string raw;
    raw.append("The samples collected every " + std::to_string(m_sampleInterval) +" "+ convertTimeUnitToString(m_timeUnit)+"s\n");

    ChannelVector::const_iterator channelIt = channels.begin();

    for(int i = 0; channelIt != channels.end(); ++channelIt, ++i)
    {

        if (channelIt->isEnabled() && !channelIt->isParport()) {
            MeasurementData measurementData;
            MeasuredChannel measuredChannel(channelIt->channelType(), channelIt->hppdl());
            markedMeasurement[measuredChannel] = measurementData;
            raw.append(channelIt->hppdl() + ";");
        }

        // clear data buffers
        std::memset(m_buffers[i * 2], 0, m_sampleCount);
        std::memset(m_buffers[i * 2 + 1], 0, m_sampleCount);

        PICO_STATUS dbStatus = ps4000aSetDataBuffers(m_handle, channelIt->channelType(), m_buffers[i * 2],
                    m_buffers[i * 2 + 1], m_sampleCount, 0, PS4000A_RATIO_MODE_AGGREGATE);
        if (dbStatus != PICO_OK)
        {
            return dbStatus;
        }
    }

    raw.append("\n");
    m_measuredValues.first = markedMeasurement;
    m_measuredValues.second = raw;

    // This function tells the oscilloscope to start collecting data in streaming mode
    PICO_STATUS status = ps4000aRunStreaming(m_handle, &m_sampleInterval, m_timeUnit,
                                preTrigger, 1000000 - preTrigger, true, 1, PS4000A_RATIO_MODE_NONE,
                                m_sampleCount);
    if (status == PICO_OK)
    {
        m_isStreaming = true;
        ChannelVector mychannel = channels;
        std::thread t1 = std::thread(&PicoScope::ScopeUnit::getStreamingValues, this, channels);
        t1.detach();
    }
    return status;
}


void PicoScope::ScopeUnit::getStreamingValues(const ChannelVector& channels)
{
    if (m_channelNumber != channels.size())
        return;

    bool isKernel = false;
    MeasuredValues values;
    while (m_isStreaming && !g_autoStop) {
        /* Poll until data is received. Until then, GetStreamingLatestValues wont call the callback */
        sleep(1);
        g_ready = false;

        double result[m_channelNumber];
        double watt[m_channelNumber];
        PICO_STATUS status = ps4000aGetStreamingLatestValues(m_handle, CallBackStreaming, NULL);
        if (status != PICO_OK)
            break;
        // can be ready and have no data, if autoStop has fired
        if (g_ready && g_sampleCount > 0)
        {
            for (uint32_t i = g_startIndex; i < (g_startIndex + g_sampleCount); i++) {

                // specify the marked measurement part from the parallelport sample
                bool markedPart = false;
                markedPart = adc_to_mv(m_buffers[m_chParPort.first*2][i], m_chParPort.second) > FILTER_NUMBER;

                if (!markedPart && isKernel) {
                    m_measurementList.push_back(values);
                    isKernel = false;

                }

                if (markedPart) {
                    if (!isKernel) {
                        isKernel = true;
                        values = m_measuredValues;
                    }
                    ChannelVector::const_iterator channelsIt = channels.begin();

                    // go over all of the initialized channels
                    for (int j = 0; channelsIt != channels.end(); ++j, ++channelsIt) {

                        // store measurement data if the channel is enabled and not measure a paralell port
                        if (!channelsIt->isParport() && channelsIt->isEnabled()) {
                            MeasuredChannel key(channelsIt->channelType(), channelsIt->hppdl());
                            MeasurementMap::iterator markedIterator =values.first.find(key);

                            if (markedIterator == values.first.end())
                                continue;

                            // specify the millivolt value converted from the digital data
                            result[j] = adc_to_mv(m_buffers[j * 2][i], channelsIt->rangeInt());

                            // specify  the current power
                            watt[j] = ((result[j]/channelsIt->gain())/1000) / channelsIt->resistance() * VOLTAGE;
                            values.second.append(std::to_string(watt[j]) + ";");

                            markedIterator->second.gainElapsedTime((double)m_sampleInterval/convertTimeUnit(m_timeUnit));
                            markedIterator->second.gainEnergy(watt[j] * (double)m_sampleInterval/convertTimeUnit(m_timeUnit));

                            if (markedIterator->second.maxPower() < watt[j])
                                markedIterator->second.setMaxPower(watt[j]);

                            if (markedIterator->second.minPower() > watt[j] || markedIterator->second.minPower() == -1)
                                markedIterator->second.setMinPower(watt[j]);
                        }
                    }
                    values.second.append("\n");
                }
            }
        }
    }

    /* This function stops the scope device from sampling data.
     * Always call this funtion after the end of a capture to ensure that
     * the scope is ready for the next capture
     */
    ps4000aStop(m_handle);
    if (!g_autoStop)
    {
        //std::cout << "Data collection aborted" << std::endl;
    }
}

int PicoScope::ScopeUnit::adc_to_mv(const int16_t& raw,const int& range)
{
    if (!(raw < PS4000A_MAX_VALUE)) {
        // TODO WARNING, raw bigger than max_value, in this case need to set up the channel range
    }
    // ADC to millivolt convert if scaleVoltages is enabled
    return ( m_scaleVoltages ) ? ( raw * range) / PS4000A_MAX_VALUE : raw;
}

unsigned long long int PicoScope::ScopeUnit::convertTimeUnit(const PS4000A_TIME_UNITS timeUnit)
{
    switch (timeUnit) {
        case PS4000A_FS :
            return 1000000000000000;
        case PS4000A_PS :
            return 1000000000000;
        case PS4000A_NS :
            return 1000000000;
        case PS4000A_US :
            return 1000000;
        case PS4000A_MS :
            return 1000;
        case PS4000A_S:
            return 1;
        default :
            return 1;
    }
}

std::string PicoScope::ScopeUnit::convertTimeUnitToString(const PS4000A_TIME_UNITS timeUnit)
{
    switch (timeUnit) {
        case PS4000A_FS :
            return "femtosecond";
        case PS4000A_PS :
            return "picosecond";
        case PS4000A_NS :
            return "nanosecond";
        case PS4000A_US :
            return "microsecond";
        case PS4000A_MS :
            return "millisecond";
        case PS4000A_S:
            return "Second";
        default :
            return "undefined second";
    }
}

void PicoScope::ScopeUnit::setSampleData(const int& sampleInterval, const PS4000A_TIME_UNITS& sampleUnit)
{
    m_sampleInterval = sampleInterval;
    m_timeUnit = sampleUnit;
}

} // namespace ps4000a
