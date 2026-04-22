/*
 * Copyright (c) 2013-2024 Meltytech, LLC
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

#ifndef FILTERSDOCK_HPP
#define FILTERSDOCK_HPP

// Local
#include "qmltypes/qmlproducer.hpp"
#include "sharedframe.hpp"

// Qt
#include <MltService.h>
#include <QDockWidget>
#include <QObject>
#include <QQuickWidget>
#include <qevent.h>
#include <qtmetamacros.h>

class QmlFilter;
class QmlMetadata;
class MetadataModel;
class AttachedFiltersModel;
class MotionTrackerModel;
class SubtitlesModel;

class FiltersDock : public QDockWidget {
	Q_OBJECT

  public:
	explicit FiltersDock(MetadataModel* metadataModel, AttachedFiltersModel* attachedModel,
	                     MotionTrackerModel* motionTrackerModel, SubtitlesModel* subtitlesModel,
	                     QWidget* parent = nullptr);

	auto qmlProducer() -> QmlProducer* {
		return &m_producer;
	}

  signals:
	void currentFilterRequested(int attachedIndex);
	void changed(); /// Notifies when a filter parameter changes.
	void seeked(int);
	void producerInChanged(int delta);
	void producerOutChanged(int delta);

  public slots:
	void setCurrentFilter(QmlFilter* filter, QmlMetadata* meta, int index);
	void onSeeked(int position);
	void onShowFrame(const SharedFrame& frame);
	void openFilterMenu() const;
	void showCopyFilterMenu();
	void onServiceInChanged(int delta, Mlt::Service* service);
	void load();

  protected:
	auto event(QEvent* event) -> bool override;
	void keyPressEvent(QKeyEvent* event) override;

  private:
	void         setupActions();
	QQuickWidget m_qview;
	QmlProducer  m_producer;
	unsigned     loadTries{0};
};

#endif // FILTERSDOCK_HPP
