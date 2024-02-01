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
  * @brief Declare D-Bus connection.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <string>
 #include <mutex>
 #include <thread>

 namespace Udjat {

 	namespace Abstract {

		namespace DBus {

			/// @brief Abstract connection.
			class UDJAT_API Connection {
			private:

				/// @brief Mutex for serialization.
				static std::mutex guard;

				/// @brief The connection name.
				std::string object_name;

				/// @brief Service thread.
				std::thread * thread = nullptr;

				/// @brief Handle signal
				DBusHandlerResult on_signal(DBusMessage *message);

				/// @brief Message filter method.
				static DBusHandlerResult filter(DBusConnection *, DBusMessage *, Abstract::DBus::Connection *) noexcept;

			protected:

				/// @brief Connection to D-Bus.
				DBusConnection * conn;

				Connection(const char *name, DBusConnection * conn);

				void open();
				void close();

				static DBusConnection * SharedConnectionFactory(DBusBusType type);

			public:

				inline const char *name() const noexcept {
					return object_name.c_str();
				}

				inline DBusConnection * connection() const noexcept {
					return conn;
				}

				virtual ~Connection();

				void flush() noexcept;

			};


		}

 	}

 	namespace DBus {

		/// @brief D-Bus shared system connection.
		class UDJAT_API SystemBus : public Abstract::DBus::Connection {
		public:
			SystemBus();
			virtual ~SystemBus();

		};

		/// @brief D-Bus shared user connection.
		class UDJAT_API SessionBus : public Abstract::DBus::Connection {
		public:
			SessionBus();
			virtual ~SessionBus();

		};

		/// @brief D-Bus shared starter connection.
		class UDJAT_API StarterBus : public Abstract::DBus::Connection {
		public:
			StarterBus();
			virtual ~StarterBus();

		};

 	}

 }
