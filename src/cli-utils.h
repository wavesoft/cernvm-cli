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
#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#include <CernVM/Utilities.h>
#include <CernVM/DownloadProvider.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

/**
 * Utility function for
 */
void open_ssh( const string& host, const int port, const string& user );

/**
 * Get User name from the context ID provided
 */
string get_user_from_context( const string& context_id, DownloadProviderPtr downloadProvider );

#endif /* end of include guard: CLI_UTILS_H */
