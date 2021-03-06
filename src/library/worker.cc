/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/tools/dbus.h>

 using namespace std;

 /*
 namespace Udjat {

	namespace DBus {

		Worker::Worker() {
		}

		Worker::~Worker() {
		}

		bool Worker::equal(DBusMessage *message) {

			if(type != -1 && type != dbus_message_get_type(message))
				return false;

			if(member && strcasecmp(member,dbus_message_get_member(message)) != 0)
				return false;

			if(interface && strcasecmp(interface,dbus_message_get_interface(message)) != 0)
				return false;

			return true;
		}

		bool Worker::work(DBus::Request UDJAT_UNUSED(&request), DBus::Response UDJAT_UNUSED(&response)) {
			throw runtime_error("Method not allowed");
		}

		const std::string Worker::getAction(DBusMessage *message) {

			const char *interface = dbus_message_get_interface(message);

			if(strlen(interface) <= strlen(this->interface)) {
				return "";
			}

			const char *action = strchr(interface + strlen(this->interface), '.');
			if(!action) {
				return "";
			}

			return action+1;
		}

	}

 }
 */
