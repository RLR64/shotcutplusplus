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

#ifndef LONGUITASK_HPP
#define LONGUITASK_HPP

// Qt
#include <QFuture>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include <qcoreapplication.h>
#include <qhashfunctions.h>
#include <qthread.h>

constexpr int MS_SLEEP = {100};

class LongUiTask : public QProgressDialog {
  public:
	explicit LongUiTask(const QString& title);
	~LongUiTask() override;

	template <class Ret> auto wait(const QString& text, const QFuture<Ret>& future) -> Ret {
		setLabelText(text);
		setRange(0, 0);
		while (!future.isFinished()) {
			setValue(0);
			QCoreApplication::processEvents();
			QThread::msleep(MS_SLEEP);
		}
		return future.result();
	}

	template <class Ret, class Func, class... Args> auto runAsync(QString text, Func&& f, Args&&... args) -> Ret {
		QFuture<Ret> future = QtConcurrent::run(f, std::forward<Args>(args)...);
		return wait<Ret>(text, future);
	}

	void        reportProgress(const QString& text, int value, int max);
	static void cancel();
};

#endif // LONGUITASK_HPP
