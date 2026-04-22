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

// Local
#include "extensionmodel.hpp"
#include "Logger.hpp"
#include "qmltypes/qmlextension.hpp"
#include "settings.hpp"

// Qt
#include <QIcon>
#include <qabstractitemmodel.h>
#include <qdir.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtpreprocessorsupport.h>
#include <qtypes.h>

ExtensionModel::ExtensionModel(QObject* parent) : QAbstractItemModel(parent), m_ext(nullptr) {
}

ExtensionModel::~ExtensionModel() {
	delete m_ext;
}

void ExtensionModel::load(const QString& id) {
	beginResetModel();
	delete m_ext;
	m_ext = QmlExtension::load(id);
	if (!m_ext) {
		LOG_ERROR() << "Extension not loaded:" << id;
	}
	endResetModel();
}

auto ExtensionModel::count() -> int {
	if (m_ext) {
		return m_ext->fileCount();
	}
	return 0;
}

auto ExtensionModel::getName(int row) const -> QString {
	if (!m_ext)
		return QString();
	return m_ext->file(row)->name();
}

auto ExtensionModel::getFormattedDataSize(int row) const -> QString {
	if (!m_ext)
		return nullptr;
	qint64 const bytes = m_ext->file(row)->size().toLongLong();
	return QLocale().formattedDataSize(bytes);
}

auto ExtensionModel::localPath(int row) const -> QString{
	if (!m_ext)
		return QString();
	return m_ext->localPath(row);
}

auto ExtensionModel::url(int row) const -> QString{
	if (!m_ext)
		return QString();
	return m_ext->file(row)->url();
}

auto ExtensionModel::downloaded(int row) const -> bool {
	if (!m_ext)
		return false;
	return m_ext->downloaded(row);
}

void ExtensionModel::deleteFile(int row) {
	QFile file(localPath(row));
	file.remove();
	QModelIndex const modelIndex = index(row, COLUMN_STATUS);
	dataChanged(modelIndex, modelIndex);
}

auto ExtensionModel::getStandardIndex() const -> int {
	for (int row = 0; row < m_ext->fileCount(); row++) {
		if (m_ext->file(row)->standard()) {
			return row;
		}
	}
	return 0;
}

auto ExtensionModel::getIndexForPath(const QString& path) -> QModelIndex {
	for (int row = 0; row < m_ext->fileCount(); row++) {
		if (path == localPath(row)) {
			return index(row, 0);
		}
	}
	return QModelIndex();
}

auto ExtensionModel::rowCount(const QModelIndex& parent) const -> int {
	Q_UNUSED(parent)
	if (!m_ext) {
		LOG_ERROR() << "Extensions not loaded";
		return 0;
	}
	return m_ext->fileCount();
}

auto ExtensionModel::columnCount(const QModelIndex& parent) const -> int {
	Q_UNUSED(parent)
	return COLUMN_COUNT;
}

auto ExtensionModel::data(const QModelIndex& index, int role) const -> QVariant {
	QVariant result;

	switch (role) {
	case Qt::StatusTipRole:
	case Qt::FontRole:
	case Qt::SizeHintRole:
	case Qt::CheckStateRole:
	case Qt::BackgroundRole:
	case Qt::ForegroundRole:
		return result;
	}

	if (!m_ext) {
		LOG_ERROR() << "Extensions not loaded";
		return result;
	}

	if (!index.isValid() || index.column() < 0 || index.column() >= COLUMN_COUNT || index.row() < 0 ||
	    index.row() >= m_ext->fileCount()) {
		LOG_ERROR() << "Invalid Index: " << index.row() << index.column() << role;
		return result;
	}

	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case COLUMN_STATUS:
			break;
		case COLUMN_NAME:
			result = getName(index.row());
			break;
		case COLUMN_SIZE:
			result = getFormattedDataSize(index.row());
			break;
		default:
			LOG_ERROR() << "Invalid Column" << index.row() << index.column() << roleNames()[role] << role;
			break;
		}
		break;
	case Qt::DecorationRole:
		switch (index.column()) {
		case COLUMN_STATUS:
			if (downloaded(index.row())) {
				result = QIcon(":/icons/oxygen/32x32/status/task-complete.png");
			} else {
				result = QIcon::fromTheme("download", QIcon(":/icons/oxygen/32x32/actions/download.png"));
			}
			break;
		case COLUMN_NAME:
		case COLUMN_SIZE:
			break;
		default:
			LOG_ERROR() << "Invalid DecorationRole Column" << index.row() << index.column() << roleNames()[role]
			            << role;
			break;
		}
		break;
	case Qt::ToolTipRole:
		return m_ext->file(index.row())->description();
	case Qt::TextAlignmentRole:
		switch (index.column()) {
		case COLUMN_NAME:
			result = Qt::AlignLeft;
			break;
		case COLUMN_SIZE:
			result = Qt::AlignRight;
			break;
		case COLUMN_STATUS:
			result = Qt::AlignCenter;
			break;
		default:
			LOG_ERROR() << "Invalid Column" << index.row() << index.column() << roleNames()[role] << role;
			break;
		}
		break;
	default:
		LOG_ERROR() << "Invalid Role" << index.row() << index.column() << roleNames()[role] << role;
		break;
	}
	return result;
}

auto ExtensionModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch (section) {
		case COLUMN_STATUS:
			return QVariant();
		case COLUMN_NAME:
			return tr("Name");
		case COLUMN_SIZE:
			return tr("Size");
		default:
			break;
		}
	}
	return QVariant();
}

auto ExtensionModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex {
	Q_UNUSED(parent)
	if (!m_ext) {
		LOG_ERROR() << "Extensions not loaded";
		return QModelIndex();
	}
	if (column < 0 || column >= COLUMN_COUNT || row < 0 || row >= m_ext->fileCount())
		return QModelIndex();
	return createIndex(row, column, (int)0);
}

auto ExtensionModel::parent(const QModelIndex& index) const -> QModelIndex {
	Q_UNUSED(index)
	return QModelIndex();
}
