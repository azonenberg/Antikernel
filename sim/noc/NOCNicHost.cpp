/***********************************************************************************************************************
*                                                                                                                      *
* ANTIKERNEL v0.1                                                                                                      *
*                                                                                                                      *
* Copyright (c) 2012-2017 Andrew D. Zonenberg                                                                          *
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
	@brief A host that simulates an Ethernet interface
 */

#include "nocsim.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

NOCNicHost::NOCNicHost(uint16_t addr, NOCRouter* parent, xypos pos)
	: NOCHost(addr, parent, pos)
	, m_rxstate(RX_STATE_IDLE)
	, m_frameBuffers(0)
	, m_cyclesIdle(0)
	, m_cyclesWaitingForMalloc(0)
	, m_cyclesWaitingForWrite(0)
	, m_cyclesWaitingForChown(0)
	, m_cyclesWaitingForSend(0)
{

}

NOCNicHost::~NOCNicHost()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Simulation

void NOCNicHost::PrintStats()
{
	LogDebug("[NIC] Cycles spent:\n");
	LogIndenter li;
	LogDebug("Idle                   : %5lu (%5.2f %%)\n",
		m_cyclesIdle, (m_cyclesIdle * 100.0f) / g_time);
	LogDebug("Waiting for RPC malloc : %5lu (%5.2f %%)\n",
		m_cyclesWaitingForMalloc, (m_cyclesWaitingForMalloc * 100.0f) / g_time);
	LogDebug("Waiting for DMA write  : %5lu (%5.2f %%)\n",
		m_cyclesWaitingForWrite, (m_cyclesWaitingForWrite * 100.0f) / g_time);
	LogDebug("Waiting for RPC chown  : %5lu (%5.2f %%)\n",
		m_cyclesWaitingForChown, (m_cyclesWaitingForChown * 100.0f) / g_time);
	LogDebug("Waiting for RPC IRQ    : %5lu (%5.2f %%)\n",
		m_cyclesWaitingForSend, (m_cyclesWaitingForSend * 100.0f) / g_time);
}

bool NOCNicHost::AcceptMessage(NOCPacket packet, SimNode* /*from*/)
{
	//We got this message
	//LogDebug("[%5u] NOCNicHost %04x: accepting %d-word message from %04x\n",
	//	g_time, m_address, packet.m_size, packet.m_from);
	packet.Processed();

	//State transitions based on incoming messages
	switch(m_rxstate)
	{
		case RX_STATE_IDLE:
			break;

		case RX_STATE_WAIT_ALLOC:

			//If we get a message from RAM, we have a new frame buffer
			if( (packet.m_type == NOCPacket::TYPE_RPC_RETURN) && (packet.m_from == RAM_ADDR) )
			{
				m_frameBuffers ++;
				m_rxstate = RX_STATE_IDLE;
			}

			break;

		case RX_STATE_WAIT_WRITE:
			break;

		case RX_STATE_WAIT_CHOWN:
			break;

		case RX_STATE_WAIT_SEND:
			break;
	}

	/*
	//If we get a DMA data message and were waiting on RAM, then unblock
	if( (m_state == RX_STATE_WAIT_RAM) && (packet.m_type == NOCPacket::TYPE_DMA_RDATA) )
	{
		LogDebug("[%5u] Got cache line, unblocking CPU\n", g_time);
		m_state = RX_STATE_EXECUTING;
	}
	*/

	return true;
}

void NOCNicHost::Timestep()
{
	//At time 0: generate an RPC request to allocate a page of RAM
	if(g_time == 0)
	{
		LogDebug("[%5u] Sending initial malloc request\n", g_time);

		NOCPacket message(m_address, RAM_ADDR, 4, NOCPacket::TYPE_RPC_CALL, 4);
		if(!m_parent->AcceptMessage(message, this))
			LogWarning("Couldn't send initial message\n");
		m_rxstate = RX_STATE_WAIT_ALLOC;
	}

	//Main state machine
	switch(m_rxstate)
	{
		case RX_STATE_IDLE:
			m_cyclesIdle ++;
			break;

		case RX_STATE_WAIT_ALLOC:
			m_cyclesWaitingForMalloc ++;
			break;

		case RX_STATE_WAIT_WRITE:
			m_cyclesWaitingForWrite ++;
			break;

		case RX_STATE_WAIT_CHOWN:
			m_cyclesWaitingForChown ++;
			break;

		case RX_STATE_WAIT_SEND:
			m_cyclesWaitingForSend ++;
			break;
	}

	/*
	//If executing, do stuff
	if(m_state == RX_STATE_EXECUTING)
	{
		//For now: 1% L1 cache miss rate
		if(0 == (rand() % 100) )
		{
			LogDebug("[%5u] Cache miss, requesting new data\n", g_time);
			NOCPacket message(m_address, RAM_ADDR, 4, NOCPacket::TYPE_DMA_READ, 64);
			if(!m_parent->AcceptMessage(message, this))
				LogWarning("Couldn't send RAM read message\n");

			m_state = RX_STATE_WAIT_RAM;
		}

		//TODO: talk to peripherals
	}
	*/
}