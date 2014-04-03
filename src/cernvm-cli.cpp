/**
 * This file is part of CernVM Command Line Interface.
 *
 * CernVM-Cli is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CernVM-Cli is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CernVM-Cli. If not, see <http://www.gnu.org/licenses/>.
 *
 * Developed by Ioannis Charalampidis 2013
 * Contact: <ioannis.charalampidis[at]cern.ch>
 */

#include <CernVM/Hypervisor.h>
#include <boost/make_shared.hpp>

#include "CLIInteraction.h"

/**
 * Entry point for the CLI
 */
int main( int argc, char ** argv ) {

	// Create a hypervisor instance
	HVInstancePtr hv = detectHypervisor();
	UserInteractionPtr ui = boost::make_shared<CLIInteraction>();

	// Setup user interaction
	hv->setUserInteraction( ui );

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", "test")
		   .set("key", "test");
	HVSessionPtr session = hv->sessionOpen( params, FiniteTaskPtr() );

	// Success
	return 0;
	
}