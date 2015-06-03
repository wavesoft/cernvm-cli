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

#include <cli-utils.h>
#include <CernVM/Hypervisor.h>
#include <vector>

/**
 * Open SSH using system's SSH utility
 */
void open_ssh( const string& host, const int port, const string& user ) {
#ifdef _WIN32
	cout << "You can now use your SSH client to connect to: " << host << ":" << port << endl;
#else
	ostringstream oss;
	oss << "/usr/bin/ssh -o \"UserKnownHostsFile /dev/null\" -p " << port << " " << user << "@" << host;
	system(oss.str().c_str());
#endif
}

/**
 * Identify who's the first user in the context specified
 */
string get_user_from_context( const string& context_id, DownloadProviderPtr downloadProvider ) {
	string dummy;
	string userName = "";

	// Prepare the URL to query
	ostringstream oss;
	oss << "https://cernvm-online.cern.ch/api/context?uuid=cernvm-cli&ver=1.0&context_id=" << context_id << "&checksum=";

	// Download data
	string context;
	int res = downloadProvider->downloadText(oss.str(), &context);

	// Check for errors
	if (res != HVE_OK)
		return "";

	// Split to lines
	vector< string > lines;
	explode( context, '\n', &lines );

	// Parse lines into map
	map< string, string > data;
	parseLines( &lines, &data, "=", " \t", 0, 1 );

	// Look for EC2_USER_DATA
	if (data.find("EC2_USER_DATA") == data.end())
		return "";

	// Base-64 decode EC2_USER_DATA
	context = base64_decode(data["EC2_USER_DATA"]);

	// Split context lines
	explode( context, '\n', &lines );

	// Analyze lines
	for (vector< string >::iterator it = lines.begin(); it != lines.end(); ++it ) {
		// Lookup users= field
		if (it->substr(0,6).compare("users=") == 0) {
			// Split on ':' and look for username
			getKV( *it, &userName, &dummy, ':', 6 );
		}
	}

	// Return username
	return userName;

}