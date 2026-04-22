/*
 * Copyright (c) 2013-2014 Meltytech, LLC
 * Author: Dan Dennedy <dan@dennedy.org>
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

#ifndef QMLUTILITIES_HPP
#define QMLUTILITIES_HPP

// Qt
#include <QDir>
#include <QObject>
#include <QPoint>
#include <QUrl>
#include <qtmetamacros.h>

class QQmlContext;
class QQmlEngine;

class QmlUtilities : public QObject {
	Q_OBJECT

  public:
	explicit QmlUtilities(QObject* parent = nullptr);

	static void        registerCommonTypes();
	static void        setCommonProperties(QQmlContext* context);
	static auto        qmlDir() -> QDir;
	static auto        blankVui() -> QUrl;
	static auto sharedEngine() -> QQmlEngine*;
};

#endif // QMLUTILITIES_HPP
