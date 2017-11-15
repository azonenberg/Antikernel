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
	@brief A top-level window containing the UI for a power supply
 */
#ifndef PSUWindow_h
#define PSUWindow_h

/**
	@brief Main application window class for a power supply
 */
class PSUWindow	: public Gtk::Window
{
public:
	PSUWindow(PowerSupply* psu, std::string host, int port);
	~PSUWindow();

protected:

	//Initialization
	void CreateWidgets();

	//Widgets
	Gtk::VBox m_vbox;
		std::vector<Gtk::HSeparator> m_hseps;
		std::vector<Gtk::Label> m_channelLabels;
		std::vector<Gtk::HBox> m_chanhboxes;
			std::vector<Gtk::VBox> m_voltboxes;
				std::vector<Gtk::HBox> m_vhboxes;
					std::vector<Gtk::Label> m_voltageLabels;
				std::vector<Gtk::HBox> m_vmhboxes;
					std::vector<Gtk::Label> m_mvoltageLabels;
					std::vector<Gtk::Label> m_voltageValueLabels;
			std::vector<Gtk::VBox> m_currboxes;
				std::vector<Gtk::HBox> m_ihboxes;
					std::vector<Gtk::Label> m_currentLabels;
				std::vector<Gtk::HBox> m_imhboxes;
					std::vector<Gtk::Label> m_mcurrentLabels;
					std::vector<Gtk::Label> m_currentValueLabels;

	//Our instrument connection
	PowerSupply* m_psu;

	//Status polling
	bool OnTimer(int timer);

	//UI handlers
	//void OnSignalSourceChanged();
	//void OnMeasurementTypeChanged();
	//void OnAutoRangeChanged();

	virtual void on_show();
	virtual void on_hide();
};


#endif
