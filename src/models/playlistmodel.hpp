/*
 * Copyright (c) 2012-2024 Meltytech, LLC
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

#ifndef PLAYLISTMODEL_HPP
#define PLAYLISTMODEL_HPP

// Qt
#include <MltPlaylist.h>
#include <QAbstractTableModel>
#include <QMimeData>
#include <QStringList>
#include <qcontainerfwd.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

class PlaylistModel : public QAbstractTableModel {
	Q_OBJECT
  public:
	enum ViewMode {
		Invalid,
		Detailed,
		Tiled,
		Icons,
	};

	enum MediaType { Video, Image, Audio, Other, Pending };

	enum Columns {
		COLUMN_INDEX = 0,
		COLUMN_THUMBNAIL,
		COLUMN_RESOURCE,
		COLUMN_IN,
		COLUMN_DURATION,
		COLUMN_START,
		COLUMN_DATE,
		COLUMN_MEDIA_TYPE,
		COLUMN_COMMENT,
		COLUMN_BIN,
		COLUMN_COUNT
	};

	enum Fields {
		FIELD_INDEX = Qt::UserRole,
		FIELD_THUMBNAIL,
		FIELD_RESOURCE,
		FIELD_IN,
		FIELD_DURATION,
		FIELD_START,
		FIELD_DATE,
		FIELD_MEDIA_TYPE,
		FIELD_MEDIA_TYPE_ENUM,
		FIELD_COMMENT,
		FIELD_BIN
	};

	static constexpr int THUMBNAIL_WIDTH{80};
	static constexpr int THUMBNAIL_HEIGHT{45};

	explicit PlaylistModel(QObject* parent = nullptr);
	~PlaylistModel() override;
	[[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
	[[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
	[[nodiscard]] auto supportedDropActions() const -> Qt::DropActions override;
	auto insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) -> bool override;
	auto removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) -> bool override;
	auto moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
				  int destinationChild) -> bool override;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
	[[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;
	[[nodiscard]] auto mimeTypes() const -> QStringList override;
	[[nodiscard]] auto mimeData(const QModelIndexList& indexes) const -> QMimeData* override;
	[[nodiscard]] auto dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) -> bool override;
	[[nodiscard]] auto createIndex(int row, int column) const -> QModelIndex;
	void createIfNeeded();
	void showThumbnail(int row);
	void refreshThumbnails();

	auto playlist() -> Mlt::Playlist* {
		return m_playlist;
	}

	void setPlaylist(Mlt::Playlist& playlist);
	void setInOut(int row, int in, int out);

	[[nodiscard]] auto viewMode() const -> ViewMode;
	void setViewMode(ViewMode mode);
	void setBin(int row, const QString& name);
	void renameBin(const QString& bin, const QString& newName = QString());

  signals:
	void created();
	void cleared();
	void closed();
	void modified();
	void loaded();
	void dropped(const QMimeData* data, int row);
	void moveClip(int from, int to);
	void inChanged(int in);
	void outChanged(int out);
	void removing(Mlt::Service* service);

  public slots:
	void clear();
	void load();
	void append(Mlt::Producer&, bool emitModified = true);
	void insert(Mlt::Producer&, int row);
	void remove(int row);
	void update(int row, Mlt::Producer& producer, bool copyFilters = false);
	void updateThumbnails(int row);
	void appendBlank(int frames);
	void insertBlank(int frames, int row);
	void close();
	void move(int from, int to);

  private:
	Mlt::Playlist* m_playlist;
	int m_dropRow;
	ViewMode m_mode;
	QList<int> m_rowsRemoved;

  private slots:
	void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
};

#endif // PLAYLISTMODEL_HPP
