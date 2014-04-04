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
#include <CernVM/Utilities.h>

#include <boost/make_shared.hpp>
#include "CLIInteraction.h"

#include <iostream>
#include <sstream>
#include <string>
#include <list>

using namespace std;

// Global variables
HVInstancePtr 		hv;
UserInteractionPtr 	userInteraction;
FiniteTaskPtr	 	progressTask;

void show_help( const string& error ) {
	if (!error.empty()) {
		cout << "ERROR: " << error << endl;
	}
	cout << "Usage:" << endl;
	cout << endl;
	cout << "   cernvm-cli <general options> <command> " << endl;
	cout << endl;
	cout << "General Options:" << endl << endl;
	cout << "   [-n|--name]            The name of the session" << endl;
	cout << "   [-k|--key]             The secret key for accessing the session" << endl;
	cout << endl;
	cout << "Commands:" << endl;
	cout << endl;
	cout << "   open     [--32]        Use 32-bit linux version (default x86_64)" << endl;
	cout << "            [--fio]       Use FloppyIO data exchange" << endl;
	cout << "            [--ui]        Enable UI additions" << endl;
	cout << "            [--dualnic]   Use two NICs instead of NATing through one" << endl;
	cout << "            [--ram <MB>]  How much RAM to allocate on the new VM" << endl;
	cout << "            [--hdd <MB>]  How much disk to allocate on the new VM" << endl;
	cout << "            [--ver <ver>] The uCernVM version to use" << endl;
	cout << endl;
	cout << "   start    <context>     The ContextID from CernVM online to boot" << endl;
	cout << "   save                   Save the VM on disk" << endl;
	cout << "   stop                   Stop the VM" << endl;
	cout << "   pause                  Pause the VM" << endl;
	cout << "   resume                 Resume the VM" << endl;
	cout << "   remove                 Destroy and remove the VM" << endl;

}

int handle_open( list<string>& args, const string& name, const string& key ) {

	int  	int_ram=512, int_hdd=10240, int_flags=HVF_SYSTEM_64BIT;
	string	str_ver="1.17-8", strval, arg;

	while (!args.empty()) {
		arg = args.front();
		if (arg.compare("--32") == 0) {
			args.pop_front();
			int_flags &= !HVF_SYSTEM_64BIT;
		} else if (arg.compare("--fio") == 0) {
			args.pop_front();
			int_flags |= HVF_FLOPPY_IO;
		} else if (arg.compare("--ui") == 0) {
			args.pop_front();
			int_flags |= HVF_GUEST_ADDITIONS;
			int_flags |= HVF_HEADFUL;
			int_flags |= HVF_GRAPHICAL;
		} else if (arg.compare("--dualnic") == 0) {
			args.pop_front();
			int_flags |= HVF_DUAL_NIC;
		} else if (arg.compare("--ram") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--ram' argument");
				return 100;
			}
			strval = args.front(); args.pop_front();
			int_ram = ston<int>(strval);
		} else if (arg.compare("--hdd") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--hdd' argument");
				return 100;
			}
			strval = args.front(); args.pop_front();
			int_hdd = ston<int>(strval);
		} else if (arg.compare("--ver") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--ver' argument");
				return 100;
			}
			str_ver = args.front(); args.pop_front();
		}
	}

	cout << "Name=" << name << ", Secret=" << key << endl;
	cout << "Version=" << str_ver << ", Flags=" << int_flags << endl;
	cout << "Ram=" << int_ram << ", Hdd=" << int_hdd << endl;
	cout << "Hypervisor=" << hv->version.verString << endl;

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key)
		   .set("version", str_ver)
		   .setNum<int>("flags", int_flags)
		   .setNum<int>("ram", int_ram)
		   .setNum<int>("hdd", int_hdd);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Wait for completion
	session->wait();

	return 0;

}

int handle_start( list<string>& args, const string& name, const string& key ) {
	
	// Prepare UserData
	ostringstream oss;
	if (!args.empty()) {

		// Make it boot the given context
		oss << "[cernvm]\ncontextualization_key=" << args.front();
		oss << "\n";

	}

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key)
		   .set("userData", oss.str());
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Start session with blank key/value userData
	ParameterMapPtr userData = ParameterMap::instance();
	session->start( userData );

	// Wait for completion
	session->wait();

	// return ok
	return 0;

}

int handle_stop( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Stop
	session->stop();

	// Wait for completion
	session->wait();

	// return ok
	return 0;

}

int handle_pause( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Pause
	session->pause();

	// Wait for completion
	session->wait();

	// return ok
	return 0;

}

int handle_resume( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Pesume
	session->resume();

	// Wait for completion
	session->wait();

	// return ok
	return 0;

}

int handle_save( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Hibernate
	session->hibernate();

	// Wait for completion
	session->wait();

	// return ok
	return 0;

}

int handle_remove( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Destroy
	session->close();

	// Wait for completion
	session->wait();

	// Delete session
	hv->sessionDelete(session);

	// return ok
	return 0;

}

/**
 * Entry point for the CLI
 */
int main( int argc, char ** argv ) {

	cout << "CernVM Command Line Interface - v1.0" << endl;
	cout << "(C) 2014 Ioannis Charalampidis, PH/TH & CernVM Groups" << endl;
	cout << endl;

	// Check for obvious errors
	if (argc < 2) {
		show_help("");
		return 100;
	}

	// Parse arguments into vector
	static list<string> args;
	for (int i=1; i<argc; i++) {
		args.push_back(argv[i]);
	}

	// Look for name and key
	string name="", key="", arg;
	while (!args.empty()) {
		arg = args.front();
		if (arg.compare("-n") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '-n' argument");
				return 100;
			}
			name = args.front(); args.pop_front();
		} else if (arg.compare("-k") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '-k' argument");
				return 100;
			}
			key = args.front(); args.pop_front();
		} else {
			if (arg[0] == '-') {
				show_help("Unknown general argument specified");
				return 100;
			}
			break;
		}
	}

	// Create a hypervisor instance
	hv = detectHypervisor();
	userInteraction = boost::make_shared<CLIInteraction>();
	progressTask = boost::make_shared<FiniteTask>();

	// Initialize hypervisor
	hv->setUserInteraction( userInteraction );
	hv->waitTillReady( progressTask, userInteraction );

	// Handle action
	arg = args.front(); args.pop_front();
	if (arg.compare("open") == 0) { /* OPEN */
		return handle_open(args, name, key);

	} else if (arg.compare("start") == 0) { /* START */
		return handle_start(args, name, key);

	} else if (arg.compare("stop") == 0) { /* STOP */
		return handle_stop(args, name, key);

	} else if (arg.compare("pause") == 0) { /* PAUSE */
		return handle_pause(args, name, key);

	} else if (arg.compare("resume") == 0) { /* RESUME */
		return handle_resume(args, name, key);

	} else if (arg.compare("save") == 0) { /* SAVE */
		return handle_save(args, name, key);

	} else if (arg.compare("remove") == 0) { /* REMOVE */
		return handle_remove(args, name, key);

	} else {
		cout << "Unknown command " << arg << "!" << endl;
		show_help("");

	}

	// Success
	return 0;
	
}