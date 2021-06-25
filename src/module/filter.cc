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

 #include "private.h"
 #include <udjat/worker.h>

 namespace Udjat {

	namespace DBus {

		DBusHandlerResult Connection::filter(DBusConnection *connection, DBusMessage *message, Connection *controller) noexcept {

			// DBusHandlerResult rc = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

#ifdef DEBUG
			cout	<< "Member:    " << dbus_message_get_member(message) << endl
					<< "Interface: " << dbus_message_get_interface(message) << endl;
#endif // DEBUG

			try {
				//
				// First search for internal workers.
				//
				for(auto worker : controller->workers) {

					if(worker->equal(message)) {

						// Found worker for this message.
						worker->work(controller, message);
						return DBUS_HANDLER_RESULT_HANDLED;

					}

				}

				//
				// Do we have any libudjat worker for this interface?
				//
				if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
					static const char *interface = "br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME) ".";
					size_t intflen = strlen(interface);

					if(strncasecmp(dbus_message_get_interface(message),interface,intflen) == 0) {

						const char * worker = dbus_message_get_interface(message) + intflen;

						cout << "Requested worker: '" << worker << "'" << endl;


					}

				}

			} catch(const std::exception &e) {

				cerr << "D-Bus\t" << e.what() << endl;

				DBusMessage * rsp = dbus_message_new_error(message,DBUS_ERROR_FAILED,e.what());
				dbus_connection_send(connection, rsp, NULL);
				dbus_message_unref(rsp);
				return DBUS_HANDLER_RESULT_HANDLED;

			}
			/*
			//
			// Not found on internal workers, try libudjat ones
			//

			if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL && !strcasecmp(dbus_message_get_interface(message),interface)) {

				try {

					const char *ptr = dbus_message_get_interface(message) + strlen(interface);

#ifdef DEBUG
					cout << "Requested worker: '" << ptr << "'" << endl;
#endif // DEBUG


				} catch(const exception &e) {


				}

				rc = DBUS_HANDLER_RESULT_HANDLED;

			}
			*/

			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}

	}

 }

