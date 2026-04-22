/*
 * Copyright (c) 2012-2024 Meltytech, LLC
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

#ifndef JOBQUEUE_HPP
#define JOBQUEUE_HPP

// Local
#include "jobs/abstractjob.hpp"

// Qt
#include <QMutex>
#include <QStandardItemModel>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>

#define JOBS JobQueue::singleton()

class JobQueue : public QStandardItemModel {
	Q_OBJECT
  protected:
	JobQueue(QObject* parent);
	void startNextJob();

  public:
	enum ColumnRole { COLUMN_ICON, COLUMN_OUTPUT, COLUMN_STATUS, COLUMN_COUNT };

	static JobQueue& singleton(QObject* parent = nullptr);
	void cleanup();
	AbstractJob* add(AbstractJob* job);
	[[nodiscard]] AbstractJob* jobFromIndex(const QModelIndex& index) const;
	void pause();
	void pauseCurrent();
	void resume();
	void resumeCurrent();
	[[nodiscard]] bool isPaused() const;
	[[nodiscard]] bool hasIncomplete() const;
	void remove(const QModelIndex& index);
	void removeFinished();

	[[nodiscard]] QList<AbstractJob*> jobs() const {
		return m_jobs;
	}

	bool targetIsInProgress(const QString& target);

  signals:
	void jobAdded();

  public slots:
	void onProgressUpdated(QStandardItem* standardItem, int percent);
	void onFinished(AbstractJob* job, bool isSuccess, const QString& time);

  private:
	QList<AbstractJob*> m_jobs;
	QMutex m_mutex; // protects m_jobs
	bool m_paused;
};

#endif // JOBQUEUE_HPP
