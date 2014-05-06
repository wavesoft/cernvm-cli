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
#ifndef CLI_INTERACTION_H
#define CLI_INTERACTION_H

#include <CernVM/UserInteraction.h>

#include <boost/bind.hpp>

#include <iostream>

/**
 * A class through interaction with the user can happen
 *
 * This implementation does command-line interaction.
 */
class CLIInteraction: public UserInteraction {
public:

	/**
	 * Constructor for user interaction class
	 */
	CLIInteraction();

	/**
	 * Confirm from the CLI
	 */
	void cli_confirm(const std::string& title, const std::string& message, const callbackResult& result);

	/**
	 * Alert from the CLI
	 */
	void cli_alert(const std::string& title, const std::string& message, const callbackResult& result);

	/**
	 * Confirm licese from the CLI
	 */
	void cli_license(const std::string& title, const std::string& message, const callbackResult& result);

	/**
	 * Confirm licese by URL from the CLI
	 */
	void cli_license_url(const std::string& title, const std::string& url, const callbackResult& result);

	/**
	 * Automatically accept messages
	 */
	bool 	silent;

};

#endif /* end of include guard: CLI_INTERACTION_H */
