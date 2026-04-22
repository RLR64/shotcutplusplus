/*
 * Copyright (c) 2015-2020 Meltytech, LLC
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

#ifndef UNDOHELPER_HPP
#define UNDOHELPER_HPP

// Local
#include "models/multitrackmodel.hpp"

// Qt
#include <MltPlaylist.h>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <qobject.h>

class UndoHelper {
  public:
	enum OptimizationHints { NoHints, SkipXML, RestoreTracks };

	UndoHelper(MultitrackModel& Model);

	void RecordBeforeState();
	void RecordAfterState();
	void UndoChanges();
	void SetHints(OptimizationHints Hints);

	[[nodiscard]] auto AffectedTracks() const -> QSet<int> {
		return m_affectedTracks;
	}

  private:
	void DebugPrintState(const QString& Title);
	void RestoreAffectedTracks();
	void FixTransitions(Mlt::Playlist Playlist, int ClipIndex, Mlt::Producer Clip);

	enum ChangeFlags { NoChange = 0x0, ClipInfoModified = 0x1, XMLModified = 0x2, Moved = 0x4, Removed = 0x8 };

	struct Info {
		int     oldTrackIndex{-1};
		int     oldClipIndex{-1};
		int     newTrackIndex{-1};
		int     newClipIndex{-1};
		bool    isBlank{false};
		QString m_xml{};
		int     frame_in{-1};
		int     frame_out{-1};
		int     in_delta{0};
		int     out_delta{0};
		int     group{-1};

		int changes{NoChange};

		Info()

		{
		}
	};

	QMap<QUuid, Info> m_state;
	QList<QUuid>      m_clipsAdded;
	QList<QUuid>      m_insertedOrder;
	QSet<int>         m_affectedTracks;
	MultitrackModel&  m_model;
	OptimizationHints m_hints;
};

#endif // UNDOHELPER_HPP
