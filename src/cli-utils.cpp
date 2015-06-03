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

#ifdef _WIN32
#include <Searchapi.h>
#include <atldbcli.h>

/**
 * Use Windows Search to locate PuTTY.exe
 */
string findPuTTY() {
    HRESULT hr;
    CDataSource cDataSource;
    wstring filename;

    // Co-Initialize
    CoInitialize( NULL );

    // Open a data source to windows search index
    hr = cDataSource.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
    if (SUCCEEDED(hr))
    {
        CSession cSession;

        // Open a session
        hr = cSession.Open(cDataSource);
        if (SUCCEEDED(hr))
        {

            // Invoke query
            CCommand<CDynamicAccessor, CRowset> cCommand;
            hr = cCommand.Open(cSession, "SELECT System.ItemURL FROM SYSTEMINDEX WHERE System.FileName = \'putty.exe\'");
            if (SUCCEEDED(hr))
            {

                // Process results
                for (hr = cCommand.MoveFirst(); S_OK == hr; hr = cCommand.MoveNext())
                {
                    DBTYPE typ;
                    cCommand.GetColumnType(1, &typ); 
 
                    /* Cast from unicode to string */
                    const wstring ws( (LPWSTR) cCommand.GetValue(1) );
                    const locale locale("");
                    typedef codecvt<wchar_t, char, mbstate_t> converter_type;
                    const converter_type& converter = use_facet<converter_type>(locale);
                    vector<char> to(ws.length() * converter.max_length());
                    mbstate_t state;
                    const wchar_t* from_next;
                    char* to_next;
                    const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
                    if (result == converter_type::ok || result == converter_type::noconv) {
                        const string s(&to[5], to_next); // Skip file:
                        return s;
                    } else {
                        // Could not convert filename
                        return "";
                    }

                }
                cCommand.Close();
            }
        }
    }

    // Not found
    return "";

}
#endif

/**
 * Open SSH using system's SSH utility
 */
void open_ssh( const string& host, const int port, const string& user ) {
    ostringstream oss;
#ifdef _WIN32

    // Locate PuTTY
    string putty = findPuTTY();
    if (putty.empty()) {
    	cout << "You can now use your SSH client to connect to: " << host << ":" << port << endl;
        return;
    }

    // Prepare command-line
    oss << "-ssh -P " << port << " " << user << "@" << host;
    string arg_str = oss.str();
    LPSTR args = const_cast<char *>(arg_str.c_str());

    // Prepare startup info
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Launch application
    if (!CreateProcess(
       putty.c_str(),
       args,
       NULL,
       NULL,
       FALSE,
       0,
       NULL,
       NULL,
       &si,
       &pi )
    ) {
        cout << "Could not start PuTTY, but you can now use any SSH client to connect to: " << host << ":" << port << endl;
        return;
    }

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

#else
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