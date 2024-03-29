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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/dbus/interface.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	DBus::Interface::Interface(const char *n) : std::string{n}, type{"signal"} {
	}

	DBus::Interface::Interface(const XML::Node &node) : std::string{String{node,"dbus-interface"}}, type{"signal"} {
	}

	DBus::Interface::~Interface() {
	}

	const std::string DBus::Interface::rule() const {
		return String{"type='",type,"',interface='",c_str(),"'"};
	}

	bool DBus::Interface::operator==(const char *intf) const noexcept {
		return strcasecmp(intf,c_str()) == 0;
	}

	Udjat::DBus::Member & DBus::Interface::push_back(const XML::Node &node,const std::function<void(Udjat::DBus::Message & message)> &callback) {
		return members.emplace_back(node,callback);
	}

	Udjat::DBus::Member & DBus::Interface::emplace_back(const char *member, const std::function<void(Udjat::DBus::Message & message)> &callback) {
		return members.emplace_back(member,callback);
	}

	void DBus::Interface::remove(const Udjat::DBus::Member &member) {
		members.remove_if([&member](Udjat::DBus::Member &m){
			return &m == &member;
		});
	}

 }


