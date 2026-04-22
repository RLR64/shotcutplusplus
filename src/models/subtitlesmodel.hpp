/*
 * Copyright (c) 2024 Meltytech, LLC
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

#ifndef SUBTITLESMODEL_HPP
#define SUBTITLESMODEL_HPP

// Local
#include "models/subtitles.hpp"

// Qt
#include <MltProducer.h>
#include <QAbstractItemModel>
#include <QString>
#include <QTimer>
#include <qhash.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

// STL
#include <cstdint>

class SubtitlesModel : public QAbstractItemModel {
	Q_OBJECT
	Q_PROPERTY(int trackCount READ trackCount NOTIFY tracksChanged)

  public:
	enum Roles {
		TextRole = Qt::UserRole + 1,
		StartRole,
		EndRole,
		DurationRole,
		SimpleText,
		StartFrameRole,
		EndFrameRole,
		SiblingCountRole,
	};

	struct SubtitleTrack {
		QString name;
		QString lang;
	};

	explicit SubtitlesModel(QObject* parent = nullptr);
	~SubtitlesModel() override;

	void    load(Mlt::Producer* producer);
	[[nodiscard]] bool    isValid() const;
	[[nodiscard]] int64_t maxTime() const;

		   // Track Functions
	[[nodiscard]] int                                  trackCount() const;
	[[nodiscard]] Q_INVOKABLE QModelIndex              trackModelIndex(int trackIndex) const;
	[[nodiscard]] QList<SubtitlesModel::SubtitleTrack> getTracks() const;
	int                                  getTrackIndex(const QString& name);
	SubtitlesModel::SubtitleTrack        getTrack(const QString& name);
	SubtitlesModel::SubtitleTrack        getTrack(int index);
	void                                 addTrack(SubtitlesModel::SubtitleTrack& track);
	void                                 removeTrack(QString& name);
	void                                 editTrack(int trackIndex, SubtitlesModel::SubtitleTrack& track);

		   // Item Functions
	[[nodiscard]] Q_INVOKABLE int                itemCount(int trackIndex) const;
	[[nodiscard]] int64_t                        endTime(int trackIndex) const;
	[[nodiscard]] QModelIndex                    itemModelIndex(int trackIndex, int itemIndex) const;
	[[nodiscard]] int                            itemIndexAtTime(int trackIndex, int64_t msTime) const;
	[[nodiscard]] int                            itemIndexBeforeTime(int trackIndex, int64_t msTime) const;
	[[nodiscard]] int                            itemIndexAfterTime(int trackIndex, int64_t msTime) const;
	[[nodiscard]] const Subtitles::SubtitleItem& getItem(int trackIndex, int itemIndex) const;
	void importSubtitles(int trackIndex, int64_t msTime, QList<Subtitles::SubtitleItem>& items);
	void importSubtitlesToNewTrack(SubtitlesModel::SubtitleTrack& track, QList<Subtitles::SubtitleItem>& items);
	void exportSubtitles(const QString& filePath, int trackIndex) const;
	void overwriteItem(int trackIndex, const Subtitles::SubtitleItem& item);
	void appendItem(int trackIndex, const Subtitles::SubtitleItem& item);
	void removeItems(int trackIndex, int firstItemIndex, int lastItemIndex);
	void setItemStart(int trackIndex, int itemIndex, int64_t msTime);
	void setItemEnd(int trackIndex, int itemIndex, int64_t msTime);
	void setText(int trackIndex, int itemIndex, const QString& text);
	Q_INVOKABLE void moveItems(int trackIndex, int firstItemIndex, int lastItemIndex, int64_t msTime);
	Q_INVOKABLE bool validateMove(const QModelIndexList& items, int64_t msTime);

		   // Only to be called by subtitle commands
	void doInsertTrack(const SubtitlesModel::SubtitleTrack& track, int trackIndex);
	void doRemoveTrack(int trackIndex);
	void doEditTrack(const SubtitlesModel::SubtitleTrack& track, int trackIndex);
	void doRemoveSubtitleItems(int trackIndex, const QList<Subtitles::SubtitleItem>& subtitles);
	void doInsertSubtitleItems(int trackIndex, const QList<Subtitles::SubtitleItem>& subtitles);
	void doSetText(int trackIndex, int itemIndex, const QString& text);
	void doSetTime(int trackIndex, int itemIndex, int64_t startTime, int64_t endTime);

  signals:
	void tracksChanged(int count);
	void modified();

  protected:
	// Implement QAbstractItemModel
	[[nodiscard]] int                    rowCount(const QModelIndex& parent) const override;
	[[nodiscard]] int                    columnCount(const QModelIndex& parent) const override;
	[[nodiscard]] QVariant               data(const QModelIndex& index, int role) const override;
	[[nodiscard]] QVariant               headerData(int section, Qt::Orientation orientation, int role) const override;
	[[nodiscard]] QModelIndex            index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const override;
	[[nodiscard]] QModelIndex            parent(const QModelIndex& index) const override;
	[[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  private:
	void                                  requestFeedCommit(int trackIndex);
	void                                  commitToFeed(int trackIndex);
	Mlt::Producer*                        m_producer;
	QList<SubtitlesModel::SubtitleTrack>  m_tracks;
	QList<QList<Subtitles::SubtitleItem>> m_items;
	QTimer*                               m_commitTimer;
	int                                   m_commitTrack;
};

#endif // SUBTITLESMODEL_HPP