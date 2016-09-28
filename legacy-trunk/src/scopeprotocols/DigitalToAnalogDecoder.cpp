/***********************************************************************************************************************
*                                                                                                                      *
* ANTIKERNEL v0.1                                                                                                      *
*                                                                                                                      *
* Copyright (c) 2012-2016 Andrew D. Zonenberg                                                                          *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Implementation of DigitalToAnalogDecoder
 */

#include "../scopehal/scopehal.h"
#include "DigitalToAnalogDecoder.h"
#include "../scopehal/AnalogRenderer.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

DigitalToAnalogDecoder::DigitalToAnalogDecoder(
	std::string hwname, std::string color, NameServer& namesrvr)
	: ProtocolDecoder(hwname, OscilloscopeChannel::CHANNEL_TYPE_ANALOG, color, namesrvr)
{
	//Set up channels
	m_signalNames.push_back("din");
	m_channels.push_back(NULL);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Factory methods

ChannelRenderer* DigitalToAnalogDecoder::CreateRenderer()
{
	return new AnalogRenderer(this);
}

std::string DigitalToAnalogDecoder::GetProtocolName()
{
	return "Digital to analog";
}

bool DigitalToAnalogDecoder::ValidateChannel(size_t i, OscilloscopeChannel* channel)
{
	if( (i == 0) && (channel->GetType() == OscilloscopeChannel::CHANNEL_TYPE_DIGITAL) )
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actual decoder logic

void DigitalToAnalogDecoder::Refresh()
{	
	//Get the input data
	if(m_channels[0] == NULL)
	{
		SetData(NULL);
		return;
	}
	DigitalBusCapture* din = dynamic_cast<DigitalBusCapture*>(m_channels[0]->GetData());
	if(din == NULL)
	{
		SetData(NULL);
		return;
	}
	
	//Can't do scaling if we have no samples to work with
	if(din->GetDepth() == 0)
	{
		SetData(NULL);
		return;
	}
	
	//Calculate scaling factor
	printf("refreshing\n");
	int width = din->m_samples[0].m_sample.size();
	int64_t nmax = pow(2, width) - 1;
	
	//DAC processing
	AnalogCapture* cap = new AnalogCapture;
	cap->m_timescale = din->m_timescale;
	for(size_t i=0; i<din->m_samples.size(); i++)
	{
		DigitalBusSample sin = din->m_samples[i];
		
		vector<bool>& s = sin.m_sample;
		int64_t ival = 0;
		for(size_t j=0; j<s.size(); j++)
			ival = (ival << 1) | (s[j] ? 1 : 0);
			
		float fval = ival;
		fval /= nmax;
		
		cap->m_samples.push_back(AnalogSample(sin.m_offset, sin.m_duration, fval));
	}
	SetData(cap);
}
