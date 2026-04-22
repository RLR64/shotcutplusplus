/*
 * Copyright (c) 2012-2025 Meltytech, LLC
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
#include "jobqueue.hpp"
#include "Logger.hpp"
#include "jobs/abstractjob.hpp"
#include "jobs/postjobaction.hpp"

// Qt
#include <QtWidgets>
#include <qalgorithms.h>
#include <qdir.h>
#include <qfontdatabase.h>
#include <qforeach.h>
#include <qguiapplication.h>
#include <qlist.h>
#include <qmutex.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qprocess.h>
#include <qstandarditemmodel.h>
#include <qtmetamacros.h>
#include <qtversionchecks.h>

// STL
#include <utility>

// clang-format off
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include "windowstools.h"
#endif
// clang-on

JobQueue::JobQueue(QObject* parent) : QStandardItemModel(0, COLUMN_COUNT, parent), m_paused(false) {
}

auto JobQueue::singleton(QObject* parent) -> JobQueue& {
	static JobQueue* instance = nullptr;
	if (!instance)
		instance = new JobQueue(parent);
	return *instance;
}

void JobQueue::cleanup() {
	QMutexLocker const locker(&m_mutex);
	foreach (AbstractJob* job, m_jobs) {
		if (job->state() == QProcess::Running) {
			job->stop();
			break;
		}
	}
	qDeleteAll(m_jobs);
}

auto JobQueue::add(AbstractJob* job) -> AbstractJob* {
	QList<QStandardItem*> items;
	QIcon const icon = QIcon::fromTheme("run-build", QIcon(":/icons/oxygen/32x32/actions/run-build.png"));
	items << new QStandardItem(icon, "");
	auto* item = new QStandardItem(job->label());
	items << item;
	item = new QStandardItem(tr("pending"));
	QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	font.setPointSize(QGuiApplication::font().pointSize());
	item->setFont(font);
	item->setToolTip(tr("Estimated Hours:Minutes:Seconds"));
	items << item;
	appendRow(items);
	job->setParent(this);
	job->setStandardItem(item);
	connect(job, SIGNAL(progressUpdated(QStandardItem*, int)), SLOT(onProgressUpdated(QStandardItem*, int)));
	connect(job, SIGNAL(finished(AbstractJob*, bool, QString)), SLOT(onFinished(AbstractJob*, bool, QString)));
	m_mutex.lock();
	m_jobs.append(job);
	m_mutex.unlock();
	emit jobAdded();
	startNextJob();

	return job;
}

void JobQueue::onProgressUpdated(QStandardItem* standardItem, int percent) {
	if (standardItem) {
		AbstractJob* job = m_jobs.at(standardItem->row());
		if (job) {
			QString remaining("--:--:--");
			QIcon   icon(":/icons/oxygen/32x32/actions/run-build.png");
			if (job->paused()) {
				icon      = QIcon(":/icons/oxygen/32x32/actions/media-playback-pause.png");
				remaining = tr("paused");
			} else if (percent > 0) {
				auto time = job->estimateRemaining(percent);
				if (QTime(0, 0).secsTo(time) == 0)
					return;
				if (percent > 2)
					remaining = time.toString();
				remaining = QStringLiteral("%1% (%2)").arg(percent).arg(remaining);
			} else if (percent < 0) {
				remaining = QTime(0, 0).addSecs(-percent).toString();
			}
			standardItem->setText(remaining);
			standardItem = JOBS.item(standardItem->row(), JobQueue::COLUMN_ICON);
			if (standardItem)
				standardItem->setIcon(icon);
		}
	}
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	WindowsTaskbarButton::getInstance().setProgress(percent);
#endif
}

void JobQueue::onFinished(AbstractJob* job, bool isSuccess, const QString& time) {
	QStandardItem* item = job->standardItem();
	if (item) {
		QIcon icon;
		if (isSuccess) {
			const QTime& time = QTime::fromMSecsSinceStartOfDay(job->time().elapsed());
			item->setText(time.toString());
			item->setToolTip(tr("Elapsed Hours:Minutes:Seconds"));
			icon = QIcon(":/icons/oxygen/32x32/status/task-complete.png");
		} else if (job->stopped()) {
			item->setText(tr("stopped"));
			icon = QIcon(":/icons/oxygen/32x32/status/task-attempt.png");
		} else {
			item->setText(tr("failed").append(' ').append(time));
			icon = QIcon(":/icons/oxygen/32x32/status/task-reject.png");
		}

		// Remove any touched or incomplete pending proxy files
		if (job->stopped() || !isSuccess)
			if (job->objectName().contains("proxies") && job->objectName().contains(".pending")) {
				QFile::remove(job->objectName());
			}

		item = JOBS.item(item->row(), JobQueue::COLUMN_ICON);
		if (item)
			item->setIcon(icon);
	}
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	WindowsTaskbarButton::getInstance().resetProgress();
#endif

	startNextJob();
}

void JobQueue::startNextJob() {
	if (m_paused)
		return;
	QMutexLocker const locker(&m_mutex);
	if (!m_jobs.isEmpty()) {
		foreach (AbstractJob* job, m_jobs) {
			// if there is already a job started or running, then exit
			if (job->ran() && job->state() != QProcess::NotRunning)
				break;
			// otherwise, start first non-started job and exit
			if (!job->ran()) {
				job->start();
				break;
			}
		}
	}
}

auto JobQueue::jobFromIndex(const QModelIndex& index) const -> AbstractJob* {
	return m_jobs.at(index.row());
}

void JobQueue::pause() {
	m_paused = true;
}

void JobQueue::pauseCurrent() {
	for (const auto job : std::as_const(m_jobs)) {
		if (job->state() == QProcess::Running) {
			job->pause();
			break;
		}
	}
}

void JobQueue::resume() {
	m_paused = false;
	startNextJob();
}

void JobQueue::resumeCurrent() {
	for (const auto job : std::as_const(m_jobs)) {
		if (job->state() == QProcess::Running) {
			job->resume();
			break;
		}
	}
}

auto JobQueue::isPaused() const -> bool {
	return m_paused;
}

auto JobQueue::hasIncomplete() const -> bool {
	foreach (AbstractJob const* job, m_jobs) {
		if (!job->ran() || job->state() == QProcess::Running)
			return true;
	}
	return false;
}

void JobQueue::remove(const QModelIndex& index) {
	const int row = index.row();
	removeRow(index.row());
	m_mutex.lock();

	AbstractJob const* job = m_jobs.at(row);
	m_jobs.removeOne(job);
	delete job;

	m_mutex.unlock();
}

void JobQueue::removeFinished() {
	QMutexLocker const locker(&m_mutex);
	auto row = 0;
	foreach (AbstractJob const* job, m_jobs) {
		if (job->isFinished()) {
			removeRow(row);
			m_jobs.removeOne(job);
			delete job;
		} else {
			++row;
		}
	}
}

auto JobQueue::targetIsInProgress(const QString& target) -> bool {
	if (!m_jobs.isEmpty() && !target.isEmpty()) {
		foreach (AbstractJob* job, m_jobs) {
			if (!job->isFinished() && job->target() == target) {
				return true;
			}
		}
	}
	return false;
}
