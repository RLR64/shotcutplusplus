/*
 * Copyright (c) 2021-2023 Meltytech, LLC
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

#ifndef MARKERCOMMANDS_HPP
#define MARKERCOMMANDS_HPP

// Local
#include "models/markersmodel.hpp"

// Qt
#include <QUndoCommand>
#include <qlist.h>

namespace Markers {

enum {
	UndoIdUpdate = 200,
};

class DeleteCommand : public QUndoCommand {
  public:
	DeleteCommand(MarkersModel& Model, Marker  DelMarker, int Index);
	void Redo();
	void Undo();

  private:
	MarkersModel& m_model;
	Marker        m_delMarker;
	int           m_index;
};

class AppendCommand : public QUndoCommand {
  public:
	AppendCommand(MarkersModel& Model, Marker  NewMarker, int Index);
	void Redo();
	void Undo();

  private:
	MarkersModel& m_model;
	Marker        m_newMarker;
	int           m_index;
};

class UpdateCommand : public QUndoCommand {
  public:
	UpdateCommand(MarkersModel& Model, Marker  NewMarker, Marker  OldMarker, int Index);
	void Redo();
	void Undo();

  protected:
	[[nodiscard]] auto id() const -> int override {
		return UndoIdUpdate;
	}

	auto mergeWith(const QUndoCommand* Other) -> bool override;

  private:
	MarkersModel& m_model;
	Marker        m_newMarker;
	Marker        m_oldMarker;
	int           m_index;
};

class ClearCommand : public QUndoCommand {
  public:
	ClearCommand(MarkersModel& Model, QList<Marker>& ClearMarkers);
	void Redo();
	void Undo();

  private:
	MarkersModel& m_model;
	QList<Marker> m_clearMarkers;
};

} // namespace Markers

#endif // MARKERCOMMANDS_HPP
