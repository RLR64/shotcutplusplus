/*
 * Copyright (c) 2022 Meltytech, LLC
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

#ifndef ACTIONSMODEL_HPP
#define ACTIONSMODEL_HPP

// Qt
#include <QAbstractItemModel>
#include <qhash.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

class QAction;

class ActionsModel : public QAbstractItemModel {
	Q_OBJECT

  public:
	enum Columns {

		COLUMN_ACTION = 0,
		COLUMN_SEQUENCE1,
		COLUMN_SEQUENCE2,
		COLUMN_COUNT
	};

	enum {
		HardKeyRole = Qt::UserRole,
		DefaultKeyRole,
	};

	explicit ActionsModel(QObject* parent = nullptr);
	[[nodiscard]] auto action(const QModelIndex& index) const -> QAction*;

  signals:
	void editError(const QString& error);

  protected:
	// Implement QAbstractItemModel
	[[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	auto setData(const QModelIndex& index, const QVariant& value, int role) -> bool override;
	[[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
	[[nodiscard]] auto index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const -> QModelIndex override;
	[[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;
	[[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;
	[[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

  private:
	QList<QAction*> m_actions;
};

#endif // ACTIONSMODEL_HPP
