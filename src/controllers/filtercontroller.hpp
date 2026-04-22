/*
 * Copyright (c) 2014-2025 Meltytech, LLC
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

#ifndef FILTERCONTROLLER_HPP
#define FILTERCONTROLLER_HPP

// Local
#include "models/attachedfiltersmodel.hpp"
#include "models/metadatamodel.hpp"
#include "models/motiontrackermodel.hpp"
#include "qmltypes/qmlfilter.hpp"
#include "qmltypes/qmlmetadata.hpp"

// Qt
#include <MltProducer.h>
#include <QFuture>
#include <QObject>
#include <QScopedPointer>
#include <qcontainerfwd.h>
#include <qtmetamacros.h>

class QTimerEvent;

class FilterController : public QObject {
	Q_OBJECT

  public:
	explicit FilterController(QObject* parent = nullptr);
	auto metadataModel() -> MetadataModel*;

	auto motionTrackerModel() -> MotionTrackerModel* {
		return &m_motionTrackerModel;
	}

	auto attachedModel() -> AttachedFiltersModel*;

	auto metadata(const QString& id) -> QmlMetadata*;
	auto metadataForService(Mlt::Service* service) -> QmlMetadata*;

	auto currentFilter() const -> QmlFilter* {
		return m_currentFilter.data();
	}

	auto isOutputTrackSelected() const -> bool;
	void onUndoOrRedo(Mlt::Service& service);

	auto currentIndex() const -> int {
		return m_currentFilterIndex;
	}

	void addOrEditFilter(Mlt::Filter* filter, const QStringList& key_properties);
	void setTrackTransitionService(const QString& service);

  protected:
	void timerEvent(QTimerEvent*) override;

  signals:
	void currentFilterChanged(QmlFilter* filter, QmlMetadata* meta, int index);
	void statusChanged(QString);
	void filterChanged(Mlt::Service*);
	void undoOrRedo();

  public slots:
	void setProducer(Mlt::Producer* producer = nullptr);
	void setCurrentFilter(int attachedIndex);
	void onFadeInChanged();
	void onFadeOutChanged();
	void onGainChanged();
	void onServiceInChanged(int delta, Mlt::Service* service = nullptr);
	void onServiceOutChanged(int delta, Mlt::Service* service = nullptr);
	void removeCurrent();
	void onProducerChanged();
	void pauseUndoTracking();
	void resumeUndoTracking();

  private slots:
	void handleAttachedModelChange();
	void handleAttachedModelAboutToReset();
	void addMetadata(QmlMetadata*);
	void handleAttachedRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
	void handleAttachedRowsRemoved(const QModelIndex& parent, int first, int last);
	void handleAttachedRowsInserted(const QModelIndex& parent, int first, int last);
	void handleAttachDuplicateFailed(int index);
	void onQmlFilterChanged(const QString& name);

  private:
	void loadFilterSets();
	void loadFilterMetadata();

	QFuture<void>             m_future;
	QScopedPointer<QmlFilter> m_currentFilter;
	Mlt::Service              m_mltService;
	MetadataModel             m_metadataModel;
	MotionTrackerModel        m_motionTrackerModel;
	AttachedFiltersModel      m_attachedModel;
	int                       m_currentFilterIndex;
};

#endif // FILTERCONTROLLER_HPP
