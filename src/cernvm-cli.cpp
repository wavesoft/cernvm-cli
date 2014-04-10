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
#include "CLIProgressFeedback.h"

#include <map>
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
		cerr << "ERROR: " << error << endl;
	}
	cout << "Usage:" << endl;
	cout << endl;
	cout << "   cernvm-cli <command> [<session> [<arguments>]]" << endl;
	cout << endl;
	cout << "Commands without arguments:" << endl;
	cout << endl;
	cout << "   list                        List the registered machines" << endl;
	cout << endl;
	cout << "Commands:" << endl;
	cout << endl;
	cout << "   open     [--32]             Use 32-bit linux version (default x86_64)" << endl;
	cout << "            [--fio]            Use FloppyIO data exchange" << endl;
	cout << "            [--gui]            Enable GUI additions" << endl;
	cout << "            [--dualnic]        Use two NICs instead of NATing through one" << endl;
	cout << "            [--ram <MB>]       How much RAM to allocate on the new VM (default 1024)" << endl;
	cout << "            [--hdd <MB>]       How much disk to allocate on the new VM (default 1024)" << endl;
	cout << "            [--ver <ver>]      The uCernVM version to use (default " << DEFAULT_CERNVM_VERSION << ")" << endl;
	cout << "            [--api <num>]      Define the API port to use (default 80)" << endl;
	cout << "            [--context <uuid>] The ContextID from CernVM-Online to boot" << endl;
	cout << endl;
	cout << "   start                       Start the VM" << endl;
	cout << "   stop                        Stop the VM" << endl;
	cout << "   save                        Save the VM on disk" << endl;
	cout << "   pause                       Pause the VM on memory" << endl;
	cout << "   resume                      Resume the VM" << endl;
	cout << "   close                       Destroy and remove the VM" << endl;
	cout << endl;
}

int handle_open( list<string>& args, const string& name, const string& key ) {

	int  	int_ram=512, int_hdd=10240, int_flags=HVF_SYSTEM_64BIT, int_port=80;
	string	str_ver="1.17-8", context_id="", strval, arg;

	while (!args.empty()) {
		arg = args.front();
		if (arg.compare("--32") == 0) {
			args.pop_front();
			int_flags &= !HVF_SYSTEM_64BIT;
		} else if (arg.compare("--fio") == 0) {
			args.pop_front();
			int_flags |= HVF_FLOPPY_IO;
		} else if (arg.compare("--gui") == 0) {
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
		} else if (arg.compare("--api") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--api' argument");
				return 100;
			}
			strval = args.front(); args.pop_front();
			int_port = ston<int>(strval);
		} else if (arg.compare("--context") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--context' argument");
				return 100;
			}
			context_id = args.front(); args.pop_front();
		} else {
			cerr << "WARNING: Ignored unnown parameter '" << arg << "'" << endl;
		}
	}

	/*
	cout << "Name=" << name << ", Secret=" << key << endl;
	cout << "Version=" << str_ver << ", Flags=" << int_flags << endl;
	cout << "Ram=" << int_ram << ", Hdd=" << int_hdd << endl;
	cout << "Hypervisor=" << hv->version.verString << endl;
	*/
	
	// Prepare UserData
	ostringstream oss;
	if (!context_id.empty()) {
		// Make it boot the given context
		oss << "[cernvm]\ncontextualization_key=" << context_id.front();
		oss << "\n";
	}

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key)
		   .set("cernvmVersion", str_ver)
		   .set("userData", oss.str())
		   .setNum<int>("apiPort", int_port)
		   .setNum<int>("flags", int_flags)
		   .setNum<int>("ram", int_ram)
		   .setNum<int>("hdd", int_hdd);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );
    
    // Open & reach poweroff state
    session->stop();

	// Wait for completion
	session->wait();

	return 0;

}

int handle_start( list<string>& args, const string& name, const string& key ) {
	
	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
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

int handle_close( list<string>& args, const string& name, const string& key ) {

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

int handle_list( list<string>& args ) {

	// Iterate over open sessions
	cout << "Registered sessions with libCernVM:" << endl;
	cout << endl;
	for (std::map< std::string, HVSessionPtr >::iterator it = hv->sessions.begin(); it != hv->sessions.end(); ++it) {
		string name = (*it).first;
		HVSessionPtr sess = (*it).second;
		cout << " - " << sess->parameters->get("name", "") << " (" << name << ")" << endl;
		cout << "   cpus=" << sess->parameters->get("cpus", "1")
		     << ", ram=" << sess->parameters->get("memory", "512")
		     << ", disk=" << sess->parameters->get("disk", "1024")
		     << ", apiPort=" << sess->parameters->get("apiPort", BOOST_PP_STRINGIZE( DEFAULT_API_PORT ))
		     << ", flags=" << sess->parameters->get("flags", "9")
             << ", uCernVM=" << sess->parameters->get("cernvmVersion", DEFAULT_CERNVM_VERSION) << endl << endl;
	}
	cout << endl;

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
	string arg, command, session;
	if (args.empty()) {
		show_help("Missing command!");
		return 100;
	}
	command = args.front(); args.pop_front();

	// Create a hypervisor instance
	hv = detectHypervisor();
	userInteraction = boost::make_shared<CLIInteraction>();
	progressTask = boost::make_shared<FiniteTask>();

	// Create a CLI-Based user feedback
	CLIProgessFeedback clifeedback;
	clifeedback.bindTo( progressTask );

	// Initialize hypervisor
	hv->setUserInteraction( userInteraction );
	hv->waitTillReady( progressTask, userInteraction );

	// Handle commands wihtout parameters
	if (command.compare("list") == 0) { /* LIST */
		return handle_list(args);	
	}

	// Handle cases where a session name is needed
	if (args.empty()) {
		show_help("Missing session name!");
		return 100;
	}
	session = args.front(); args.pop_front();

	// Calculate session key
	// TODO: Make this a bit more difficult to guess
	string key = session;

	// Validate session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", session)
		   .set("secret", session);
	int status = hv->sessionValidate( params );
	if (status == 2) {
		cerr << "ERROR: Could not open session " << session <<"!" << endl;
		cerr << "       (was that session created from another source?)" << endl;
		return 1;
	} else if ((status == 0) && (command.compare("open") != 0)) {
		cerr << "ERROR: The specified session " << session <<" does not exist!" << endl;
		return 2;
	}

	// Handle action
	if (command.compare("open") == 0) { /* OPEN */
		return handle_open(args, session, key);

	} else if (command.compare("start") == 0) { /* START */
		return handle_start(args, session, key);

	} else if (command.compare("stop") == 0) { /* STOP */
		return handle_stop(args, session, key);

	} else if (command.compare("pause") == 0) { /* PAUSE */
		return handle_pause(args, session, key);

	} else if (command.compare("resume") == 0) { /* RESUME */
		return handle_resume(args, session, key);

	} else if (command.compare("save") == 0) { /* SAVE */
		return handle_save(args, session, key);

	} else if (command.compare("close") == 0) { /* REMOVE */
		return handle_close(args, session, key);

	} else {
		cout << "Unknown command " << arg << "!" << endl;
		show_help("");

	}

	// Success
	return 0;
	
}