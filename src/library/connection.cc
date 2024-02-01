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

 /*
 #include <config.h>
 #include "private.h"
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/worker.h>
 #include <iostream>
 #include <unistd.h>
 #include <pthread.h>
 #include <pwd.h>

 using namespace std;

 namespace Udjat {

	class DataSlot {
	private:
		dbus_int32_t slot = 0;
		DataSlot() {
			dbus_connection_allocate_data_slot(&slot);
			Logger::String{"Got slot '",slot,"' for watching connections"}.trace("d-bus");
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

	std::recursive_mutex DBus::Connection::guard;

	DBus::Connection & DBus::Connection::getInstance() {
		if(getuid() == 0) {
			return getSystemInstance();
		}
		return getSessionInstance();
	}

	static void trace_connection_free(const DBus::Connection *connection) {
		debug("-------------------------------------------------");
		Logger::String("Connection '",((unsigned long) connection),"' was released").trace("d-bus");
	}

	DBus::Connection::Connection(DBusConnection * c, const char *n, bool reg) : name(n), connection(c) {

		static bool initialized = false;
		if(!initialized) {

			initialized = true;

			// Initialize d-bus threads.
			dbus_threads_init_default();

		}

		lock_guard<recursive_mutex> lock(guard);

		try {

			if(reg) {
				// Register
				DBusError err;
				dbus_error_init(&err);

				dbus_bus_register(connection,&err);
				if(dbus_error_is_set(&err)) {
					std::string message(err.message);
					dbus_error_free(&err);
					throw std::runtime_error(message);
				}
			}

			if (dbus_connection_add_filter(connection, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
				throw std::runtime_error("Cant add filter to D-Bus connection");
			}

			// Não encerro o processo ao desconectar.
			dbus_connection_set_exit_on_disconnect(connection, false);

			if(use_thread) {
				//
				// Thread mode
				//
				thread = new std::thread([this] {

					pthread_setname_np(pthread_self(),name.c_str());

					trace() << "Service thread begin" << endl;
					auto connct = connection;
					dbus_connection_ref(connct);
					while(connection && dbus_connection_read_write(connct,100)) {
						dispatch(connct);
					}
					trace() << "Flushing connection" << endl;
					dbus_connection_flush(connct);
					dbus_connection_unref(connct);
					trace() << "Service thread end" << endl;

				});

			} else {
				//
				// Non thread mode
				//

				// Initialize Main loop.
				MainLoop::getInstance();

				// Set watch functions.
				if(!dbus_connection_set_watch_functions(
					connection,
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
					connection,
					(DBusAddTimeoutFunction) add_timeout,
					(DBusRemoveTimeoutFunction) remove_timeout,
					(DBusTimeoutToggledFunction) toggle_timeout,
					this,
					nullptr)
				) {
					throw runtime_error("dbus_connection_set_timeout_functions has failed");
				}

			}

		} catch(...) {

			dbus_connection_unref(connection);
			throw;

		}

		if(Logger::enabled(Logger::Trace)) {
			dbus_connection_set_data(connection,DataSlot::getInstance().value(),this,(DBusFreeFunction) trace_connection_free);
			Logger::String("Connection '",((unsigned long) this),"' was allocated").trace("d-bus");
		}

	}

	DBus::Connection::Connection(uid_t uid, const char *sid) : Connection(Factory(uid,sid), "user") {

		// Replace session name with user's login name.
		int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize < 0)
				bufsize = 16384;

		string rc;
		char * buf = new char[bufsize];

		struct passwd     pwd;
		struct passwd   * result;
		if(getpwuid_r(uid, &pwd, buf, bufsize, &result)) {
			name = "U";
			name += to_string(uid);
		} else {
			name = buf;
		}
		delete[] buf;

	}

	static DBusConnection * ConnectionFactory(const char *busname) {

		if(!(busname && *busname)) {
			throw system_error(EINVAL,system_category(),"Invalid busname");
		}

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connection = dbus_connection_open_private(busname, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

        if(Logger::enabled(Logger::Trace)) {
			int fd = -1;
			if(dbus_connection_get_socket(connection,&fd)) {
				Logger::String("Got connection '",((unsigned long) connection),"' to '",busname,"' on socket '",fd,"'").trace("d-bus");
			} else {
				Logger::String("Unable to got socket for connection '",((unsigned long) connection),"' to '",busname,"'").trace("d-bus");
			}
        }

		return connection;

	}

	DBus::Connection::Connection(const char *busname, const char *name) : Connection(ConnectionFactory(busname),name) {
	}

	DBus::Connection::~Connection() {

		debug("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa DEALLOCATE");

		Logger::String("Deallocating connection '",((unsigned long) this),"'").trace("d-bus");

		flush();

		// Remove listeners.
		interfaces.remove_if([this](Interface &intf) {
			intf.remove_from(this);
			return true;
		});

		if(connection) {

			// Remove filter
			dbus_connection_remove_filter(connection,(DBusHandleMessageFunction) filter, this);

			// Stop D-Bus connection
			if(thread) {

				trace() << "Waiting for service thread " << thread << endl;
				thread->join();
				delete thread;


			} else if(!use_thread) {

				trace() << "Restoring d-bus watchers" << endl;

				if(!dbus_connection_set_watch_functions(
					connection,
					(DBusAddWatchFunction) NULL,
					(DBusRemoveWatchFunction) NULL,
					(DBusWatchToggledFunction) NULL,
					this,
					nullptr)
				) {
					error() << "dbus_connection_set_watch_functions has failed" << endl;
				}

				if(!dbus_connection_set_timeout_functions(
					connection,
					(DBusAddTimeoutFunction) NULL,
					(DBusRemoveTimeoutFunction) NULL,
					(DBusTimeoutToggledFunction) NULL,
					NULL,
					nullptr)
				) {
					error() << "dbus_connection_set_timeout_functions has failed" << endl;
				}

			}

			if(Logger::enabled(Logger::Trace)) {
				int fd = -1;
				if(dbus_connection_get_socket(connection,&fd)) {
					Logger::String("Releasing connection '",((unsigned long) connection),"' on socket '",fd,"'").trace("d-bus");
				} else {
					Logger::String("Unable to got socket for connection '",((unsigned long) connection),"'").trace("d-bus");
				}
			}

			debug("--------------------------> Will unref");
			dbus_connection_close(connection);
			dbus_connection_unref(connection);

			connection = nullptr;

		} else {

			warning() << "Connection was already disabled" << endl;
		}

	}

	void DBus::Connection::flush() noexcept {
		if(connection) {
			dbus_connection_flush(connection);
		}
	}

	std::ostream & DBus::Connection::info() const {
		return std::cout << name << "\t";
	}

	std::ostream & DBus::Connection::warning() const {
		return std::clog << name << "\t";
	}

	std::ostream & DBus::Connection::error() const {
		return std::cerr << name << "\t";
	}

	std::ostream & DBus::Connection::trace() const {
		return Logger::trace() << name << "\t";
	}

 }
 */
