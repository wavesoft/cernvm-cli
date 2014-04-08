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

#pragma once
#ifndef CLI_FEEDBACK_H
#define CLI_FEEDBACK_H

#include <CernVM/CallbacksProgress.h>
#include <CernVM/ProgressFeedback.h>

#include <boost/bind.hpp>
#include <boost/variant.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

/**
 * A class through interaction with the user can happen
 *
 * This implementation does command-line interaction.
 */
class CLIProgessFeedback {
public:

	void 	bindTo( const FiniteTaskPtr & pf );

private:

	void 	fb_started(VariantArgList& args);

	void 	fb_completed(VariantArgList& args);

	void 	fb_failed(VariantArgList& args);

	void 	fb_progress(VariantArgList& args);

	int 	messageLength;
	void 	showMessage(const std::ostringstream& ss);
	void 	clearMessage();

};

#endif /* end of include guard: CLI_FEEDBACK_H */
