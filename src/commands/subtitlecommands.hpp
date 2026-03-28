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

#ifndef SUBTITLECOMMANDS_HPP
#define SUBTITLECOMMANDS_HPP

#include "models/subtitlesmodel.hpp"

#include <QUndoCommand>

namespace Subtitles {

enum {
	UndoIdSubText = 400,
	UndoIdSubStart,
	UndoIdSubEnd,
	UndoIdSubMove,
};

class InsertTrackCommand : public QUndoCommand {
  public:
	InsertTrackCommand(SubtitlesModel& Model, const SubtitlesModel::SubtitleTrack& Track, int Index);
	void Redo();
	void Undo();

  private:
	SubtitlesModel&               m_model;
	SubtitlesModel::SubtitleTrack m_track;
	int                           m_index;
};

class RemoveTrackCommand : public QUndoCommand {
  public:
	RemoveTrackCommand(SubtitlesModel& Model, int TrackIndex);
	void Redo();
	void Undo();

  private:
	SubtitlesModel&                m_model;
	int                            m_trackIndex;
	SubtitlesModel::SubtitleTrack  m_saveTrack;
	QList<Subtitles::SubtitleItem> m_saveSubtitles;
};

class EditTrackCommand : public QUndoCommand {
  public:
	EditTrackCommand(SubtitlesModel& Model, const SubtitlesModel::SubtitleTrack& Track, int Index);
	void Redo();
	void Undo();

  private:
	SubtitlesModel&               m_model;
	SubtitlesModel::SubtitleTrack m_oldTrack;
	SubtitlesModel::SubtitleTrack m_newTrack;
	int                           m_index;
};

class OverwriteSubtitlesCommand : public QUndoCommand {
  public:
	OverwriteSubtitlesCommand(SubtitlesModel& Model, int TrackIndex, const QList<Subtitles::SubtitleItem>& Items);
	void Redo();
	void Undo();

  protected:
	QList<Subtitles::SubtitleItem> m_newSubtitles;

  private:
	SubtitlesModel&                m_model;
	int                            m_trackIndex;
	QList<Subtitles::SubtitleItem> m_saveSubtitles;
};

class RemoveSubtitlesCommand : public QUndoCommand {
  public:
	RemoveSubtitlesCommand(SubtitlesModel& Model, int TrackIndex, const QList<Subtitles::SubtitleItem>& Items);
	void Redo();
	void Undo();

  private:
	SubtitlesModel&                m_model;
	int                            m_trackIndex;
	QList<Subtitles::SubtitleItem> m_items;
};

class SetTextCommand : public QUndoCommand {
  public:
	SetTextCommand(SubtitlesModel& Model, int TrackIndex, int ItemIndex, const QString& Text);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdSubText;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	SubtitlesModel& m_model;
	int             m_trackIndex;
	int             m_itemIndex;
	QString         m_newText;
	QString         m_oldText;
};

class SetStartCommand : public QUndoCommand {
  public:
	SetStartCommand(SubtitlesModel& Model, int TrackIndex, int ItemIndex, int64_t MsTime);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdSubStart;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	SubtitlesModel& m_model;
	int             m_trackIndex;
	int             m_itemIndex;
	int64_t         m_newStart;
	int64_t         m_oldStart;
};

class SetEndCommand : public QUndoCommand {
  public:
	SetEndCommand(SubtitlesModel& Model, int TrackIndex, int ItemIndex, int64_t MsTime);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdSubEnd;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	SubtitlesModel& m_model;
	int             m_trackIndex;
	int             m_itemIndex;
	int64_t         m_newEnd;
	int64_t         m_oldEnd;
};

class MoveSubtitlesCommand : public QUndoCommand {
  public:
	MoveSubtitlesCommand(SubtitlesModel& Model, int TrackIndex, const QList<Subtitles::SubtitleItem>& Items,
	                     int64_t MsTime);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdSubMove;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	SubtitlesModel&                m_model;
	int                            m_trackIndex;
	QList<Subtitles::SubtitleItem> m_oldSubtitles;
	QList<Subtitles::SubtitleItem> m_newSubtitles;
};

} // namespace Subtitles

#endif // SUBTITLECOMMANDS_HPP
