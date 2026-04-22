/*
 * Copyright (c) 2020 Meltytech, LLC
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

#ifndef QIMAGEJOB_HPP
#define QIMAGEJOB_HPP

// Local
#include "abstractjob.hpp"
#include "jobs/postjobaction.hpp"

// Qt
#include <QSize>
#include <qtmetamacros.h>

class QImageJob : public AbstractJob {
	Q_OBJECT
  public:
	QImageJob(const QString& destFilePath, const QString& srcFilePath, const int height);
	~QImageJob() override;
	void start() override;
	void execute();

  private:
	QString m_srcFilePath;
	QString m_destFilePath;
	int     m_height;
};

#endif // QIMAGEJOB_HPP
