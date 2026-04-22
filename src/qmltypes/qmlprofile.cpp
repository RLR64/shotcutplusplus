/*
 * Copyright (c) 2014 Meltytech, LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Local
#include "qmlprofile.hpp"
#include "mltcontroller.hpp"
#include <qobject.h>

auto QmlProfile::singleton() -> QmlProfile& {
	static QmlProfile instance;
	return instance;
}

QmlProfile::QmlProfile() : QObject() {
}

auto QmlProfile::width() const -> int {
	return MLT.profile().width();
}

auto QmlProfile::height() const -> int {
	return MLT.profile().height();
}

auto QmlProfile::aspectRatio() const -> double {
	return MLT.profile().dar();
}

auto QmlProfile::fps() const -> double {
	return MLT.profile().fps();
}

auto QmlProfile::sar() const -> double {
	return MLT.profile().sar();
}
