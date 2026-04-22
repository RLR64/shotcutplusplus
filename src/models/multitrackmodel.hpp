/*
 * Copyright (c) 2013-2026 Meltytech, LLC
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

#ifndef MULTITRACKMODEL_HPP
#define MULTITRACKMODEL_HPP

// Qt
#include <MltPlaylist.h>
#include <MltTractor.h>
#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qtmetamacros.h>

// STL
#include <memory>

typedef enum { PlaylistTrackType = 0, BlackTrackType, SilentTrackType, AudioTrackType, VideoTrackType } TrackType;

typedef struct {
	TrackType type;
	int number;
	int mlt_index;
} Track;

typedef QList<Track> TrackList;

class MultitrackModel : public QAbstractItemModel {
	Q_OBJECT
	Q_PROPERTY(int trackHeight READ trackHeight WRITE setTrackHeight NOTIFY trackHeightChanged)
	Q_PROPERTY(
	    int trackHeaderWidth READ trackHeaderWidth WRITE setTrackHeaderWidth NOTIFY trackHeaderWidthChanged FINAL)
	Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged)
	Q_PROPERTY(bool filtered READ isFiltered NOTIFY filteredChanged)

  public:
	/// Two level model: tracks and clips on track
	enum {
		NameRole = Qt::UserRole + 1,
		CommentRole,  /// clip only
		ResourceRole, /// clip only
		ServiceRole,  /// clip only
		IsBlankRole,  /// clip only
		StartRole,    /// clip only
		DurationRole,
		InPointRole,   /// clip only
		OutPointRole,  /// clip only
		FramerateRole, /// clip only
		IsMuteRole,    /// track only
		IsHiddenRole,  /// track only
		IsAudioRole,
		AudioLevelsRole,  /// clip only
		IsCompositeRole,  /// track only
		IsLockedRole,     /// track only
		FadeInRole,       /// clip only
		FadeOutRole,      /// clip only
		IsTransitionRole, /// clip only
		FileHashRole,     /// clip only
		SpeedRole,        /// clip only
		IsFilteredRole,
		IsTopVideoRole,    /// track only
		IsBottomVideoRole, /// track only
		IsTopAudioRole,    /// track only
		IsBottomAudioRole, /// track only
		AudioIndexRole,    /// clip only
		GroupRole,         /// clip only
		GainRole,          /// clip only
		GainEnabledRole,   /// clip only
	};

	explicit MultitrackModel(QObject* parent = nullptr);
	~MultitrackModel() override;

	[[nodiscard]] auto tractor() const -> Mlt::Tractor* {
		return m_tractor;
	}

	[[nodiscard]] auto trackList() const -> const TrackList& {
		return m_trackList;
	}

	[[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
	[[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
	[[nodiscard]] auto data(const QModelIndex& index, int role) const -> QVariant override;
	[[nodiscard]] auto index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const -> QModelIndex override;
	[[nodiscard]] auto makeIndex(int trackIndex, int clipIndex) const -> QModelIndex;
	[[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;
	[[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;
	Q_INVOKABLE void audioLevelsReady(const QPersistentModelIndex& index);
	[[nodiscard]] auto getAudioLevels(int trackIndex, int clipIndex) const -> const QVariantList*;
	auto createIfNeeded() -> bool;
	void addBackgroundTrack();
	auto addAudioTrack() -> int;
	auto addVideoTrack() -> int;
	void removeTrack(int trackIndex);
	void load();
	void close();
	auto clipIndex(int trackIndex, int position) -> int;
	auto trimClipInValid(int trackIndex, int clipIndex, int delta, bool ripple) -> bool;
	auto trimClipOutValid(int trackIndex, int clipIndex, int delta, bool ripple) -> bool;
	[[nodiscard]] auto trackHeight() const -> int;
	void setTrackHeight(int height);
	[[nodiscard]] auto trackHeaderWidth() const -> int;
	void setTrackHeaderWidth(int width);
	[[nodiscard]] auto scaleFactor() const -> double;
	void setScaleFactor(double scale);
	auto isTransition(Mlt::Playlist& playlist, int clipIndex) const -> bool;
	void insertTrack(int trackIndex, TrackType type = VideoTrackType);
	void moveTrack(int fromTrackIndex, int toTrackIndex);
	void insertOrAdjustBlankAt(QList<int> tracks, int position, int length);
	auto mergeClipWithNext(int trackIndex, int clipIndex, bool dryrun) -> bool;
	auto findClipByUuid(const QUuid& uuid, int& trackIndex, int& clipIndex) -> std::unique_ptr<Mlt::ClipInfo>;
	auto getClipInfo(int trackIndex, int clipIndex) -> std::unique_ptr<Mlt::ClipInfo>;
	auto getTrackName(int trackIndex) -> QString;
	[[nodiscard]] auto bottomVideoTrackIndex() const -> int;
	[[nodiscard]] auto mltIndexForTrack(int trackIndex) const -> int;
	auto checkForEmptyTracks(int trackIndex) -> bool;
	auto trackTransitionService() -> QString;

  signals:
	void created();
	void aboutToClose();
	void closed();
	void modified();
	void seeked(int position, bool seekPlayer = true);
	void trackHeightChanged();
	void trackHeaderWidthChanged();
	void scaleFactorChanged();
	void showStatusMessage(QString);
	void durationChanged();
	void filteredChanged();
	void reloadRequested();
	void appended(int trackIndex, int clipIndex);
	void inserted(int trackIndex, int clipIndex);
	void overWritten(int trackIndex, int clipIndex);
	void removing(Mlt::Service* service);
	void noMoreEmptyTracks(bool isAudio);

  public slots:
	void refreshTrackList();
	void setTrackName(int row, const QString& value);
	void setTrackMute(int row, bool mute);
	void setTrackHidden(int row, bool hidden);
	void setTrackComposite(int row, bool composite);
	void setTrackLock(int row, bool lock);
	auto trimClipIn(int trackIndex, int clipIndex, int delta, bool ripple, bool rippleAllTracks) -> int;
	void notifyClipIn(int trackIndex, int clipIndex);
	auto trimClipOut(int trackIndex, int clipIndex, int delta, bool ripple, bool rippleAllTracks) -> int;
	void notifyClipOut(int trackIndex, int clipIndex);
	auto moveClip(int fromTrack, int toTrack, int clipIndex, int position, bool ripple, bool rippleAllTracks) -> bool;
	auto  overwriteClip(int trackIndex, Mlt::Producer& clip, int position, bool seek = true) -> int;
	auto overwrite(int trackIndex, Mlt::Producer& clip, int position, bool seek = true, bool notify = true) -> QString;
	auto insertClip(int trackIndex, Mlt::Producer& clip, int position, bool rippleAllTracks, bool seek = true,
				   bool notify = true) -> int;
	auto appendClip(int trackIndex, Mlt::Producer& clip, bool seek = true, bool notify = true) -> int;
	void removeClip(int trackIndex, int clipIndex, bool rippleAllTracks);
	void liftClip(int trackIndex, int clipIndex);
	void splitClip(int trackIndex, int clipIndex, int position);
	void joinClips(int trackIndex, int clipIndex);
	void changeGain(int trackIndex, int clipIndex, double gain);
	void fadeIn(int trackIndex, int clipIndex, int duration);
	void fadeOut(int trackIndex, int clipIndex, int duration);
	auto addTransitionValid(int fromTrack, int toTrack, int clipIndex, int position, bool ripple) -> bool;
	auto addTransition(int trackIndex, int clipIndex, int position, bool ripple, bool rippleAllTracks) -> int;
	void removeTransition(int trackIndex, int clipIndex);
	void removeTransitionByTrimIn(int trackIndex, int clipIndex, int delta);
	void removeTransitionByTrimOut(int trackIndex, int clipIndex, int delta);
	auto trimTransitionInValid(int trackIndex, int clipIndex, int delta) -> bool;
	void trimTransitionIn(int trackIndex, int clipIndex, int delta, bool slip = false);
	auto trimTransitionOutValid(int trackIndex, int clipIndex, int delta) -> bool;
	void trimTransitionOut(int trackIndex, int clipIndex, int delta, bool slip = false);
	auto addTransitionByTrimInValid(int trackIndex, int clipIndex, int delta) -> bool;
	auto addTransitionByTrimIn(int trackIndex, int clipIndex, int delta) -> int;
	auto addTransitionByTrimOutValid(int trackIndex, int clipIndex, int delta) -> bool;
	void addTransitionByTrimOut(int trackIndex, int clipIndex, int delta);
	auto removeTransitionByTrimInValid(int trackIndex, int clipIndex, int delta) -> bool;
	auto removeTransitionByTrimOutValid(int trackIndex, int clipIndex, int delta) -> bool;
	void filterAddedOrRemoved(Mlt::Producer* producer);
	void onFilterChanged(Mlt::Service* service);
	void reload(bool asynchronous = false);
	void replace(int trackIndex, int clipIndex, Mlt::Producer& clip, bool copyFilters = true);

  private:
	Mlt::Tractor* m_tractor;
	TrackList m_trackList;
	bool m_isMakingTransition;
	void moveClipToEnd(Mlt::Playlist& playlist, int trackIndex, int clipIndex, int position, bool ripple,
	                               bool rippleAllTracks);
	void moveClipInBlank(Mlt::Playlist& playlist, int trackIndex, int clipIndex, int position, bool ripple,
	                                 bool rippleAllTracks, int duration = 0);
	void consolidateBlanks(Mlt::Playlist& playlist, int trackIndex);
	void consolidateBlanksAllTracks();
	void getAudioLevels();
	void addBlackTrackIfNeeded();
	void convertOldDoc();
	[[nodiscard]] auto getTransition(const QString& name, int trackIndex) const -> Mlt::Transition*;
	[[nodiscard]] auto getFilter(const QString& name, int trackIndex) const -> Mlt::Filter*;
	[[nodiscard]] auto getFilter(const QString& name, Mlt::Service* service) const -> Mlt::Filter*;
	void removeBlankPlaceholder(Mlt::Playlist& playlist, int trackIndex);
	void retainPlaylist();
	void loadPlaylist();
	void removeRegion(int trackIndex, int position, int length);
	void clearMixReferences(int trackIndex, int clipIndex);
	auto isFiltered(Mlt::Producer* producer = nullptr) const -> bool;
	auto getDuration() -> int;
	void adjustServiceFilterDurations(Mlt::Service& service, int duration);
	auto warnIfInvalid(Mlt::Service& service) -> bool;
	[[nodiscard]] auto getVideoBlendTransition(int trackIndex) const -> Mlt::Transition*;
	void refreshVideoBlendTransitions();
	[[nodiscard]] auto bottomVideoTrackMltIndex() const -> int;
	[[nodiscard]] auto hasEmptyTrack(TrackType trackType) const -> bool;

	friend class UndoHelper;

  private slots:
	void adjustBackgroundDuration();
	void adjustTrackFilters();
};

#endif // MULTITRACKMODEL_HPP
