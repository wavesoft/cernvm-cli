/**
 * This file is part of CernVM Web API Plugin.
 *
 * CVMWebAPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CVMWebAPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CVMWebAPI. If not, see <http://www.gnu.org/licenses/>.
 *
 * Developed by Ioannis Charalampidis 2013
 * Contact: <ioannis.charalampidis[at]cern.ch>
 */

#include "CLIProgressFeedback.h"
#include <math.h>

void CLIProgessFeedback::bindTo( const FiniteTaskPtr & pf ) {
	// Bind event handlers
	pf->on("started", boost::bind(&CLIProgessFeedback::fb_started, this, _1));
	pf->on("completed", boost::bind(&CLIProgessFeedback::fb_completed, this, _1));
	pf->on("failed", boost::bind(&CLIProgessFeedback::fb_failed, this, _1));
	pf->on("progress", boost::bind(&CLIProgessFeedback::fb_progress, this, _1));
}

void CLIProgessFeedback::fb_started(VariantArgList& args) {
	std::ostringstream oss;
	oss << "[----] " << boost::get<string>( args[0] );
	showMessage(oss);
}

void CLIProgessFeedback::fb_completed(VariantArgList& args) {
	clearMessage();
}

void CLIProgessFeedback::fb_failed(VariantArgList& args) {
	std::ostringstream oss;
	oss << "[!!!!] " << boost::get<string>( args[0] );
	showMessage(oss);
	cout << endl;
}

void CLIProgessFeedback::fb_progress(VariantArgList& args) {
	std::ostringstream oss;
	int percentInt = (int)(boost::get<double>( args[1] ) * 100.0);

	oss << "[" << percentInt;
	if (percentInt < 100) { oss << " "; }
	if (percentInt < 10) { oss << " "; }

	oss << "%] " << boost::get<string>( args[0] );
	showMessage(oss);
}

void CLIProgessFeedback::showMessage(const std::ostringstream& ss) {
	std::string msg = ss.str();
	if (messageLength > 0)
		clearMessage();
	messageLength = msg.length();
	cout << msg;
	cout.flush();
}

void CLIProgessFeedback::clearMessage() {
	if (messageLength <= 0) return;
	// Clear area
	int i;
	for (i=0; i<messageLength; i++) { cout << "\b"; };
	for (i=0; i<messageLength; i++) { cout << " "; };
	// Rewind cursor
	for (i=0; i<messageLength; i++) { cout << "\b"; };
	// Reset
	cout.flush();
	messageLength = 0;
}
