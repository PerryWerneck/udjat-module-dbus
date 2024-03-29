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
  * @brief Declare D-Bus Message.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <string>
 #include <udjat/tools/dbus/value.h>

 namespace Udjat {

 	namespace DBus {

 		/// @brief D-Bus message
		class UDJAT_API Message {
		protected:

			struct {
				DBusMessage *value = nullptr;
				DBusMessageIter iter;
			} message;

			const char *name = "dbus";

		private:

			struct {
				bool valid = false;		/// @brief True if this is an error message.
				std::string name;		/// @brief Error name.
				std::string message;	/// @brief Error Message.
			} err;

			inline Message & add() {
				return *this;
			}

			template<typename T, typename... Targs>
			Message & add(const T &value, Targs... Fargs) {
				push_back(value);
				return add(Fargs...);
			}

		public:
			Message(const Message &message) = delete;
			Message(const Message *message) = delete;

			Message(const char *destination, const char *path, const char *iface, const char *method);

			template<typename T, typename... Targs>
			Message(const char *destination, const char *path, const char *iface, const char *method, const T &value, Targs... Fargs)
				: Message(destination,path,iface,method) {
				push_back(value);
				add(Fargs...);
			}

			Message(const DBusError &error);
			Message(DBusMessage *m);

			~Message();

			inline operator DBusMessage *() const noexcept {
				return message.value;
			}

			inline operator DBusMessageIter *() noexcept {
				return &message.iter;
			}

			inline operator bool() const {
				return !err.valid;
			}

			Message & pop(Value &value);

			DBusMessageIter * getIter();

			inline bool failed() const {
				return err.valid;
			}

			bool next();

			template <typename T>
			Message & pop(T &value) {
				Value v;
				pop(v);
				v.get(value);
				return *this;
			}

			inline const char * error_name() const {
				return this->err.name.c_str();
			}

			inline const char * error_message() const {
				return err.message.c_str();
			}

			Message & push_back(const DBus::Value &value);

			Message & push_back(const char *value);

			inline Message & push_back(const std::string &value) {
				return push_back(value.c_str());
			}

			Message & push_back(const bool value);

			Message & push_back(const int16_t value);
			Message & push_back(const uint16_t value);

			Message & push_back(const int32_t value);
			Message & push_back(const uint32_t value);

			Message & push_back(const int64_t value);
			Message & push_back(const uint64_t value);

			Message & push_back(const std::vector<std::string> &elements);

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;
			std::ostream & trace() const;

		};

 	}

 }

