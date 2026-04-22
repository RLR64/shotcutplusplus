/*
 * Copyright (c) 2021 Meltytech, LLC
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
#include "markercommands.hpp"
#include "Logger.hpp"
#include "models/markersmodel.hpp"

// Qt
#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qundostack.h>

// STL
#include <utility>

namespace Markers {

DeleteCommand::DeleteCommand(MarkersModel& model, Marker  delMarker, int index)
	: QUndoCommand(nullptr), m_model(model), m_delMarker(std::move(delMarker)), m_index(index) {
	setText(QObject::tr("Delete marker: %1").arg(m_delMarker.text));
}

void DeleteCommand::Redo() {
	m_model.doRemove(m_index);
}

void DeleteCommand::Undo() {
	m_model.doInsert(m_index, m_delMarker);
}

AppendCommand::AppendCommand(MarkersModel& model, Marker  newMarker, int index)
	: QUndoCommand(nullptr), m_model(model), m_newMarker(std::move(newMarker)), m_index(index) {
	setText(QObject::tr("Add marker: %1").arg(m_newMarker.text));
}

void AppendCommand::Redo() {
	m_model.doAppend(m_newMarker);
}

void AppendCommand::Undo() {
	m_model.doRemove(m_index);
}

UpdateCommand::UpdateCommand(MarkersModel& model, Marker  newMarker, Marker  oldMarker, int index)
	: QUndoCommand(nullptr), m_model(model), m_newMarker(std::move(newMarker)), m_oldMarker(std::move(oldMarker)), m_index(index) {
	if (m_newMarker.text == m_oldMarker.text && m_newMarker.color == m_oldMarker.color) {
		setText(QObject::tr("Move marker: %1").arg(m_oldMarker.text));
	} else {
		setText(QObject::tr("Edit marker: %1").arg(m_oldMarker.text));
	}
}

void UpdateCommand::Redo() {
	m_model.doUpdate(m_index, m_newMarker);
}

void UpdateCommand::Undo() {
	m_model.doUpdate(m_index, m_oldMarker);
}

auto UpdateCommand::mergeWith(const QUndoCommand* other) -> bool {
	const auto* that = dynamic_cast<const UpdateCommand*>(other);
	LOG_DEBUG() << "this index" << m_index << "that index" << that->m_index;
	if (that->id() != id() || that->m_index != m_index)
		return false;
	bool merge = false;
	if (that->m_newMarker.text == m_oldMarker.text && that->m_newMarker.color == m_oldMarker.color) {
		// Only start/end change. Merge with previous move command.
		merge = true;
	} else if (that->m_newMarker.end == m_oldMarker.end && that->m_newMarker.start == m_oldMarker.start) {
		// Only text/color change. Merge with previous edit command.
		merge = true;
	}
	if (!merge)
		return false;
	m_newMarker = that->m_newMarker;
	return true;
}

ClearCommand::ClearCommand(MarkersModel& model, QList<Marker>& clearMarkers)
	: QUndoCommand(nullptr), m_model(model), m_clearMarkers(clearMarkers) {
	setText(QObject::tr("Clear markers"));
}

void ClearCommand::Redo() {
	m_model.doClear();
}

void ClearCommand::Undo() {
	m_model.doReplace(m_clearMarkers);
}

} // namespace Markers
