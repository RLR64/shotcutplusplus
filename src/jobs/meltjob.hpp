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

#ifndef MELTJOB_HPP
#define MELTJOB_HPP

// Local
#include "abstractjob.hpp"
#include "jobs/postjobaction.hpp"
#include "settings.hpp"

// Qt
#include <MltProfile.h>
#include <QTemporaryFile>
#include <qcontainerfwd.h>
#include <qprocess.h>
#include <qscopedpointer.h>
#include <qthread.h>
#include <qtmetamacros.h>

class MeltJob : public AbstractJob {
	Q_OBJECT
  public:
	MeltJob(const QString& name, const QString& xml, int frameRateNum, int frameRateDen,
	        QThread::Priority priority = Settings.jobPriority());
	MeltJob(const QString& name, const QStringList& args, int frameRateNum, int frameRateDen);
	MeltJob(const QString& name, const QString& xml, const QStringList& args, int frameRateNum, int frameRateDen);
	~MeltJob() override;
	auto xml() -> QString;

	[[nodiscard]] auto xmlPath() const -> QString {
		return m_xml->fileName();
	}

	void setIsStreaming(bool streaming);
	void setUseMultiConsumer(bool multi = true);
	void setInAndOut(int in, int out);

  public slots:
	void start() override;
	void onViewXmlTriggered();

  protected slots:
	virtual void onOpenTiggered();
	void         onFinished(int exitCode, QProcess::ExitStatus exitStatus) override;
	void         onShowFolderTriggered();
	void         onShowInFilesTriggered();
	void         onReadyRead() override;

  protected:
	QScopedPointer<QTemporaryFile> m_xml;

  private:
	bool         m_isStreaming;
	int          m_previousPercent;
	QStringList  m_args;
	int          m_currentFrame;
	Mlt::Profile m_profile;
	bool         m_useMultiConsumer;
	int          m_in{-1};
	int          m_out{-1};
};

#endif // MELTJOB_HPP
