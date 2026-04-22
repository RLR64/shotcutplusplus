/*
 * Copyright (c) 2023-2024 Meltytech, LLC
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

#ifndef RESOURCEMODEL_HPP
#define RESOURCEMODEL_HPP

// Qt
#include <MltProducer.h>
#include <QAbstractItemModel>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

class ResourceModel : public QAbstractItemModel {
	Q_OBJECT

  public:
	enum Columns {
		COLUMN_INFO = 0,
		COLUMN_NAME,
		COLUMN_SIZE,
		COLUMN_VID_DESCRIPTION,
		COLUMN_AUD_DESCRIPTION,
		COLUMN_COUNT,
	};

	explicit ResourceModel(QObject* parent = nullptr);
	~ResourceModel() override;
	void search(Mlt::Producer* producer);
	void add(Mlt::Producer* producer, const QString& location = QString());
	auto getProducers(const QModelIndexList& indices) -> QList<Mlt::Producer>;
	auto exists(const QString& hash) -> bool;
	auto producerCount() -> int;
	auto producer(int index) -> Mlt::Producer;
	// Implement QAbstractItemModel
	[[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	[[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
	[[nodiscard]] auto index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const -> QModelIndex override;
	[[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

  private:
	QList<Mlt::Producer>   m_producers;
	QMap<QString, QString> m_locations;
};

#endif // RESOURCEMODEL_HPP
