/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements abstract connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <string>
 #include <mutex>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/mainloop.h>
 #include <private/mainloop.h>

 using namespace std;

 namespace Udjat {

	class DataSlot {
	private:
		dbus_int32_t slot = 0;
		DataSlot() {
			dbus_connection_allocate_data_slot(&slot);
			Logger::String{"Got slot '",slot,"' for connection watchdog"}.trace("d-bus");
		}

	public:

		~DataSlot() {
			dbus_connection_free_data_slot(&slot);
		}

		static DataSlot & getInstance() {
			static DataSlot instance;
			return instance;
		}

		inline dbus_int32_t value() const noexcept {
			return slot;
		}

	};

	std::mutex Abstract::DBus::Connection::guard;

	static void trace_connection_free(const Abstract::DBus::Connection *connection) {
		Logger::String("Connection '",((unsigned long) connection),"' was released").trace("d-bus");
	}

	DBusConnection * Abstract::DBus::Connection::SharedConnectionFactory(DBusBusType type) {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(type, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connct;

	}

	Abstract::DBus::Connection::Connection(const char *name, DBusConnection *c) : object_name{name}, conn{c} {

		static bool initialized = false;
		if(!initialized) {

			initialized = true;

			// Initialize d-bus threads.
			Logger::String("Initializing d-bus thread system").trace("d-bus");
			dbus_threads_init_default();

		}

	}

	Abstract::DBus::Connection::~Connection() {
	}

	void Abstract::DBus::Connection::open() {

		lock_guard<mutex> lock(guard);

		// Keep running if d-bus disconnect.
		dbus_connection_set_exit_on_disconnect(conn, false);

		// Add message filter.
		if (dbus_connection_add_filter(conn, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
			throw std::runtime_error("Cant add filter to D-Bus connection");
		}

		// Initialize Main loop.
		MainLoop::getInstance();

		// Set watch functions.
		if(!dbus_connection_set_watch_functions(
			conn,
			(DBusAddWatchFunction) add_watch,
			(DBusRemoveWatchFunction) remove_watch,
			(DBusWatchToggledFunction) toggle_watch,
			this,
			nullptr)
		) {
			throw runtime_error("dbus_connection_set_watch_functions has failed");
		}

		// Set timeout functions.
		if(!dbus_connection_set_timeout_functions(
			conn,
			(DBusAddTimeoutFunction) add_timeout,
			(DBusRemoveTimeoutFunction) remove_timeout,
			(DBusTimeoutToggledFunction) toggle_timeout,
			this,
			nullptr)
		) {
			throw runtime_error("dbus_connection_set_timeout_functions has failed");
		}


		if(Logger::enabled(Logger::Trace)) {

			dbus_connection_set_data(conn,DataSlot::getInstance().value(),this,(DBusFreeFunction) trace_connection_free);

			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Allocating connection '",((unsigned long) this),"' with socket '",fd,"'").trace(name());
			} else {
				Logger::String("Allocating connection '",((unsigned long) this),"'").trace(name());
			}

		}

	}

	void Abstract::DBus::Connection::close() {

		lock_guard<mutex> lock(guard);

        if(Logger::enabled(Logger::Trace)) {
			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Dealocating connection '",((unsigned long) this),"' from socket '",fd,"'").trace(name());
			} else {
				Logger::String("Dealocating connection '",((unsigned long) this),"'").trace(name());
			}
        }

		flush();

		// Remove listeners.
		/*
		interfaces.remove_if([this](Interface &intf) {
			intf.remove_from(this);
			return true;
		});
		*/

		// Remove filter
		dbus_connection_remove_filter(conn,(DBusHandleMessageFunction) filter, this);

		Logger::String{"Restoring d-bus watchers"}.trace(name());

		if(!dbus_connection_set_watch_functions(
			conn,
			(DBusAddWatchFunction) NULL,
			(DBusRemoveWatchFunction) NULL,
			(DBusWatchToggledFunction) NULL,
			this,
			nullptr)
		) {
			Logger::String{"dbus_connection_set_watch_functions failed"}.error(name());
		}

		if(!dbus_connection_set_timeout_functions(
			conn,
			(DBusAddTimeoutFunction) NULL,
			(DBusRemoveTimeoutFunction) NULL,
			(DBusTimeoutToggledFunction) NULL,
			NULL,
			nullptr)
		) {
			Logger::String{"dbus_connection_set_timeout_functions failed"}.error(name());
		}

		dbus_connection_unref(conn);

	}

	DBusHandlerResult Abstract::DBus::Connection::filter(DBusConnection *, DBusMessage *message, Abstract::DBus::Connection *connection) noexcept {

		if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_SIGNAL) {
			return connection->on_signal(message);
		}

		// TODO: Filter method calls.

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	DBusHandlerResult Abstract::DBus::Connection::on_signal(DBusMessage *message) {

		lock_guard<mutex> lock(guard);

		const char *member = dbus_message_get_member(message);
		const char *interface = dbus_message_get_interface(message);

		if(Logger::enabled(Logger::Trace)) {
			Logger::String{"Signal ", interface," ",member}.trace(name());
		}

		/*
		try {

			for(auto intf : interfaces) {
				if(!intf.name.compare(interface)) {
					for(auto memb : intf.members) {
						if(!memb.name.compare(member)) {

							try {

								Message msg(message);
								memb.call(msg);

							} catch(const exception &e) {

								cerr << name << "\tError '" << e.what() << "' processing signal " << interface << "." << member << endl;

							} catch(...) {

								cerr << name << "\tUnexpected error processing signal " << interface << "." << member << endl;

							}

						}

					}
					break;
				}
			}

		} catch(const exception &e) {

			cerr << name << "\t" << interface << " " << member << ": " << e.what() << endl;

		} catch(...) {

			cerr << name << "\t" << interface << " " << member << ": Unexpected error" << endl;

		}
		*/

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	void Abstract::DBus::Connection::flush() noexcept {
		dbus_connection_flush(conn);
	}

 }
