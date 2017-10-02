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
	@brief Program entry point
 */

#include "scopeclient.h"
#include "MainWindow.h"
#include "ScopeConnectionDialog.h"
//#include "../scopehal/NetworkedOscilloscope.h"
#include "../scopehal/RedTinLogicAnalyzer.h"
#include "../scopehal/LeCroyVICPOscilloscope.h"
#include "../scopeprotocols/scopeprotocols.h"

using namespace std;

int main(int argc, char* argv[])
{
	int exit_code = 0;
	try
	{
		Gtk::Main kit(argc, argv);

		//Global settings
		unsigned short port = 0;
		string server = "";
		string api = "redtin_uart";
		//bool scripted = false;
		string scopename = "";
		string tty = "/dev/ttyUSB0";

		Severity console_verbosity = Severity::NOTICE;

		//Parse command-line arguments
		for(int i=1; i<argc; i++)
		{
			string s(argv[i]);

			//Let the logger eat its args first
			if(ParseLoggerArguments(i, argc, argv, console_verbosity))
				continue;

			if(s == "--help")
			{
				//not implemented
				return 0;
			}
			else if(s == "--port")
				port = atoi(argv[++i]);
			else if(s == "--server")
				server = argv[++i];
			else if(s == "--api")
				api = argv[++i];
			//else if(s == "--scripted")
			//	scripted = true;
			else if(s == "--scopename")
				scopename = argv[++i];
			else if(s == "--tty")
				tty = argv[++i];
			else if(s == "--version")
			{
				//not implemented
				//ShowVersion();
				return 0;
			}
			else
			{
				fprintf(stderr, "Unrecognized command-line argument \"%s\", use --help\n", s.c_str());
				return 1;
			}
		}

		//Set up logging
		g_log_sinks.emplace(g_log_sinks.begin(), new ColoredSTDLogSink(console_verbosity));

		//Initialize the protocol decoder library
		ScopeProtocolStaticInit();

		//Connect to the server
		Oscilloscope* scope = NULL;
		//NameServer* namesrvr = NULL;
		/*if(api == "scoped")
			scope = new NetworkedOscilloscope(server, port);
		else if(api == "redtin")
		{
			//Not scripting? Normal dialog process
			if(!scripted)
			{
				ScopeConnectionDialog dlg(server, port);
				if(Gtk::RESPONSE_OK != dlg.run())
					return 0;

				namesrvr = dlg.DetachNameServer();
				scope = dlg.DetachScope();
			}
			else
			{
				RedTinLogicAnalyzer* la = new RedTinLogicAnalyzer(server, port);
				namesrvr = new NameServer(&la->m_iface);
				la->Connect(scopename);
				scope = la;
			}
		}*/
		if(api == "redtin_uart")
		{
			//TODO
			scope = new RedTinLogicAnalyzer(tty, 115200);
		}
		else if(api == "lecroy_vicp")
		{
			//default port if not specified
			if(port == 0)
				port = 1861;
			scope = new LeCroyVICPOscilloscope(server, port);
		}
		else
		{
			LogError("Unrecognized API \"%s\", use --help\n", api.c_str());
			return 1;
		}

		//and run the app
		MainWindow wnd(scope, server, port/*, namesrvr*/);
		kit.run(wnd);

		//if(namesrvr)
		//	delete namesrvr;
		delete scope;
	}
	catch(const JtagException& ex)
	{
		LogError("%s\n", ex.GetDescription().c_str());
		exit_code = 1;
	}

	return exit_code;
}