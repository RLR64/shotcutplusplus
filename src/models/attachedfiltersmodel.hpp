/*
 * Copyright (c) 2013-2023 Meltytech, LLC
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

#ifndef ATTACHEDFILTERSMODEL_HPP
#define ATTACHEDFILTERSMODEL_HPP

// Qt
#include <MltEvent.h>
#include <MltFilter.h>
#include <MltProducer.h>
#include <QAbstractListModel>
#include <framework/mlt_types.h>
#include <qhash.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <qtmetamacros.h>

class QmlMetadata;

class AttachedFiltersModel : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(QString producerTitle READ producerTitle NOTIFY trackTitleChanged)
	Q_PROPERTY(bool isProducerSelected READ isProducerSelected NOTIFY isProducerSelectedChanged)
	Q_PROPERTY(bool supportsLinks READ supportsLinks NOTIFY supportsLinksChanged)
  public:
	enum ModelRoles {
		TypeDisplayRole = Qt::UserRole + 1,
		PluginTypeRole,
	};

	explicit AttachedFiltersModel(QObject* parent = nullptr);

	[[nodiscard]] auto getService(int row) const -> Mlt::Service*;
	[[nodiscard]] auto  getMetadata(int row) const -> QmlMetadata*;
	void setProducer(Mlt::Producer* producer = nullptr);
	[[nodiscard]] auto producerTitle() const -> QString;
	[[nodiscard]] auto isProducerSelected() const -> bool;
	[[nodiscard]] auto isSourceClip() const -> bool;
	[[nodiscard]] auto supportsLinks() const -> bool;

	[[nodiscard]] auto producer() const -> Mlt::Producer* {
		return m_producer.data();
	}

	[[nodiscard]] auto name(int row) const -> QString;

	// The below are used by QUndoCommands
	void doAddService(Mlt::Producer& producer, Mlt::Service& service, int row);
	void doRemoveService(Mlt::Producer& producer, int row);
	void doMoveService(Mlt::Producer& producer, int fromRow, int toRow);
	void doSetDisabled(Mlt::Producer& producer, int row, bool disable);
	auto doGetService(Mlt::Producer& producer, int row) -> Mlt::Service;

	// QAbstractListModel Implementation
	[[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
	[[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	[[nodiscard]] auto setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) -> bool override;
	[[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;
	[[nodiscard]] auto supportedDropActions() const -> Qt::DropActions override;
	[[nodiscard]] auto insertRows(int row, int count, const QModelIndex& parent) -> bool override;
	[[nodiscard]] auto removeRows(int row, int count, const QModelIndex& parent) -> bool override;
	[[nodiscard]] auto moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
								int destinationRow) -> bool override;

  signals:
	void changed();
	void duplicateAddFailed(int index);
	void trackTitleChanged();
	void isProducerSelectedChanged();
	void supportsLinksChanged();
	void addedOrRemoved(Mlt::Producer*);
	void requestConvert(QString, bool set709Convert, bool withSubClip);

  public slots:
	auto add(QmlMetadata* meta) -> int;
	auto addService(Mlt::Service* service) -> int;
	void remove(int row);
	auto move(int fromRow, int toRow) -> bool;
	void pasteFilters();

  private:
	static void producerChanged(mlt_properties owner, AttachedFiltersModel* model);
	void reset(Mlt::Producer* producer = nullptr);
	auto isProducerLoaded(Mlt::Producer& producer) const -> bool;
	auto findInsertRow(QmlMetadata* meta) -> int;
	auto getFilterSetProducer(QmlMetadata* meta) -> Mlt::Producer;

	int                           m_dropRow;
	int                           m_removeRow;
	QScopedPointer<Mlt::Producer> m_producer;
	QScopedPointer<Mlt::Event>    m_event;
	typedef QList<QmlMetadata*>   MetadataList;
	MetadataList                  m_metaList;
};

#endif // ATTACHEDFILTERSMODEL_HPP
