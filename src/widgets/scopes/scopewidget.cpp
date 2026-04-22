/*
 * Copyright (c) 2015-2022 Meltytech, LLC
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
#include "Logger.hpp"
#include "dataqueue.hpp"
#include "sharedframe.hpp"
#include "scopewidget.h"

// Qt
#include <QtConcurrent/qtconcurrentrun.h>
#include <qcontainerfwd.h>
#include <qcoreevent.h>
#include <qmutex.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qsize.h>
#include <qvariant.h>
#include <qwidget.h>

ScopeWidget::ScopeWidget(const QString& name)
    : QWidget(), m_queue(3, DataQueue<SharedFrame>::OverflowModeDiscardOldest), m_future(), m_refreshPending(false),
      m_mutex(), m_forceRefresh(false), m_size(0, 0) {
	LOG_DEBUG() << "begin" << m_future.isFinished();
	setObjectName(name);
	LOG_DEBUG() << "end";
}

ScopeWidget::~ScopeWidget() = default;

void ScopeWidget::onNewFrame(const SharedFrame& frame) {
	m_queue.push(frame);
	requestRefresh();
}

void ScopeWidget::requestRefresh() {
	if (m_future.isFinished()) {
		m_future = QtConcurrent::run(&ScopeWidget::refreshInThread, this);
	} else {
		m_refreshPending = true;
	}
}

void ScopeWidget::refreshInThread() {
	if (m_size.isEmpty()) {
		return;
	}

	m_mutex.lock();
	QSize const size = m_size;
	const bool full = m_forceRefresh;
	m_forceRefresh = false;
	m_mutex.unlock();

	m_refreshPending = false;
	refreshScope(size, full);
	// Tell the GUI thread that the refresh is complete.
	QMetaObject::invokeMethod(this, "onRefreshThreadComplete", Qt::QueuedConnection);
}

void ScopeWidget::onRefreshThreadComplete() {
	update();
	if (m_refreshPending) {
		requestRefresh();
	}
}

void ScopeWidget::resizeEvent(QResizeEvent*) {
	m_mutex.lock();
	m_size = size();
	m_mutex.unlock();
	if (isVisible()) {
		requestRefresh();
	}
}

void ScopeWidget::changeEvent(QEvent*) {
	m_mutex.lock();
	m_forceRefresh = true;
	m_mutex.unlock();
	if (isVisible()) {
		requestRefresh();
	}
}
