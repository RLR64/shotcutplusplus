/*
 * Copyright (c) 2016 Meltytech, LLC
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
#include "ffprobejob.hpp"
#include "Logger.hpp"
#include "dialogs/textviewerdialog.hpp"
#include "jobs/abstractjob.hpp"
#include "jobs/postjobaction.hpp"
#include "mainwindow.hpp"
#include "util.hpp"

// Qt
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <qcontainerfwd.h>
#include <qprocess.h>

FfprobeJob::FfprobeJob(const QString& name, const QStringList& args) : AbstractJob(name) {
	m_args.append(args);
}

FfprobeJob::~FfprobeJob() = default;

void FfprobeJob::start() {
	QString const shotcutPath = qApp->applicationDirPath();
	QFileInfo const ffprobePath(shotcutPath, "ffprobe");
	setReadChannel(QProcess::StandardOutput);
	LOG_DEBUG() << ffprobePath.absoluteFilePath() + " " + m_args.join(' ');
	AbstractJob::start(ffprobePath.absoluteFilePath(), m_args);
}

void FfprobeJob::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	AbstractJob::onFinished(exitCode, exitStatus);
	if (exitStatus == QProcess::NormalExit && exitCode == 0) {
		TextViewerDialog dialog(&MAIN);
		dialog.setWindowTitle(tr("More Information"));
		dialog.setText(log().replace("\\:", ":"));
		dialog.exec();
	}
	deleteLater();
}
