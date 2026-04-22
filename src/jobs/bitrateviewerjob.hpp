/*
 * Copyright (c) 202-2025 Meltytech, LLC
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

#ifndef BITRATEVIEWERJOB_HPP
#define BITRATEVIEWERJOB_HPP

// Local
#include "ffprobejob.hpp"
#include "jobs/postjobaction.hpp"

// Qt
#include <QJsonArray>
#include <qcontainerfwd.h>
#include <qprocess.h>
#include <qtmetamacros.h>

class BitrateViewerJob : public FfprobeJob {
	Q_OBJECT
  public:
	BitrateViewerJob(const QString& name, const QStringList& args, double fps);
	~BitrateViewerJob() override;

  private slots:
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus) override;
	void onOpenTriggered();

  private:
	QString    m_resource;
	double     m_fps{0.0};
	QJsonArray m_data;
};

#endif // BITRATEVIEWERJOB_HPP
