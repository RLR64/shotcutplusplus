/*
 * Copyright (c) 2013-2024 Meltytech, LLC
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
#include "qmlapplication.hpp"
#include "controllers/filtercontroller.hpp"
#include "mainwindow.hpp"
#include "mltcontroller.hpp"
#include "models/attachedfiltersmodel.hpp"
#include "settings.hpp"
#include "util.hpp"
#include "videowidget.hpp"

// Qt
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCursor>
#include <QFileInfo>
#include <QMessageBox>
#include <QPalette>
#include <QStyle>
#include <QSysInfo>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qguiapplication.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#ifdef Q_OS_WIN
#include <QLocale>
#else
#include <clocale>
#endif

// STL
#include <limits>

auto QmlApplication::singleton() -> QmlApplication& {
	static QmlApplication instance;
	return instance;
}

QmlApplication::QmlApplication() : QObject() {
}

auto QmlApplication::dialogModality() -> Qt::WindowModality {
#ifdef Q_OS_MAC
	return Qt::WindowModal;
#else
	return Qt::ApplicationModal;
#endif
}

auto QmlApplication::mousePos() -> QPoint {
	return QCursor::pos();
}

auto QmlApplication::toolTipBaseColor() -> QColor {
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	if ("gtk+" == QApplication::style()->objectName())
		return QApplication::palette().highlight().color();
#endif
	return QApplication::palette().toolTipBase().color();
}

auto QmlApplication::toolTipTextColor() -> QColor {
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	if ("gtk+" == QApplication::style()->objectName())
		return QApplication::palette().highlightedText().color();
#endif
	return QApplication::palette().toolTipText().color();
}

auto QmlApplication::OS() -> QString {
#if defined(Q_OS_MAC)
	return "macOS";
#elif defined(Q_OS_LINUX)
	return "Linux";
#elif defined(Q_OS_UNIX)
	return "UNIX";
#elif defined(Q_OS_WIN)
	return "Windows";
#else
	return "";
#endif
}

auto QmlApplication::mainWinRect() -> QRect {
	return MAIN.geometry();
}

auto QmlApplication::hasFiltersOnClipboard() -> bool {
	return MLT.hasFiltersOnClipboard();
}

void QmlApplication::copyEnabledFilters() {
	QScopedPointer<Mlt::Producer> const producer(new Mlt::Producer(MAIN.filterController()->attachedModel()->producer()));
	MLT.copyFilters(producer.data(), MLT.FILTER_INDEX_ENABLED);
	QGuiApplication::clipboard()->setText(MLT.filtersClipboardXML());
	emit QmlApplication::singleton().filtersCopied();
}

void QmlApplication::copyAllFilters() {
	QScopedPointer<Mlt::Producer> const producer(new Mlt::Producer(MAIN.filterController()->attachedModel()->producer()));
	MLT.copyFilters(producer.data(), MLT.FILTER_INDEX_ALL);
	QGuiApplication::clipboard()->setText(MLT.filtersClipboardXML());
	emit QmlApplication::singleton().filtersCopied();
}

void QmlApplication::copyCurrentFilter() {
	const int currentIndex = MAIN.filterController()->currentIndex();
	if (currentIndex < 0) {
		MAIN.showStatusMessage(tr("Select a filter to copy"));
		return;
	}
	QScopedPointer<Mlt::Producer> const producer(new Mlt::Producer(MAIN.filterController()->attachedModel()->producer()));
	MLT.copyFilters(producer.data(), currentIndex);
	QGuiApplication::clipboard()->setText(MLT.filtersClipboardXML());
	emit QmlApplication::singleton().filtersCopied();
}

auto QmlApplication::clockFromFrames(int frames) -> QString {
	if (MLT.producer()) {
		return MLT.producer()->frames_to_time(frames, Settings.timeFormat());
	}
	return {};
}

auto QmlApplication::timeFromFrames(int frames) -> QString {
	if (MLT.producer()) {
		return MLT.producer()->frames_to_time(frames, Settings.timeFormat());
	}
	return {};
}

auto QmlApplication::audioChannels() -> int {
	return MLT.audioChannels();
}

auto QmlApplication::getNextProjectFile(const QString& filename) -> QString {
	QDir const dir(MLT.projectFolder());
	if (!MLT.projectFolder().isEmpty() && dir.exists()) {
		QFileInfo const info(filename);
		QString basename  = info.completeBaseName();
		QString extension = info.suffix();
		if (extension.isEmpty()) {
			extension = basename;
			basename = QString();
		}
		for (unsigned i = 1; i < std::numeric_limits<unsigned>::max(); i++) {
			QString const filename = QString::fromLatin1("%1%2.%3").arg(basename).arg(i).arg(extension);
			if (!dir.exists(filename))
				return dir.filePath(filename);
		}
	}
	return {};
}

auto QmlApplication::isProjectFolder() -> bool {
	QDir const dir(MLT.projectFolder());
	return (!MLT.projectFolder().isEmpty() && dir.exists());
}

auto QmlApplication::devicePixelRatio() -> qreal {
	return MAIN.devicePixelRatioF();
}

void QmlApplication::showStatusMessage(const QString& message, int timeoutSeconds) {
	MAIN.showStatusMessage(message, timeoutSeconds);
}

auto QmlApplication::maxTextureSize() -> int {
	auto* videoWidget = qobject_cast<Mlt::VideoWidget*>(MLT.videoWidget());
	return videoWidget ? videoWidget->maxTextureSize() : 0;
}

auto QmlApplication::confirmOutputFilter() -> bool {
	bool result = true;
	if (MAIN.filterController()->isOutputTrackSelected() && Settings.askOutputFilter()) {
		QMessageBox dialog(QMessageBox::Warning, qApp->applicationName(),
						   tr("<p>Do you really want to add filters to <b>Output</b>?</p>"
							  "<p><b>Timeline > Output</b> is currently selected. "
							  "Adding filters to <b>Output</b> affects ALL clips in the "
							  "timeline including new ones that will be added.</p>"),
						   QMessageBox::No | QMessageBox::Yes, &MAIN);
		dialog.setWindowModality(dialogModality());
		dialog.setDefaultButton(QMessageBox::No);
		dialog.setEscapeButton(QMessageBox::Yes);
		dialog.setCheckBox(new QCheckBox(tr("Do not show this anymore.", "confirm output filters dialog")));
		result = dialog.exec() == QMessageBox::Yes;
		if (dialog.checkBox()->isChecked()) {
			Settings.setAskOutputFilter(false);
		}
	}
	return result;
}

auto QmlApplication::dataDir() -> QDir {
	QDir dir(qApp->applicationDirPath());
#if defined(Q_OS_MAC)
	dir.cdUp();
	dir.cd("Resources");
#else
#if defined(Q_OS_UNIX) || (defined(Q_OS_WIN) && defined(NODEPLOY))
	dir.cdUp();
#endif
	dir.cd("share");
#endif
	return dir;
}

auto QmlApplication::contrastingColor(const QString& color) -> QColor {
	return Util::textColor(color);
}

auto QmlApplication::wipes() -> QStringList {
	QStringList result;
	const auto  transitions = QString::fromLatin1("transitions");
	QDir dir(Settings.appDataLocation());
	if (!dir.exists(transitions)) {
		dir.mkdir(transitions);
	}
	if (dir.cd(transitions)) {
		for (auto& s : dir.entryList(QDir::Files | QDir::Readable)) {
			result << dir.filePath(s);
		}
	}
	return result;
}

auto QmlApplication::addWipe(const QString& filePath) -> bool {
	const auto transitions = QString::fromLatin1("transitions");
	QDir dir(Settings.appDataLocation());
	if (!dir.exists(transitions)) {
		dir.mkdir(transitions);
	}
	if (dir.cd(transitions)) {
		return QFile::copy(filePath, dir.filePath(QFileInfo(filePath).fileName()));
	}
	return false;
}

auto QmlApplication::intersects(const QRectF& a, const QRectF& b) -> bool {
	return a.intersects(b);
}