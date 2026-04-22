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

#ifndef SCREENCAPTUREJOB_HPP
#define SCREENCAPTUREJOB_HPP

// Local
#include "abstractjob.hpp"
#include "jobs/postjobaction.hpp"

// Qt
#include <QRect>
#include <QTimer>
#include <qprocess.h>
#include <qtmetamacros.h>

// clang-format off
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include <QDBusConnection>
#endif
// clang-format on

class ScreenCaptureJob : public AbstractJob {
	Q_OBJECT
  public:
	ScreenCaptureJob(const QString& name, const QString& filename, const QRect& captureRect, bool recordAudio = true);
	~ScreenCaptureJob() override;
	void start() override;
	void stop() override;

  private slots:
	void onOpenTriggered();
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus) override;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	void onDBusRecordingTaken(const QString& fileName);
	void onDBusRecordingFailed();
#endif

  private:
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	enum DBusService { None, GNOME, KDE };

	bool startWaylandRecording();
	bool startGnomeScreencast();
	bool startKdeSpectacle();
#endif
	QString m_filename;
	QString m_actualFilename;
	QRect   m_rect;
	bool    m_isAutoOpen;
	bool    m_recordAudio;
	QTimer  m_progressTimer;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	DBusService m_dbusService = DBusService::None;
#endif
};

#endif // SCREENCAPTUREJOB_HPP
