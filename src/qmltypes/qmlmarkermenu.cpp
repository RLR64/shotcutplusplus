/*
 * Copyright (c) 2022-2025 Meltytech, LLC
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
#include "qmlmarkermenu.hpp"
#include "actions.hpp"
#include "docks/timelinedock.hpp"
#include "qmltypes/colordialog.hpp"
#include "qmltypes/qmlapplication.hpp"

// Qt
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>
#include <qcontainerfwd.h>
#include <qobject.h>

// STL
#include <utility>

QmlMarkerMenu::QmlMarkerMenu(QObject* parent) : QObject(parent), m_timeline(nullptr), m_index(-1) {
}

auto QmlMarkerMenu::target() -> QObject* {
	return m_timeline;
}

void QmlMarkerMenu::setTarget(QObject* target) {
	m_timeline = dynamic_cast<TimelineDock*>(target);
}

auto QmlMarkerMenu::index() -> int {
	return m_index;
}

void QmlMarkerMenu::setIndex(int index) {
	m_index = index;
}

void QmlMarkerMenu::popup() {
	if (!m_timeline || m_index < 0)
		return;

	QMenu menu;

	QAction editAction(tr("Edit..."));
	editAction.setShortcut(Actions["timelineMarkerAction"]->shortcut());
	connect(&editAction, &QAction::triggered, this, [&]() -> void { m_timeline->editMarker(m_index); });
	menu.addAction(&editAction);

	QAction deleteAction(tr("Delete"));
	deleteAction.setShortcut(Actions["timelineDeleteMarkerAction"]->shortcut());
	connect(&deleteAction, &QAction::triggered, this, [&]() -> void { m_timeline->deleteMarker(m_index); });
	menu.addAction(&deleteAction);

	QAction colorAction(tr("Choose Color..."));
	connect(&colorAction, &QAction::triggered, this, [&]() -> void {
		QColor const markerColor = m_timeline->markersModel()->getMarker(m_index).color;
		auto newColor    = ColorDialog::getColor(markerColor, nullptr, QString(), false);
		if (newColor.isValid()) {
			m_timeline->markersModel()->setColor(m_index, newColor);
		}
	});
	menu.addAction(&colorAction);

	QMenu* recentColorMenu = menu.addMenu(tr("Choose Recent Color"));
	QStringList colors = m_timeline->markersModel()->recentColors();
	QString const highlightColor = QApplication::palette().highlight().color().name();
	for (const auto & color : std::as_const(colors)) {
		auto* widgetAction = new QWidgetAction(recentColorMenu);
		auto* colorButton  = new QToolButton();
		colorButton->setText(color);
		QString const textColor  = QmlApplication::contrastingColor(color).name();
		QString const styleSheet = QString("QToolButton {"
		                             "    background-color: %1;"
		                             "    border-style: solid;"
		                             "    border-width: 3px;"
		                             "    border-color: %1;"
		                             "    color: %2"
		                             "}"
		                             "QToolButton:hover {"
		                             "    background-color: %1;"
		                             "    border-style: solid;"
		                             "    border-width: 3px;"
		                             "    border-color: %3;"
		                             "    color: %2"
		                             "}")
								 .arg(color)
		                         .arg(textColor)
		                         .arg(highlightColor);
		colorButton->setStyleSheet(styleSheet);
		connect(colorButton, &QToolButton::clicked, this, [&, colorButton]() -> void {
			m_timeline->markersModel()->setColor(m_index, colorButton->text());
			menu.close();
		});
		widgetAction->setDefaultWidget(colorButton);
		recentColorMenu->addAction(widgetAction);
	}

	menu.exec(QCursor::pos());
}
