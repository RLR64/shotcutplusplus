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
#include "abstractjob.hpp"
#include "Logger.hpp"
#include "postjobaction.hpp"

// Qt
#include <QAction>
#include <QApplication>
#include <QTimer>
#include <debugapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qobjectdefs.h>
#include <qprocess.h>
#include <qthread.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <winnt.h>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <signal.h>
#endif

// Number constants
constexpr int singleShotNumber{2000};

AbstractJob::AbstractJob(const QString& name, QThread::Priority priority)
	: QProcess(nullptr), m_item(nullptr), m_ran(false), m_killed(false), m_label(name), m_startingPercent(0), m_priority(priority),
      m_isPaused(false) {
	setObjectName(name);
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
	connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(this, SIGNAL(started()), this, SLOT(onStarted()));
	connect(this, SIGNAL(progressUpdated(QStandardItem*, int)), SLOT(onProgressUpdated(QStandardItem*, int)));
	m_actionPause = new QAction(tr("Pause This Job"), this);
	m_standardActions << m_actionPause;
	m_actionPause->setEnabled(false);
	m_actionResume = new QAction(tr("Resume This Job"), this);
	m_actionResume->setEnabled(false);
	m_standardActions << m_actionResume;

	connect(m_actionPause, &QAction::triggered, this, &AbstractJob::pause);
	connect(m_actionResume, &QAction::triggered, this, &AbstractJob::resume);
	connect(this, &AbstractJob::finished, this, [this]() -> void {
		m_actionPause->setEnabled(false);
		m_actionResume->setEnabled(false);
	});
}

void AbstractJob::start() {
	m_killed = false;
	m_ran    = true;
	m_estimateTime.start();
	m_totalTime.start();
	emit progressUpdated(m_item, 0);
}

void AbstractJob::setStandardItem(QStandardItem* item) {
	m_item = item;
}

auto AbstractJob::standardItem() -> QStandardItem* {
	return m_item;
}

auto AbstractJob::ran() const -> bool {
	return m_ran;
}

auto AbstractJob::stopped() const -> bool {
	return m_killed;
}

void AbstractJob::appendToLog(const QString& s) {
	if (m_log.size() < 100 * 1024 * 1024 /* MiB */) {
		m_log.append(s);
	}
}

auto AbstractJob::log() const -> QString {
	return m_log;
}

void AbstractJob::setLabel(const QString& label) {
	m_label = label;
}

auto AbstractJob::estimateRemaining(int percent) -> QTime {
	QTime result;
	if (percent) {
		const int averageMs = m_estimateTime.elapsed() / qMax(1, percent - qMax(0, m_startingPercent));
		result        = QTime::fromMSecsSinceStartOfDay(averageMs * (100 - percent));
	}
	return result;
}

void AbstractJob::setPostJobAction(PostJobAction* action) {
	m_postJobAction.reset(action);
}

auto AbstractJob::paused() const -> bool {
	return m_isPaused;
}

void AbstractJob::start(const QString& program, const QStringList& arguments) {
	QString const& prog = program;
	QStringList const& args = arguments;
#ifndef Q_OS_WIN
	if (m_priority == QThread::LowPriority || m_priority == QThread::HighPriority) {
		args.prepend(program);
		args.prepend(m_priority == QThread::LowPriority ? "3" : "-3");
		args.prepend("-n");
		prog = "nice";
	}
#endif
	QProcess::start(prog, args);
	AbstractJob::start();
	m_actionPause->setEnabled(true);
	m_actionResume->setEnabled(false);
	m_isPaused = false;
}

void AbstractJob::stop() {
	if (paused()) {
#ifdef Q_OS_WIN
		::DebugActiveProcessStop(QProcess::processId());
#else
		::kill(QProcess::processId(), SIGCONT);
#endif
	}
	closeWriteChannel();
	terminate();
	QTimer::singleShot(singleShotNumber, this, SLOT(kill()));
	m_killed = true;
	m_actionPause->setEnabled(false);
	m_actionResume->setEnabled(false);
}

void AbstractJob::pause() {
	m_isPaused = true;
	m_actionPause->setEnabled(false);
	m_actionResume->setEnabled(true);

#ifdef Q_OS_WIN
	::DebugActiveProcess(QProcess::processId());
#else
	::kill(QProcess::processId(), SIGSTOP);
#endif
	emit progressUpdated(m_item, -1);
}

void AbstractJob::resume() {
	m_actionPause->setEnabled(true);
	m_actionResume->setEnabled(false);
	m_startingPercent = -1;
#ifdef Q_OS_WIN
	::DebugActiveProcessStop(QProcess::processId());
#else
	::kill(QProcess::processId(), SIGCONT);
#endif
	m_isPaused = false;
	emit progressUpdated(m_item, 0);
}

void AbstractJob::setKilled(bool killed) {
	m_killed = killed;
}

void AbstractJob::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	const QTime& time = QTime::fromMSecsSinceStartOfDay(m_totalTime.elapsed());
	if (isOpen()) {
		m_log.append(readAll());
	}
	if (exitStatus == QProcess::NormalExit && exitCode == 0 && !m_killed) {
		if (m_postJobAction) {
			m_postJobAction->doAction();
		}
		LOG_INFO() << "job succeeded";
		m_log.append(QStringLiteral("Completed successfully in %1\n").arg(time.toString()));
		emit progressUpdated(m_item, 100);
		emit finished(this, true);
	} else if (m_killed) {
		LOG_INFO() << "job stopped";
		m_log.append(QStringLiteral("Stopped by user at %1\n").arg(time.toString()));
		emit finished(this, false);
	} else {
		LOG_INFO() << "job failed with" << exitCode;
		m_log.append(QStringLiteral("Failed with exit code %1\n").arg(exitCode));
		emit finished(this, false);
	}
	m_isPaused = false;
}

void AbstractJob::onReadyRead() {
	QString msg;
	do {
		msg = readLine();
		appendToLog(msg);
	} while (!msg.isEmpty());
}

void AbstractJob::onStarted() {
#ifdef Q_OS_WIN
	qint64 const processId = QProcess::processId();
	HANDLE processHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processId);
	if (processHandle) {
		switch (m_priority) {
		case QThread::LowPriority:
			SetPriorityClass(processHandle, BELOW_NORMAL_PRIORITY_CLASS);
			break;
		case QThread::HighPriority:
			SetPriorityClass(processHandle, ABOVE_NORMAL_PRIORITY_CLASS);
			break;
		default:
			SetPriorityClass(processHandle, NORMAL_PRIORITY_CLASS);
		}
		CloseHandle(processHandle);
	}
#endif
}

void AbstractJob::onProgressUpdated(QStandardItem*, int percent) {
	// Start timer on first reported percentage > 0.
	if (percent == 1 || m_startingPercent < 0) {
		m_estimateTime.restart();
		m_startingPercent = percent;
	}
}
