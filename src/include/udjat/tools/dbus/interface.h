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
  * @brief Declare an interface inside the d-bus connection.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/dbus/member.h>
 #include <list>
 #include <functional>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Interface : public std::string {
		private:

			const char *type;
			std::list<Udjat::DBus::Member> members;

		public:
			Interface(const char *name);
			Interface(const XML::Node &node);

			~Interface();

			bool operator==(const char *intf) const noexcept;

			Udjat::DBus::Member & push_back(const XML::Node &node,const std::function<void(Message & message)> &callback);
			Udjat::DBus::Member & emplace_back(const char *member, const std::function<void(Message & message)> &callback);

			/// @brief Get textual form of match rule for this interface.
			const std::string rule() const;

			inline auto begin() const noexcept {
				return members.begin();
			}

			inline auto end() const noexcept {
				return members.end();
			}

		};

	}

 }