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

#include "CLIInteraction.h"

bool prompt( const std::string& message ) {
	std::string line;
	while (true) {
		std::cout << message << " [y/n]: ";
		std::getline( std::cin, line );
		if ((line.compare("y") == 0) || (line.compare("Y") == 0)) {
			std::cout << "-----------------------------" << std::endl;
			return true;
		} else if ((line.compare("n") == 0) || (line.compare("N") == 0)) {
			std::cout << "-----------------------------" << std::endl;
			return false;
		}
	}

}

CLIInteraction::CLIInteraction() : UserInteraction() { 
	this->setConfirmHandler( boost::bind(&CLIInteraction::cli_confirm, this, _1, _2, _3) );
	this->setAlertHandler( boost::bind(&CLIInteraction::cli_alert, this, _1, _2, _3) );
	this->setLicenseHandler( boost::bind(&CLIInteraction::cli_license, this, _1, _2, _3) );
	this->setLicenseURLHandler( boost::bind(&CLIInteraction::cli_license_url, this, _1, _2, _3) );
	this->silent = false;
};

/**
 * Confirm from the CLI
 */
void CLIInteraction::cli_confirm(const std::string& title, const std::string& message, const callbackResult& result) {
	if (silent) {
		result(UI_OK);
		return;
	}
	std::cout << std::endl << "---[ " << title << " ]---" << std::endl;
	std::cout << message << std::endl;
	if (prompt("Do you confirm?")) {
		result(UI_OK);
	} else {
		result(UI_CANCEL);
	}
}

/**
 * Alert from the CLI
 */
void CLIInteraction::cli_alert(const std::string& title, const std::string& message, const callbackResult& result) {
	if (silent) {
		result(UI_OK);
		return;
	}
	std::cout << std::endl << "---[ " << title << " ]---" << std::endl;
	std::cout << message << std::endl;
	std::cout << "-----------------------------" << std::endl;
}

/**
 * Confirm licese from the CLI
 */
void CLIInteraction::cli_license(const std::string& title, const std::string& message, const callbackResult& result) {
	if (silent) {
		result(UI_OK);
		return;
	}
	std::cout << std::endl << "---[ " << title << " ]---" << std::endl;
	std::cout << message << std::endl;
	if (prompt("Do you accept the above license?")) {
		result(UI_OK);
	} else {
		result(UI_CANCEL);
	}
}

/**
 * Confirm licese by URL from the CLI
 */
void CLIInteraction::cli_license_url(const std::string& title, const std::string& url, const callbackResult& result) {
	if (silent) {
		result(UI_OK);
		return;
	}
	std::cout << std::endl << "---[ " << title << " ]---" << std::endl;
	std::cout << "See the license here: " << url << std::endl;
	if (prompt("Do you accept the above license?")) {
		result(UI_OK);
	} else {
		result(UI_CANCEL);
	}
}


