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

#ifndef ALIGNCLIPSMODEL_HPP
#define ALIGNCLIPSMODEL_HPP

// Qt
#include <QAbstractItemModel>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

// STL
#include <limits>

static constexpr int INVALID_OFFSET = std::numeric_limits<int>::max();


class AlignClipsModel : public QAbstractItemModel {
	Q_OBJECT

  public:
	enum Columns {
		COLUMN_ERROR = 0,
		COLUMN_NAME,
		COLUMN_OFFSET,
		COLUMN_SPEED,
		COLUMN_COUNT,
	};

	explicit AlignClipsModel(QObject* parent = nullptr);
	~AlignClipsModel() override = default;
	void clear();
	void addClip(const QString& name, int offset, int speed, const QString& error);
	void updateProgress(int row, int percent);
	[[nodiscard]] auto getProgress(int row) const -> int;
	void updateOffsetAndSpeed(int row, int offset, double speed, const QString& error);
	auto getOffset(int row) -> int;
	auto getSpeed(int row) -> double;

  protected:
	// Implement QAbstractItemModel
	[[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	[[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
	[[nodiscard]] auto index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const -> QModelIndex override;
	[[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

  private:
	typedef struct {
		QString name;
		int offset;
		double speed;
		QString error;
		int progress;
	} ClipAlignment;

	QList<ClipAlignment> m_clips;
};

#endif // ALIGNCLIPSMODEL_HPP
