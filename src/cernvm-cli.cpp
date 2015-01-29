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
#include <CernVM/DomainKeystore.h>

#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "CLIInteraction.h"
#include "CLIProgressFeedback.h"

#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <algorithm>
#include <locale>

using namespace std;

// Global variables
HVInstancePtr 						hv;
boost::shared_ptr<CLIInteraction> 	userInteraction;
FiniteTaskPtr	 					progressTask;

void show_help( const string& error ) {
	if (!error.empty()) {
		cerr << "ERROR: " << error << endl;
	}
	cerr << "CernVM Command Line Interface - v1.0" << endl;
	cerr << "(C) 2014 Ioannis Charalampidis, PH/TH & CernVM Group, CERN" << endl;
	cerr << endl;
    cerr << "Usage:" << endl;
	cerr << endl;
	cerr << "   cernvm-cli [<options>] <command> [<session> [<arguments>]]" << endl;
	cerr << endl;
	cerr << "Options:" << endl;
	cerr << endl;
	cerr << "   -s | --silent                           Do not display any message" << endl;
	cerr << "   -h | --help                             Show this help screen" << endl;
	cerr << endl;
	cerr << "Commands without arguments:" << endl;
	cerr << endl;
	cerr << "   list                                    List the registered machines" << endl;
	cerr << endl;
	cerr << "Commands:" << endl;
	cerr << endl;
	cerr << "   setup     <session>                     The name of the new session" << endl;
	cerr << "             [--32]                        Use 32-bit CPU (default is 64-bit)" << endl;
	cerr << "             [--fio]                       Use FloppyIO for data exchange" << endl;
	cerr << "             [--gui]                       Enable GUI additions" << endl;
	cerr << "             [--dualnic]                   Use two NICs instead of NATing through one" << endl;
	cerr << "             [--ram <MB>]                  How much RAM to allocate on the new VM (default 1024)" << endl;
	cerr << "             [--hdd <MB>]                  How much disk to allocate on the new VM (default 1024)" << endl;
	cerr << "             [--api <num>]                 Define the API port to use (default " << DEFAULT_API_PORT << ")" << endl;
	cerr << "             [--context <uuid>]            The ContextID for CernVM-Online to boot" << endl;
	cerr << "             [--ver <ver>]                 The uCernVM version to use (default " << DEFAULT_CERNVM_VERSION << ")" << endl;
    cerr << "             [--flavor devel|testing|prod] The uCernVM flavor to use (default " << DEFAULT_CERNVM_FLAVOR <<")" << endl;
	cerr << "             [--start]                     Start the VM after configuration" << endl;
	cerr << endl;
	cerr << "   start     <session>                     Start the VM" << endl;
	cerr << "   stop      <session>                     Stop the VM" << endl;
	cerr << "   save      <session>                     Save the VM on disk" << endl;
	cerr << "   pause     <session>                     Pause the VM on memory" << endl;
	cerr << "   resume    <session>                     Resume the VM" << endl;
	cerr << "   remove    <session>                     Destroy and remove the VM" << endl;
	cerr << "   get       <session> <parm> [<param>...] Get one or more configuration parameter values" << endl;
	cerr << "   waitstate <session> [<state>]           Wait until the session state changes (optionally to the given state)" << endl;
	cerr << endl;
	cerr << "Examples:" << endl;
	cerr << endl;
	cerr << "   Before you use a session, you must first set it up, using the 'setup' command," << endl;
	cerr << "   like this:" << endl;
	cerr << endl;
	cerr << "      cernvm-cli setup myvm --gui" << endl;
	cerr << endl;
	cerr << "   Then you can control it using the control commands like this:" << endl;
	cerr << endl;
	cerr << "      cernvm-cli start myvm" << endl;
	cerr << endl;
}

/**
 * Handle the SETUP command
 */
int handle_setup( list<string>& args, const string& name, const string& key ) {

	int  	int_ram=512, int_hdd=10240, int_flags=HVF_SYSTEM_64BIT, int_port=80;
	string	str_ver=DEFAULT_CERNVM_VERSION, context_id="", str_flavor=DEFAULT_CERNVM_FLAVOR, strval, arg;
	bool 	bool_start=false;

	while (!args.empty()) {
		arg = args.front();
		if (arg.compare("--32") == 0) {
			args.pop_front();
			int_flags &= !HVF_SYSTEM_64BIT;
		} else if (arg.compare("--fio") == 0) {
			args.pop_front();
			int_flags |= HVF_FLOPPY_IO;
		} else if (arg.compare("--start") == 0) {
			args.pop_front();
			bool_start = true;
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
				return 5;
			}
			strval = args.front(); args.pop_front();
			int_ram = ston<int>(strval);
		} else if (arg.compare("--hdd") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--hdd' argument");
				return 5;
			}
			strval = args.front(); args.pop_front();
			int_hdd = ston<int>(strval);
		} else if (arg.compare("--ver") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--ver' argument");
				return 5;
			}
			str_ver = args.front(); args.pop_front();
		} else if (arg.compare("--flavor") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--flavor' argument");
				return 5;
			}
            str_flavor = args.front(); args.pop_front();
            if ((str_flavor.compare("prod") != 0) &&
                (str_flavor.compare("testing") != 0) &&
                (str_flavor.compare("devel") != 0) &&
                (str_flavor.compare("slc5") != 0)) {
				show_help("Unknown flavor specified! Should be one of: prod,testing,devel,slc5");
				return 5;
            }
		} else if (arg.compare("--api") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--api' argument");
				return 5;
			}
			strval = args.front(); args.pop_front();
			int_port = ston<int>(strval);
		} else if (arg.compare("--context") == 0) {
			args.pop_front();
			if (args.empty()) {
				show_help("Missing value for the '--context' argument");
				return 5;
			}
			context_id = args.front(); args.pop_front();
		} else {
			args.pop_front();
            show_help("Unknown parameter '" + arg + "'");
            return 5;
		}
	}

	/*
	cerr << "Name=" << name << ", Secret=" << key << endl;
	cerr << "Version=" << str_ver << ", Flags=" << int_flags << endl;
	cerr << "Ram=" << int_ram << ", Hdd=" << int_hdd << endl;
	cerr << "Hypervisor=" << hv->version.verString << endl;
	*/
	
	// Prepare UserData
	ostringstream oss;
	if (!context_id.empty()) {
		// Make it boot the given context
		oss << "[cernvm]\ncontextualization_key=" << context_id;
		oss << "\n";
	}

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key)
		   .set("cernvmVersion", str_ver)
           .set("cernvmFlavor", str_flavor)
		   .set("userData", oss.str())
		   .setNum<int>("apiPort", int_port)
		   .setNum<int>("flags", int_flags)
		   .setNum<int>("ram", int_ram)
		   .setNum<int>("hdd", int_hdd);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );
    
    // Open & reach poweroff state
    if (bool_start) {
		ParameterMapPtr userData = ParameterMap::instance();
		session->start( userData );
    } else {
	    session->stop();
    }

	// Wait for completion
	session->wait();

	// Cleanup thread
	session->abort();

	return 0;

}

/**
 * Handle the START command
 */
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

	// Cleanup thread
	session->abort();

	// return ok
	return 0;

}

/**
 * Handle the STOP command
 */
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

	// Cleanup thread
	session->abort();

	// return ok
	return 0;

}

/**
 * Handle the PAUSE command
 */
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

	// Cleanup thread
	session->abort();

	// return ok
	return 0;

}

/**
 * Handle the RESUME command
 */
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

	// Cleanup thread
	session->abort();

	// return ok
	return 0;

}

/**
 * Handle the SAVE command
 */
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

	// Cleanup thread
	session->abort();

	// return ok
	return 0;

}

/**
 * Handle the REMOVE command
 */
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

	// Cleanup thread
	session->abort();

	// Delete session
	hv->sessionDelete(session);

	// return ok
	return 0;

}

/**
 * Handle the LIST command
 */
int handle_list( list<string>& args ) {

	// Iterate over open sessions
	cerr << "Registered sessions with libCernVM:" << endl;
	cerr << endl;
	for (std::map< std::string, HVSessionPtr >::iterator it = hv->sessions.begin(); it != hv->sessions.end(); ++it) {
		string name = (*it).first;
		HVSessionPtr sess = (*it).second;
		cout << " - " << sess->parameters->get("name", "") << " (" << name << ")" << endl;
		cout << "   cpus=" << sess->parameters->get("cpus", "1")
		     << ", ram=" << sess->parameters->get("ram", "512")
		     << ", disk=" << sess->parameters->get("disk", "1024")
		     << ", apiPort=" << sess->parameters->get("apiPort", BOOST_PP_STRINGIZE( DEFAULT_API_PORT ))
		     << ", flags=" << sess->parameters->get("flags", "9")
             << ", uCernVM=" << sess->parameters->get("cernvmVersion", DEFAULT_CERNVM_VERSION) << "," << sess->parameters->get("cernvmFlavor", DEFAULT_CERNVM_FLAVOR) << endl << endl;
	}
    if (hv->sessions.empty()) {
        cerr << " (There are no registered sessions)" << endl;
    }

	// return ok
	return 0;

}

/**
 * Handle the GET command
 */
int handle_get( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Flush stderror (status) messages
	cerr.flush();

    // Return parameter
    for ( list<string>::iterator it = args.begin(); it != args.end(); ++it) {
        string arg = *it;
        cout << arg << "=" << session->parameters->get(arg,"<not defined>") << endl;
    }

    return 0;
}


/**
 * Callback for handling the state change
 */
boost::mutex 				stateWaitMutex;
boost::condition_variable 	stateWaitCond;
bool 						stateWaitFlag;
int 						stateWaitTarget;

int handle_waitstate( list<string>& args, const string& name, const string& key ) {

	// Try to open a session
	ParameterMapPtr params = ParameterMap::instance();
	params->set("name", name)
		   .set("secret", key);
	HVSessionPtr session = hv->sessionOpen( params, progressTask );

	// Flush stderror (status) messages
	cerr.flush();

	// Calculate the state target
	stateWaitTarget = -1;
	if (!args.empty()) {
		string state = args.front();
		if (state == "available") {
			stateWaitTarget = SS_AVAILABLE;
		} else if (state == "poweroff") {
			stateWaitTarget = SS_POWEROFF;
		} else if (state == "saved") {
			stateWaitTarget = SS_SAVED;
		} else if (state == "paused") {
			stateWaitTarget = SS_PAUSED;
		} else if (state == "running") {
			stateWaitTarget = SS_RUNNING;
		} else if (state == "missing") {
			stateWaitTarget = SS_MISSING;
		} else {
            show_help("Unknown state specified! It must be one of: available, poweroff, saved, paused, running, missing");
            return 5;
		}
	}

	// Synchronize state
	session->update();

	// Check if we are already on the specified state
	int lastState = session->local->getNum<int>( "state", -1 );
	if (lastState == stateWaitTarget) {
		return lastState;
	}

	// Wait for state change
	while (true) {
		int state = session->local->getNum<int>( "state", -1 );
		if (state != lastState) {
			lastState = state;
			if ((stateWaitTarget == -1) || (stateWaitTarget == state)) {
				break;
			}
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		session->update();
	}

	// Return
	return lastState;

}

/**
 * Entry point for the CLI
 */
int main( int argc, char ** argv ) {
	int res;

	// Check for obvious errors
	if (argc < 2) {
		show_help("");
		return 5;
	}

	// Prepare for user interaction
	userInteraction = boost::make_shared<CLIInteraction>();
	progressTask = boost::make_shared<FiniteTask>();

	// Create a CLI-Based user feedback
	CLIProgessFeedback clifeedback;
	clifeedback.bindTo( progressTask );

	// Parse arguments into vector
    string arg;
	static list<string> args;
	for (int i=1; i<argc; i++) {
        arg = argv[i];

        // Take this opportunity to scan for flags
        if ((arg.compare("-h") == 0) || (arg.compare("--help") == 0)) {
            show_help("");
            return 5;
        } else if ((arg.compare("-s") == 0) || (arg.compare("--silent") == 0)) {
        	userInteraction->silent = true;
        	clifeedback.silent = true;
        } else {
        	args.push_back(arg);
        }
	}

	// Look for name and key
	string command, session;
	if (args.empty()) {
		show_help("Missing command!");
		return 5;
	}
	command = args.front(); args.pop_front();

	// Initialize cryptographic keystore
	DomainKeystore::Initialize();
	DomainKeystore keystore;

    // Synchronize keystore (if it's nessecary)
    res = keystore.updateAuthorizedKeystore( DownloadProvider::Default() );
    if (res != HVE_OK) {
		cerr << "ERROR: Could not initialize the cryptographic keystore." << endl;
		return 3;
    }

	// Create a hypervisor instance
	hv = detectHypervisor();
	if (!hv) {
		if (userInteraction->confirm("No hypervisor found", "Would you like to auto-install VirtualBox in your system?") == UI_OK) {
			int ans = installHypervisor(
					DownloadProvider::Default(),
					keystore,
					userInteraction,
					progressTask
				);
			if (ans != HVE_OK) {
				cerr << "ERROR: Unable to install hypervisor" << endl;
				return 3;
			} else {
				hv = detectHypervisor();
				if (!hv) {				
					cerr << "ERROR: Could not detect hypervisor even after installation. Sorry." << endl;
					return 3;
				}
			}
		} else {
			return 3;
		}
	}

	// Initialize hypervisor
	hv->setUserInteraction( userInteraction );
	hv->waitTillReady( keystore, progressTask, userInteraction );

	// Handle commands wihtout parameters
	if (command.compare("list") == 0) { /* LIST */
		return handle_list(args);	
	}

	// Handle cases where a session name is needed
	if (args.empty()) {
		show_help("Missing session name!");
		return 5;
	}
	session = args.front(); args.pop_front();

	// Check for flag in place of session
	if (session[0] == '-') {
		show_help("Expected session name, not command");
		return 5;
	}

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
		cerr << endl;
		return 1;
	} else if ((status == 0) && (command.compare("setup") != 0)) {
		cerr << "ERROR: The specified session " << session <<" does not exist!" << endl;
		cerr << "       Use the 'setup' command to initialize the session before." << endl;
		cerr << endl;
		return 2;
	}

	// Handle action
	if (command.compare("setup") == 0) { /* OPEN */
		return handle_setup(args, session, key);

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

	} else if (command.compare("remove") == 0) { /* REMOVE */
		return handle_remove(args, session, key);

    } else if (command.compare("get") == 0) { /* GET PARAMETER */
        return handle_get(args, session, key);

    } else if (command.compare("waitstate") == 0) { /* WAIT STATE CHANGE */
    	return handle_waitstate(args, session, key);

	} else {
		cerr << "Unknown command " << arg << "!" << endl;
		show_help("");

	}

	// Success
	return 0;
	
}