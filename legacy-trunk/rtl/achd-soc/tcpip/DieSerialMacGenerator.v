`timescale 1ns / 1ps
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
	@brief Generates a MAC address based on the FPGA die serial number
 */
module DieSerialMacGenerator(
	
	//Clocks
	clk,

	//NoC interface
	rpc_tx_en, rpc_tx_data, rpc_tx_ack, rpc_rx_en, rpc_rx_data, rpc_rx_ack
	);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// I/O declarations
	
	//Clocks
	input wire			clk;
	
	//NoC interface
	output wire			rpc_tx_en;
	output wire[31:0]	rpc_tx_data;
	input wire[1:0]		rpc_tx_ack;
	input wire			rpc_rx_en;
	input wire[31:0]	rpc_rx_data;
	output wire[1:0]	rpc_rx_ack;	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NoC transceivers
	
	`include "RPCv2Router_type_constants.v"	//Pull in autogenerated constant table
	`include "RPCv2Router_ack_constants.v"
	
	parameter NOC_ADDR = 16'h0000;
	
	reg			rpc_fab_tx_en 		= 0;
	reg[15:0]	rpc_fab_tx_dst_addr	= 0;
	reg[7:0]	rpc_fab_tx_callnum	= 0;
	reg[2:0]	rpc_fab_tx_type		= 0;
	reg[20:0]	rpc_fab_tx_d0		= 0;
	reg[31:0]	rpc_fab_tx_d1		= 0;
	reg[31:0]	rpc_fab_tx_d2		= 0;
	wire		rpc_fab_tx_done;
	
	wire		rpc_fab_rx_inbox_full;
	wire[15:0]	rpc_fab_rx_src_addr;
	wire[15:0]	rpc_fab_rx_dst_addr;
	wire[7:0]	rpc_fab_rx_callnum;
	wire[2:0]	rpc_fab_rx_type;
	wire[20:0]	rpc_fab_rx_d0;
	wire[31:0]	rpc_fab_rx_d1;
	wire[31:0]	rpc_fab_rx_d2;
	reg			rpc_fab_rx_done		= 0;
	
	RPCv2Transceiver #(
		.LEAF_PORT(1),
		.LEAF_ADDR(NOC_ADDR)
	) txvr(
		.clk(clk),
		
		.rpc_tx_en(rpc_tx_en),
		.rpc_tx_data(rpc_tx_data),
		.rpc_tx_ack(rpc_tx_ack),
		
		.rpc_rx_en(rpc_rx_en),
		.rpc_rx_data(rpc_rx_data),
		.rpc_rx_ack(rpc_rx_ack),
		
		.rpc_fab_tx_en(rpc_fab_tx_en),
		.rpc_fab_tx_src_addr(16'h0000),
		.rpc_fab_tx_dst_addr(rpc_fab_tx_dst_addr),
		.rpc_fab_tx_callnum(rpc_fab_tx_callnum),
		.rpc_fab_tx_type(rpc_fab_tx_type),
		.rpc_fab_tx_d0(rpc_fab_tx_d0),
		.rpc_fab_tx_d1(rpc_fab_tx_d1),
		.rpc_fab_tx_d2(rpc_fab_tx_d2),
		.rpc_fab_tx_done(rpc_fab_tx_done),
		
		.rpc_fab_inbox_full(rpc_fab_rx_inbox_full),
		.rpc_fab_rx_en(),
		.rpc_fab_rx_src_addr(rpc_fab_rx_src_addr),
		.rpc_fab_rx_dst_addr(rpc_fab_rx_dst_addr),
		.rpc_fab_rx_callnum(rpc_fab_rx_callnum),
		.rpc_fab_rx_type(rpc_fab_rx_type),
		.rpc_fab_rx_d0(rpc_fab_rx_d0),
		.rpc_fab_rx_d1(rpc_fab_rx_d1),
		.rpc_fab_rx_d2(rpc_fab_rx_d2),
		.rpc_fab_rx_done(rpc_fab_rx_done)
		);
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Retransmit timer (waiting for die serial to be ready)
	
	reg[31:0]	delay_count	= 0;
	reg			delay_wrap	= 0;
	
	always @(posedge clk) begin
		
		//Bump the counter
		delay_count	<= delay_count + 1;
		
		//If we hit the limit, wrap around
		delay_wrap	<= 0;
		if(delay_count == 32'h00020000) begin
			delay_count		<= 0;
			delay_wrap		<= 1;
		end
		
	end
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Main state machine
	
	`include "NOCSysinfo_constants.v"
	`include "NOCNameServer_constants.v"
	`include "IPv6OffloadEngine_opcodes_constants.v"
	
	localparam		STATE_SEARCH_0	= 0;
	localparam		STATE_SEARCH_1	= 1;
	localparam		STATE_SEARCH_2	= 2;
	localparam		STATE_SEARCH_3	= 3;
	localparam		STATE_GETMAC_0	= 4;
	localparam		STATE_GETMAC_1	= 5;
	localparam		STATE_GETMAC_2	= 6;
	localparam		STATE_CONFIG_0	= 7;
	localparam		STATE_CONFIG_1	= 8;
	localparam		STATE_HANG		= 9;
	
	reg[3:0] state 			= STATE_SEARCH_0;
	
	reg[15:0] sysinfo_addr	= 0;
	reg[15:0]ipv6_addr		= 0;

	always @(posedge clk) begin
	
		rpc_fab_tx_en	<= 0;
		rpc_fab_rx_done <= 0;
		
		case(state)
		
			////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// HANG - either done doing stuff, or fatal error during init
			
			STATE_HANG: begin
			end	//end STATE_HANG
		
			////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// SEARCH - find sysinfo and ipv6
			
			//Search for sysinfo
			STATE_SEARCH_0: begin
				rpc_fab_tx_en		<= 1;
				rpc_fab_tx_dst_addr	<= NAMESERVER_ADDR;
				rpc_fab_tx_type		<= RPC_TYPE_CALL;
				rpc_fab_tx_callnum	<= NAMESERVER_FQUERY;
				rpc_fab_tx_d0		<= 0;
				rpc_fab_tx_d1		<= "sysi";
				rpc_fab_tx_d2		<= {"nfo", 8'h0};
				state				<= STATE_SEARCH_1;
			end	//end STATE_SEARCH_0
			
			//Wait for response from name server
			STATE_SEARCH_1: begin		
				if(rpc_fab_rx_inbox_full) begin
				
					rpc_fab_rx_done				<= 1;
				
					case(rpc_fab_rx_type)
						
						RPC_TYPE_RETURN_FAIL: 
							state				<= STATE_HANG;
					
						RPC_TYPE_RETURN_SUCCESS: begin
							sysinfo_addr		<= rpc_fab_rx_d0[15:0];
							state 				<= STATE_SEARCH_2;								
						end
						
						RPC_TYPE_RETURN_RETRY:
							rpc_fab_tx_en	<= 1;
						
					endcase
					
				end	
			end	//end STATE_SEARCH_1
			
			//Search for sysinfo
			STATE_SEARCH_2: begin
				rpc_fab_tx_en		<= 1;
				rpc_fab_tx_dst_addr	<= NAMESERVER_ADDR;
				rpc_fab_tx_type		<= RPC_TYPE_CALL;
				rpc_fab_tx_callnum	<= NAMESERVER_FQUERY;
				rpc_fab_tx_d0		<= 0;
				rpc_fab_tx_d1		<= "ipv6";
				rpc_fab_tx_d2		<= 0;
				state				<= STATE_SEARCH_3;
			end	//end STATE_SEARCH_2
			
			//Wait for response from name server
			STATE_SEARCH_3: begin		
				if(rpc_fab_rx_inbox_full) begin
				
					rpc_fab_rx_done				<= 1;
				
					case(rpc_fab_rx_type)
						
						RPC_TYPE_RETURN_FAIL: 
							state				<= STATE_HANG;
					
						RPC_TYPE_RETURN_SUCCESS: begin
							ipv6_addr			<= rpc_fab_rx_d0[15:0];
							state 				<= STATE_GETMAC_0;								
						end
						
						RPC_TYPE_RETURN_RETRY:
							rpc_fab_tx_en	<= 1;
						
					endcase
					
				end	
			end	//end STATE_SEARCH_3
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// GETMAC - look up the serial number
			
			//Ask for the serial
			STATE_GETMAC_0: begin
				rpc_fab_tx_en		<= 1;
				rpc_fab_tx_dst_addr	<= sysinfo_addr;
				rpc_fab_tx_type		<= RPC_TYPE_CALL;
				rpc_fab_tx_callnum	<= SYSINFO_CHIP_SERIAL;
				rpc_fab_tx_d0		<= 0;
				rpc_fab_tx_d1		<= 0;
				rpc_fab_tx_d2		<= 0;
				state				<= STATE_GETMAC_1;
			end	//end STATE_GETMAC_0
			
			//Wait for sysinfo to respond
			STATE_GETMAC_1: begin
				if(rpc_fab_rx_inbox_full) begin
				
					rpc_fab_rx_done				<= 1;
				
					case(rpc_fab_rx_type)
							
						RPC_TYPE_RETURN_FAIL:
							state				<= STATE_HANG;
					
						RPC_TYPE_RETURN_SUCCESS: begin
						
							//Set MAC based on die serial number
							//Some messy mangling required here due to quirks of NOCSysinfo
							//TODO: Try to make this a bit saner??
							rpc_fab_tx_en		<= 1;
							rpc_fab_tx_dst_addr	<= ipv6_addr;
							rpc_fab_tx_type		<= RPC_TYPE_CALL;
							rpc_fab_tx_callnum	<= IPV6_OP_SET_MAC;
							rpc_fab_tx_d0		<= {8'h02, rpc_fab_rx_d2[7:0]};
							rpc_fab_tx_d1		<= rpc_fab_rx_d1;
							rpc_fab_tx_d2		<= 0;
						
							state 				<= STATE_CONFIG_0;
						end
						
						RPC_TYPE_RETURN_RETRY:
							state				<= STATE_GETMAC_2;
						
					endcase
				end
				
			end	//end STATE_GETMAC_1
			
			STATE_GETMAC_2: begin
				if(delay_wrap) begin
					rpc_fab_tx_en		<= 1;
					state				<= STATE_GETMAC_1;
				end
			end	//end STATE_GETMAC_2
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Wait for return value
			
			STATE_CONFIG_0: begin
			
				if(rpc_fab_rx_inbox_full) begin
				
					rpc_fab_rx_done				<= 1;
				
					case(rpc_fab_rx_type)
							
						RPC_TYPE_RETURN_FAIL:
							state				<= STATE_HANG;
					
						RPC_TYPE_RETURN_SUCCESS:
							state 				<= STATE_HANG;
						
						RPC_TYPE_RETURN_RETRY:
							state				<= STATE_CONFIG_1;
						
					endcase
				end
				
			end	//end STATE_CONFIG_0
			
			STATE_CONFIG_1: begin
				if(delay_wrap) begin
					rpc_fab_tx_en		<= 1;
					state				<= STATE_CONFIG_1;
				end
			end	//end STATE_CONFIG_1
		
		endcase	
		
	end
	
endmodule