/*
 * Copyright (c) 2016-2022 Meltytech, LLC
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

#ifndef FFMPEGJOB_HPP
#define FFMPEGJOB_HPP

// Local
#include "abstractjob.hpp"
#include "jobs/postjobaction.hpp"
#include "settings.hpp"

// Qt
#include <QStringList>
#include <qcontainerfwd.h>
#include <qthread.h>
#include <qtmetamacros.h>

class FfmpegJob : public AbstractJob {
	Q_OBJECT
  public:
	FfmpegJob(const QString& name, const QStringList& args, bool isOpenLog = true,
	          QThread::Priority priority = Settings.jobPriority());
	~FfmpegJob() override;
	void start() override;

  public slots:
	void stop() override;

  private slots:
	void onOpenTriggered();
	void onReadyRead() override;

  private:
	QStringList m_args;
	double      m_duration;
	int         m_previousPercent;
	bool        m_isOpenLog;
};

#endif // FFMPEGJOB_HPP
