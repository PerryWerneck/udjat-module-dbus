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

 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <memory>
 #include <cstdlib>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	class Service : public SystemService {
	private:
		DBus::Connection *bus = nullptr;

	protected:
		/// @brief Initialize service.
		void init() override {
			cout << Application::Name() << "\tInitializing" << endl;

			// udjat_module_init();

			auto root = Abstract::Agent::init("*.xml");

			cout << "http://localhost:8989/api/1.0/info/modules.xml" << endl;
			cout << "http://localhost:8989/api/1.0/info/workers.xml" << endl;
			cout << "http://localhost:8989/api/1.0/info/factories.xml" << endl;
			cout << "http://localhost:8989/api/1.0/agent.xml" << endl;

			for(auto agent : *root) {
				cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << ".xml" << endl;
			}

			//DBus::Connection &session = DBus::Connection::getSessionInstance();
			bus = new DBus::Connection(getenv("DBUS_SESSION_BUS_ADDRESS"));

			bus->subscribe(
				this,
				"org.gnome.ScreenSaver",
				"ActiveChanged",
				[](DBus::Message &message) {

					cout << "org.gnome.ScreenSaver.ActiveChanged " << DBus::Value(message).to_string() << endl;

				}
			);

			bus->subscribe(
				this,
				"com.example.signal",
				"hello",
				[](DBus::Message &message) {

					cout << "com.example.signal.hello " << DBus::Value(message).to_string() << endl;
				}
			);

		}

		/// @brief Deinitialize service.
		void deinit() override {
			delete bus;
			cout << Application::Name() << "\tDeinitializing" << endl;
			Udjat::Module::unload();
		}

	public:
		Service() = default;


	};

	return Service().run(argc,argv);


}
