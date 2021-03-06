/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2015 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/mainloop.h>
 #include <unistd.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

 class Context : public MainLoop::Handler {
 private:

	DBus::Connection	* connection	= nullptr;
	DBusWatch			* watch			= nullptr;

 protected:
	bool call(const MainLoop::Event events) const override;

 public:
	Context(DBus::Connection *c, int f, DBusWatch *w, short e) : MainLoop::Handler(c,f,(MainLoop::Event) e), connection(c), watch(w) {
	}

	void set(int fd) {
		this->fd = fd;
	}

 };

 dbus_bool_t add_watch(DBusWatch *watch, DBus::Connection *connection) {

	// Get event
	short event = 0;
 	unsigned int flags = dbus_watch_get_flags(watch);

	if (flags & DBUS_WATCH_READABLE)
		event |= POLLIN;

	if (flags & DBUS_WATCH_WRITABLE)
		event |= POLLOUT;

	if (flags & DBUS_WATCH_HANGUP)
		event |= POLLHUP;

	if (flags & DBUS_WATCH_ERROR)
		event |= POLLERR;

	auto context = make_shared<Context>(
						connection,
						dbus_watch_get_unix_fd(watch),
						watch,
						event
					);

	dbus_watch_set_data(watch, context.get(), NULL);

	MainLoop::getInstance().push_back(context);

	return true;
 }

 void remove_watch(DBusWatch *watch, DBus::Connection UDJAT_UNUSED(*obj)) {

	Context *context = (Context *) dbus_watch_get_data(watch);

	if(context) {
		dbus_watch_set_data(watch, NULL, NULL);
		context->clear();
	}

 }

 void toggle_watch(DBusWatch *watch, DBus::Connection UDJAT_UNUSED(*obj)) {

	Context *context = (Context *) dbus_watch_get_data(watch);

	if(context) {

		context->set(dbus_watch_get_unix_fd(watch));

		if (dbus_watch_get_enabled(watch)) {
			context->enable();
		} else {
			context->disable();
		}

	}

 }

 bool Context::call(const MainLoop::Event events) const {

	unsigned int flags = 0;

	if (events & POLLIN)
		flags |= DBUS_WATCH_READABLE;

	if (events & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;

	if (events & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;

	if (events & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	if(dbus_watch_handle(watch, flags) == FALSE) {
		cerr << "d-bus\tdbus_watch_handle() failed" << endl;
		return false;
	}

	// http://lists.freedesktop.org/archives/dbus/2007-October/008859.html
	while(!dbus_watch_handle(watch, flags)) {
		clog << "d-bus\tdbus_watch_handle needs more memory" << endl;
		sleep(1);
	}

	handle_dispatch_status(connection->getConnection(), DBUS_DISPATCH_DATA_REMAINS, connection);

	return true;
 }

 /*
 static void handle_watch(short events, WatchContext *ctx) {

	unsigned int flags = 0;

	if (events & POLLIN)
		flags |= DBUS_WATCH_READABLE;

	if (events & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;

	if (events & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;

	if (events & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	if (dbus_watch_handle(ctx->watch, flags) == FALSE) {
		cerr << "d-bus\tdbus_watch_handle() failed" << endl;
	}

	// http://lists.freedesktop.org/archives/dbus/2007-October/008859.html
	while(!dbus_watch_handle(ctx->watch, flags)) {
		clog << "d-bus\tdbus_watch_handle needs more memory" << endl;
		sleep(1);
	}

	handle_dispatch_status(ctx->conn, DBUS_DISPATCH_DATA_REMAINS, ctx->obj);

 }

 dbus_bool_t add_watch(DBusWatch *w, DBus::Connection *obj) {

	WatchContext *ctx = new WatchContext();

	dbus_watch_set_data(w, ctx, NULL);

	ctx->conn 	= obj->getConnection();
	ctx->obj	= obj;
	ctx->watch	= w;
	ctx->fd		= dbus_watch_get_unix_fd(w);

	unsigned int flags = dbus_watch_get_flags(w);

	if (flags & DBUS_WATCH_READABLE)
		ctx->event |= POLLIN;

	if (flags & DBUS_WATCH_WRITABLE)
		ctx->event |= POLLOUT;

	if (flags & DBUS_WATCH_HANGUP)
		ctx->event |= POLLHUP;

	if (flags & DBUS_WATCH_ERROR)
		ctx->event |= POLLERR;

	if (dbus_watch_get_enabled(w)) {
		MainLoop::getInstance().insert(
				(void *) ctx,
				ctx->fd,
				(MainLoop::Event) ctx->event,
				[ctx](const MainLoop::Event events){
					handle_watch((short) events, ctx);
					return true;
				});

	}

	return true;
 }

 void remove_watch(DBusWatch *w, DBus::Connection UDJAT_UNUSED(*obj)) {

	WatchContext *ctx = (WatchContext *) dbus_watch_get_data(w);

	if(ctx) {
		MainLoop::getInstance().remove(ctx);
		delete ctx;
	}

 }

 void toggle_watch(DBusWatch *w, DBus::Connection UDJAT_UNUSED(*obj)) {

	WatchContext *ctx = (WatchContext *) dbus_watch_get_data(w);

	if(ctx) {

		MainLoop::getInstance().remove(ctx);

		ctx->fd = dbus_watch_get_unix_fd(w);

		if (dbus_watch_get_enabled(w)) {
			MainLoop::getInstance().insert(
					(void *) ctx,
					ctx->fd,
					(MainLoop::Event) ctx->event,
					[ctx](const MainLoop::Event events){
						handle_watch((short) events, ctx);
						return true;
					});

		}

	}

 }
*/

