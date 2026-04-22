/*
 * Copyright (c) 2025 Meltytech, LLC
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

#ifndef EXTENSIONMODEL_HPP
#define EXTENSIONMODEL_HPP

// Local
#include "qmltypes/qmlextension.hpp"

// Qt
#include <QAbstractItemModel>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

class ExtensionModel : public QAbstractItemModel {
	Q_OBJECT

  public:
	enum Columns {
		COLUMN_STATUS = 0,
		COLUMN_NAME,
		COLUMN_SIZE,
		COLUMN_COUNT,
	};

	explicit ExtensionModel(QObject* parent = nullptr);
	~ExtensionModel() override;
	void load(const QString& id);
	auto count() -> int;
	[[nodiscard]] auto getName(int row) const -> QString;
	[[nodiscard]] auto getFormattedDataSize(int row) const -> QString;
	[[nodiscard]] auto localPath(int row) const -> QString;
	[[nodiscard]] auto url(int row) const -> QString;
	[[nodiscard]] auto downloaded(int row) const -> bool;
	void deleteFile(int row);
	[[nodiscard]] auto getStandardIndex() const -> int;
	auto getIndexForPath(const QString& path) -> QModelIndex;

  protected:
	// Implement QAbstractItemModel
	[[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	[[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
	[[nodiscard]] auto index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const -> QModelIndex override;
	[[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

  private:
	QmlExtension* m_ext;
};

#endif // EXTENSIONMODEL_HPP
