/*
 * Copyright (c) 2013-2026 Meltytech, LLC
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

#ifndef AUDIOLEVELSTASK_HPP
#define AUDIOLEVELSTASK_HPP

// Qt
#include <MltProducer.h>
#include <MltProfile.h>
#include <QList>
#include <QPersistentModelIndex>
#include <QRunnable>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qscopedpointer.h>

class AudioLevelsTask : public QRunnable {
  public:
	AudioLevelsTask(Mlt::Producer& producer, QObject* object, const QModelIndex& index);
	~AudioLevelsTask() override;
	static void start(Mlt::Producer& producer, QObject* object, const QModelIndex& index, bool force = false);
	static void closeAll();
	auto operator==(AudioLevelsTask& b) -> bool;

  protected:
	void run() override;

  private:
	auto tempProducer() -> Mlt::Producer*;
	auto cacheKey() -> QString;
	void notifyQObjects(const QPersistentModelIndex& index);

	QObject*                                             m_object;
	QObject*                                             m_qmlProducer;
	typedef QPair<Mlt::Producer*, QPersistentModelIndex> ProducerAndIndex;
	QList<ProducerAndIndex>                              m_producers;
	QScopedPointer<Mlt::Producer>                        m_tempProducer;
	bool                                                 m_isCanceled;
	bool                                                 m_isForce;
	Mlt::Profile                                         m_profile;
};

#endif // AUDIOLEVELSTASK_HPP
