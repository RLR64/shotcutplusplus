/*
 * Copyright (c) 2013-2025 Meltytech, LLC
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

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "docks/timelinedock.hpp"
#include "models/markersmodel.hpp"
#include "models/multitrackmodel.hpp"
#include "undohelper.hpp"

#include <MltProducer.h>
#include <MltTransition.h>
#include <QObject>
#include <QString>
#include <QUndoCommand>
#include <QUuid>
#include <vector>

namespace Timeline {

enum {
	UndoIdTrimClipIn = 100,
	UndoIdTrimClipOut,
	UndoIdFadeIn,
	UndoIdFadeOut,
	UndoIdTrimTransitionIn,
	UndoIdTrimTransitionOut,
	UndoIdAddTransitionByTrimIn,
	UndoIdAddTransitionByTrimOut,
	UndoIdUpdate,
	UndoIdMoveClip,
	UndoIdChangeGain,
};

struct ClipPosition {
	ClipPosition(int track, int clip) : trackIndex(track), clipIndex(clip) {
	}

	bool operator<(const ClipPosition& rhs) const {
		if (trackIndex == rhs.trackIndex) {
			return clipIndex < rhs.clipIndex;
		}
		return trackIndex < rhs.trackIndex;
	}

	int trackIndex;
	int clipIndex;
};

class AppendCommand : public QUndoCommand {
  public:
	AppendCommand(MultitrackModel& Model, int TrackIndex, const QString& Xml, bool SkipProxy = false, bool Seek = true,
	              QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	QString          m_xml;
	UndoHelper       m_undoHelper;
	bool             m_skipProxy;
	bool             m_seek;
	QVector<QUuid>   m_uuids;
};

class InsertCommand : public QUndoCommand {
  public:
	InsertCommand(MultitrackModel& Model, MarkersModel& MarkersModel, int TrackIndex, int Position, const QString& Xml,
	              bool Seek = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	MarkersModel&    m_markersModel;
	int              m_trackIndex;
	int              m_position;
	QString          m_xml;
	QStringList      m_oldTracks;
	UndoHelper       m_undoHelper;
	bool             m_seek;
	bool             m_rippleAllTracks;
	bool             m_rippleMarkers;
	int              m_markersShift;
	QVector<QUuid>   m_uuids;
};

class OverwriteCommand : public QUndoCommand {
  public:
	OverwriteCommand(MultitrackModel& Model, int TrackIndex, int Position, const QString& Xml, bool Seek = true,
	                 QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_position;
	QString          m_xml;
	UndoHelper       m_undoHelper;
	bool             m_seek;
	QVector<QUuid>   m_uuids;
};

class LiftCommand : public QUndoCommand {
  public:
	LiftCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	UndoHelper       m_undoHelper;
};

class RemoveCommand : public QUndoCommand {
  public:
	RemoveCommand(MultitrackModel& Model, MarkersModel& MarkersModel, int TrackIndex, int ClipIndex,
	              QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel&       m_model;
	MarkersModel&          m_markersModel;
	int                    m_trackIndex;
	int                    m_clipIndex;
	UndoHelper             m_undoHelper;
	bool                   m_rippleAllTracks;
	bool                   m_rippleMarkers;
	int                    m_markerRemoveStart;
	int                    m_markerRemoveEnd;
	QList<Markers::Marker> m_markers;
};

class GroupCommand : public QUndoCommand {
  public:
	GroupCommand(MultitrackModel& Model, QUndoCommand* Parent = nullptr);
	void AddToGroup(int TrackIndex, int ClipIndex);
	void Redo();
	void Undo();

  private:
	MultitrackModel&        m_model;
	QList<ClipPosition>     m_clips;
	QMap<ClipPosition, int> m_prevGroups;
};

class UngroupCommand : public QUndoCommand {
  public:
	UngroupCommand(MultitrackModel& Model, QUndoCommand* Parent = nullptr);
	void RemoveFromGroup(int TrackIndex, int ClipIndex);
	void Redo();
	void Undo();

  private:
	MultitrackModel&        m_model;
	QMap<ClipPosition, int> m_prevGroups;
};

class NameTrackCommand : public QUndoCommand {
  public:
	NameTrackCommand(MultitrackModel& Model, int TrackIndex, const QString& Name, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	QString          m_name;
	QString          m_oldName;
};

class MergeCommand : public QUndoCommand {
  public:
	MergeCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	UndoHelper       m_undoHelper;
};

class MuteTrackCommand : public QUndoCommand {
  public:
	MuteTrackCommand(MultitrackModel& Model, int TrackIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	bool             m_oldValue;
};

class HideTrackCommand : public QUndoCommand {
  public:
	HideTrackCommand(MultitrackModel& Model, int TrackIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	bool             m_oldValue;
};

class CompositeTrackCommand : public QUndoCommand {
  public:
	CompositeTrackCommand(MultitrackModel& Model, int TrackIndex, bool Value, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	bool             m_value;
	bool             m_oldValue;
};

class LockTrackCommand : public QUndoCommand {
  public:
	LockTrackCommand(MultitrackModel& Model, int TrackIndex, bool Value, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	bool             m_value;
	bool             m_oldValue;
};

class MoveClipCommand : public QUndoCommand {
  public:
	MoveClipCommand(TimelineDock& Timeline, int TrackDelta, int PositionDelta, bool Ripple,
	                QUndoCommand* Parent = nullptr);
	void AddClip(int TrackIndex, int ClipIndex);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdMoveClip;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	void RedoMarkers();

	TimelineDock&    m_timeline;
	MultitrackModel& m_model;
	MarkersModel&    m_markersModel;

	struct Info {
		int   trackIndex{-1};
		int   clipIndex{-1};
		int   frame_in{-1};
		int   frame_out{-1};
		int   start{0};
		int   group{-1};
		QUuid m_uuid{};

		Info()

		{
		}
	};

	int                    m_trackDelta;
	int                    m_positionDelta;
	bool                   m_ripple;
	bool                   m_rippleAllTracks;
	bool                   m_rippleMarkers;
	UndoHelper             m_undoHelper;
	QMultiMap<int, Info>   m_clips; // ordered by position
	bool                   m_redo;
	int                    m_earliestStart;
	QList<Markers::Marker> m_markers;
};

class TrimCommand : public QUndoCommand {
  public:
	explicit TrimCommand(QUndoCommand* Parent = nullptr) : QUndoCommand(Parent) {
	}

	void SetUndoHelper(UndoHelper* Helper) {
		m_undoHelper.reset(Helper);
	}

  protected:
	QScopedPointer<UndoHelper> m_undoHelper{};
};

class TrimClipInCommand : public TrimCommand {
  public:
	TrimClipInCommand(MultitrackModel& Model, MarkersModel& MarkersModel, int TrackIndex, int ClipIndex, int Delta,
	                  bool Ripple, bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimClipIn;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel&       m_model;
	MarkersModel&          m_markersModel;
	int                    m_trackIndex;
	int                    m_clipIndex;
	int                    m_delta;
	bool                   m_ripple;
	bool                   m_rippleAllTracks;
	bool                   m_rippleMarkers;
	bool                   m_redo;
	int                    m_markerRemoveStart;
	int                    m_markerRemoveEnd;
	QList<Markers::Marker> m_markers;
};

class TrimClipOutCommand : public TrimCommand {
  public:
	TrimClipOutCommand(MultitrackModel& Model, MarkersModel& MarkersModel, int TrackIndex, int ClipIndex, int Delta,
	                   bool Ripple, bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimClipOut;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel&       m_model;
	MarkersModel&          m_markersModel;
	int                    m_trackIndex;
	int                    m_clipIndex;
	int                    m_delta;
	bool                   m_ripple;
	bool                   m_rippleAllTracks;
	bool                   m_rippleMarkers;
	bool                   m_redo;
	int                    m_markerRemoveStart;
	int                    m_markerRemoveEnd;
	QList<Markers::Marker> m_markers;
};

class SplitCommand : public QUndoCommand {
  public:
	SplitCommand(MultitrackModel& Model, const std::vector<int>& TrackIndex, const std::vector<int>& ClipIndex,
	             int Position, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	std::vector<int> m_trackIndex;
	std::vector<int> m_clipIndex;
	int              m_position;
	UndoHelper       m_undoHelper;
};

class FadeInCommand : public QUndoCommand {
  public:
	FadeInCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Duration, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdFadeIn;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_duration;
	int              m_previous;
};

class FadeOutCommand : public QUndoCommand {
  public:
	FadeOutCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Duration, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdFadeOut;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_duration;
	int              m_previous;
};

class AddTransitionCommand : public QUndoCommand {
  public:
	AddTransitionCommand(TimelineDock& Timeline, int TrackIndex, int ClipIndex, int Position, bool Ripple,
	                     QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

	int GetTransitionIndex() const {
		return m_transitionIndex;
	}

  private:
	TimelineDock&          m_timeline;
	MultitrackModel&       m_model;
	MarkersModel&          m_markersModel;
	int                    m_trackIndex;
	int                    m_clipIndex;
	int                    m_position;
	int                    m_transitionIndex;
	bool                   m_ripple;
	UndoHelper             m_undoHelper;
	bool                   m_rippleAllTracks;
	bool                   m_rippleMarkers;
	int                    m_markerOldStart;
	int                    m_markerNewStart;
	QList<Markers::Marker> m_markers;
};

class TrimTransitionInCommand : public TrimCommand {
  public:
	TrimTransitionInCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Delta, bool Redo = true,
	                        QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimTransitionIn;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_delta;
	bool             m_notify;
	bool             m_redo;
};

class TrimTransitionOutCommand : public TrimCommand {
  public:
	TrimTransitionOutCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Delta, bool Redo = true,
	                         QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimTransitionOut;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_delta;
	bool             m_notify;
	bool             m_redo;
};

class AddTransitionByTrimInCommand : public TrimCommand {
  public:
	AddTransitionByTrimInCommand(TimelineDock& Timeline, int TrackIndex, int ClipIndex, int Duration, int TrimDelta,
	                             bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdAddTransitionByTrimIn;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	TimelineDock& m_timeline;
	int           m_trackIndex;
	int           m_clipIndex;
	int           m_duration;
	int           m_trimDelta;
	bool          m_notify;
	bool          m_redo;
};

class RemoveTransitionByTrimInCommand : public TrimCommand {
  public:
	RemoveTransitionByTrimInCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Delta, QString Xml,
	                                bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_delta;
	QString          m_xml;
	bool             m_redo;
};

class RemoveTransitionByTrimOutCommand : public TrimCommand {
  public:
	RemoveTransitionByTrimOutCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Delta, QString Xml,
	                                 bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_delta;
	QString          m_xml;
	bool             m_redo;
};

class AddTransitionByTrimOutCommand : public TrimCommand {
  public:
	AddTransitionByTrimOutCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, int Duration, int TrimDelta,
	                              bool Redo = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdAddTransitionByTrimOut;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	int              m_duration;
	int              m_trimDelta;
	bool             m_notify;
	bool             m_redo;
};

class AddTrackCommand : public QUndoCommand {
  public:
	AddTrackCommand(MultitrackModel& Model, bool IsVideo, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	bool             m_isVideo;
	QUuid            m_uuid;
};

class InsertTrackCommand : public QUndoCommand {
  public:
	InsertTrackCommand(MultitrackModel& Model, int TrackIndex, TrackType TrackType = PlaylistTrackType,
	                   QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	TrackType        m_trackType;
	QUuid            m_uuid;
};

class RemoveTrackCommand : public QUndoCommand {
  public:
	RemoveTrackCommand(MultitrackModel& Model, int TrackIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel&              m_model;
	int                           m_trackIndex;
	TrackType                     m_trackType;
	QString                       m_trackName;
	UndoHelper                    m_undoHelper;
	QScopedPointer<Mlt::Producer> m_filtersProducer;
	QUuid                         m_uuid;
};

class MoveTrackCommand : public QUndoCommand {
  public:
	MoveTrackCommand(MultitrackModel& Model, int FromTrackIndex, int ToTrackIndex, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_fromTrackIndex;
	int              m_toTrackIndex;
};

class ChangeBlendModeCommand : public QObject, public QUndoCommand {
	Q_OBJECT
  public:
	ChangeBlendModeCommand(Mlt::Transition& transition, const QString& propertyName, const QString& mode,
	                       QUndoCommand* parent = 0);
	void Redo();
	void Undo();
  signals:
	void modeChanged(QString& mode);

  private:
	Mlt::Transition m_transition{};
	QString         m_propertyName{};
	QString         m_newMode{};
	QString         m_oldMode{};
};

class UpdateCommand : public QUndoCommand {
  public:
	UpdateCommand(TimelineDock& Timeline, int TrackIndex, int ClipIndex, int Position, QUndoCommand* Parent = nullptr);
	void SetXmlAfter(const QString& Xml);
	void SetPosition(int TrackIndex, int ClipIndex, int Position);
	void SetRippleAllTracks(bool);

	int TrackIndex() const {
		return m_trackIndex;
	}

	int ClipIndex() const {
		return m_clipIndex;
	}

	int Position() const {
		return m_position;
	}

	void Redo();
	void Undo();

  private:
	TimelineDock& m_timeline;
	int           m_trackIndex;
	int           m_clipIndex;
	int           m_position;
	QString       m_xmlAfter;
	bool          m_isFirstRedo;
	UndoHelper    m_undoHelper;
	bool          m_ripple;
	bool          m_rippleAllTracks;
};

class DetachAudioCommand : public QUndoCommand {
  public:
	DetachAudioCommand(TimelineDock& Timeline, int TrackIndex, int ClipIndex, int Position, const QString& Xml,
	                   QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	TimelineDock& m_timeline;
	int           m_trackIndex;
	int           m_clipIndex;
	int           m_position;
	int           m_targetTrackIndex;
	QString       m_xml;
	UndoHelper    m_undoHelper;
	bool          m_trackAdded;
	QUuid         m_uuid;
};

class ReplaceCommand : public QUndoCommand {
  public:
	ReplaceCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, const QString& Xml,
	               QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	QString          m_xml;
	bool             m_isFirstRedo;
	UndoHelper       m_undoHelper;
};

class AlignClipsCommand : public QUndoCommand {
  public:
	AlignClipsCommand(MultitrackModel& Model, QUndoCommand* Parent = nullptr);
	void AddAlignment(QUuid Uuid, int Offset, double SpeedCompensation);
	void Redo();
	void Undo();

  private:
	MultitrackModel& m_model;
	UndoHelper       m_undoHelper;
	bool             m_redo;

	struct Alignment {
		QUuid  m_uuid{};
		int    m_offset{};
		double m_speed{};
	};

	QVector<Alignment> m_alignments;
};

class ApplyFiltersCommand : public QUndoCommand {
  public:
	ApplyFiltersCommand(MultitrackModel& Model, const QString& FilterProducerXml, QUndoCommand* Parent = nullptr);
	void AddClip(int TrackIndex, int ClipIndex);
	void Redo();
	void Undo();

  private:
	MultitrackModel&            m_model;
	QString                     m_xml;
	QMap<ClipPosition, QString> m_prevFilters;
};

class ChangeGainCommand : public QUndoCommand {
  public:
	ChangeGainCommand(MultitrackModel& Model, int TrackIndex, int ClipIndex, double Gain,
	                  QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdChangeGain;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	MultitrackModel& m_model;
	int              m_trackIndex;
	int              m_clipIndex;
	double           m_gain;
	double           m_previous;
};

} // namespace Timeline

#endif // COMMANDS_HPP
