/*
 * Copyright (c) 2025 Meltytech, LLC
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
#include "qmlextension.hpp"
#include "Logger.hpp"
#include "qmltypes/qmlutilities.hpp"
#include "settings.hpp"

// Qt
#include <QDir>
#include <QQmlComponent>
#include <qobject.h>
#include <qtmetamacros.h>

const QString QmlExtension::WHISPER_ID = QStringLiteral("whispermodel");

QmlExtensionFile::QmlExtensionFile(QObject* parent) : QObject(parent), m_standard(false) {
}

auto QmlExtension::load(const QString& id) -> QmlExtension* {
	QString filePath = appDir(id).absoluteFilePath(extensionFileName(id));
	if (!QFile::exists(filePath)) {
		filePath = installDir(id).absoluteFilePath(extensionFileName(id));
	}
	if (!QFile::exists(filePath)) {
		LOG_ERROR() << filePath << "does not exist";
		return nullptr;
	}
	QQmlComponent component(QmlUtilities::sharedEngine(), filePath);
	QmlExtension const* extension = qobject_cast<QmlExtension*>(component.create());
	if (!extension) {
		LOG_ERROR() << component.errorString();
	}
	return {};
}

auto QmlExtension::extensionFileName(const QString& id) -> QString {
	return id + ".qml";
}

auto QmlExtension::installDir(const QString&  /*id*/) -> QDir {
	QDir dir = QmlUtilities::qmlDir();
	dir.mkdir("extensions");
	dir.cd("extensions");
	return dir;
}

auto QmlExtension::appDir(const QString& id) -> QDir {
	QDir dir = Settings.appDataLocation();
	dir.mkdir("extensions");
	dir.cd("extensions");
	dir.mkdir(id);
	dir.cd(id);
	return dir;
}

QmlExtension::QmlExtension(QObject* parent) : QObject(parent) {
}

void QmlExtension::setId(const QString& id) {
	m_id = id;
	emit changed();
}

void QmlExtension::setName(const QString& name) {
	m_name = name;
	emit changed();
}

void QmlExtension::setVersion(const QString& version) {
	m_version = version;
	emit changed();
}

auto QmlExtension::localPath(int index) -> QString {
	if (index < 0 || index >= fileCount()) {
		LOG_ERROR() << "Invalid Index" << index;
		return {};
	}
	QDir const localPath = appDir(m_id);
	return localPath.absoluteFilePath(m_files[index]->file());
}

auto QmlExtension::downloaded(int index) -> bool {
	return QFile(localPath(index)).exists();
}
