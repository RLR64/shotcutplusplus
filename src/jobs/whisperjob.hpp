/*
 * Copyright (c) 2024 Meltytech, LLC
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

#ifndef WHISPERJOB_HPP
#define WHISPERJOB_HPP

// Local
#include "abstractjob.hpp"
#include "jobs/postjobaction.hpp"
#include "settings.hpp"

// Qt
#include <QTemporaryFile>
#include <qthread.h>
#include <qtmetamacros.h>

class WhisperJob : public AbstractJob {
	Q_OBJECT
  public:
	WhisperJob(const QString& name, const QString& iWavFile, const QString& oSrtFile, const QString& lang,
	           bool translate, int maxLength, QThread::Priority priority = Settings.jobPriority());
	~WhisperJob() override;

  public slots:
	void start() override;
	void onViewSrtTriggered();

  protected slots:
	void onReadyRead() override;

  private:
	const QString m_iWavFile;
	const QString m_oSrtFile;
	const QString m_lang;
	const bool    m_translate;
	const int     m_maxLength;
	int           m_previousPercent;
};

#endif // WHISPERJOB_HPP
