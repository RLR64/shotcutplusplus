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

// Local
#include "subtitlecommands.hpp"
#include "Logger.hpp"
#include "mainwindow.hpp"
#include "models/subtitles.hpp"
#include "models/subtitlesmodel.hpp"

// Qt
#include <QFileInfo>
#include <QTreeWidget>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <qundostack.h>

// STL
#include <cstdint>
#include <utility>

namespace Subtitles {

InsertTrackCommand::InsertTrackCommand(SubtitlesModel& model, SubtitlesModel::SubtitleTrack  track, int index)
	: QUndoCommand(nullptr), m_model(model), m_track(std::move(track)), m_index(index) {
	setText(QObject::tr("Add subtitle track: %1").arg(m_track.name));
}

void InsertTrackCommand::Redo() {
	LOG_DEBUG() << m_track.name;
	m_model.doInsertTrack(m_track, m_index);
}

void InsertTrackCommand::Undo() {
	m_model.doRemoveTrack(m_index);
}

RemoveTrackCommand::RemoveTrackCommand(SubtitlesModel& model, int trackIndex)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_saveTrack(m_model.getTrack(trackIndex)) {
	setText(QObject::tr("Remove subtitle track: %1").arg(m_saveTrack.name));
	const int count = m_model.itemCount(m_trackIndex);
	m_saveSubtitles.reserve(count);
	for (int i = 0; i < count; i++) {
		m_saveSubtitles.push_back(m_model.getItem(m_trackIndex, i));
	}
}

void RemoveTrackCommand::Redo() {
	m_model.doRemoveTrack(m_trackIndex);
}

void RemoveTrackCommand::Undo() {
	m_model.doInsertTrack(m_saveTrack, m_trackIndex);
	m_model.doInsertSubtitleItems(m_trackIndex, m_saveSubtitles);
}

EditTrackCommand::EditTrackCommand(SubtitlesModel& model, SubtitlesModel::SubtitleTrack  track, int index)
	: QUndoCommand(nullptr), m_model(model), m_newTrack(std::move(track)), m_index(index) {
	m_oldTrack = m_model.getTrack(index);
	setText(QObject::tr("Edit subtitle track: %1").arg(m_newTrack.name));
}

void EditTrackCommand::Redo() {
	LOG_DEBUG() << m_oldTrack.name;
	m_model.doEditTrack(m_newTrack, m_index);
}

void EditTrackCommand::Undo() {
	m_model.doEditTrack(m_oldTrack, m_index);
}

OverwriteSubtitlesCommand::OverwriteSubtitlesCommand(SubtitlesModel& model, int trackIndex, const QList<Subtitles::SubtitleItem>& items)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_newSubtitles(items) {
	if (m_newSubtitles.size() == 1) {
		setText(QObject::tr("Add subtitle"));
	} else {
		setText(QObject::tr("Add %n subtitles", nullptr, m_newSubtitles.size()));
	}

	if (m_newSubtitles.size() <= 0) {
		return;
	}
	// Save anything that will be removed
	const int64_t startPosition = m_newSubtitles[0].start;
	const int64_t endPosition = m_newSubtitles[m_newSubtitles.size() - 1].end;
	const int count = m_model.itemCount(m_trackIndex);
	for (int i = 0; i < count; i++) {
		auto item = m_model.getItem(m_trackIndex, i);
		if ((item.start >= startPosition && item.start < endPosition) ||
		    (item.end > startPosition && item.end < endPosition)) {
			m_saveSubtitles.push_back(item);
		}
	}
}

void OverwriteSubtitlesCommand::Redo() {
	LOG_DEBUG() << m_newSubtitles.size();

	if (m_newSubtitles.size() > 0) {
		if (m_saveSubtitles.size() > 0) {
			m_model.doRemoveSubtitleItems(m_trackIndex, m_saveSubtitles);
		}
		m_model.doInsertSubtitleItems(m_trackIndex, m_newSubtitles);
	}
}

void OverwriteSubtitlesCommand::Undo() {
	LOG_DEBUG() << m_newSubtitles.size();

	if (m_newSubtitles.size() > 0) {
		m_model.doRemoveSubtitleItems(m_trackIndex, m_newSubtitles);
		if (m_saveSubtitles.size() > 0) {
			m_model.doInsertSubtitleItems(m_trackIndex, m_saveSubtitles);
		}
	}
}

RemoveSubtitlesCommand::RemoveSubtitlesCommand(SubtitlesModel& model, int trackIndex, const QList<Subtitles::SubtitleItem>& items)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_items(items) {
	if (m_items.size() == 1) {
		setText(QObject::tr("Remove subtitle"));
	} else {
		setText(QObject::tr("Remove %n subtitles", nullptr, m_items.size()));
	}
}

void RemoveSubtitlesCommand::Redo() {
	LOG_DEBUG() << m_items.size();
	m_model.doRemoveSubtitleItems(m_trackIndex, m_items);
}

void RemoveSubtitlesCommand::Undo() {
	LOG_DEBUG() << m_items.size();
	if (m_items.size() > 0) {
		m_model.doInsertSubtitleItems(m_trackIndex, m_items);
	}
}

SetTextCommand::SetTextCommand(SubtitlesModel& model, int trackIndex, int itemIndex, QString  text)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_itemIndex(itemIndex), m_newText(std::move(text)) {
	setText(QObject::tr("Edit subtitle text"));
	m_oldText = QString::fromStdString(m_model.getItem(trackIndex, itemIndex).text);
}

void SetTextCommand::Redo() {
	m_model.doSetText(m_trackIndex, m_itemIndex, m_newText);
}

void SetTextCommand::Undo() {
	m_model.doSetText(m_trackIndex, m_itemIndex, m_oldText);
}

auto SetTextCommand::mergeWith(const QUndoCommand* other) -> bool {
	const auto* that = dynamic_cast<const SetTextCommand*>(other);
	if (m_trackIndex != that->m_trackIndex || m_itemIndex != that->m_itemIndex) {
		return false;
	}
	LOG_DEBUG() << "track" << m_trackIndex << "item" << m_itemIndex;
	m_newText = that->m_newText;
	return true;
}

SetStartCommand::SetStartCommand(SubtitlesModel& model, int trackIndex, int itemIndex, int64_t msTime)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_itemIndex(itemIndex), m_newStart(msTime) {
	setText(QObject::tr("Change subtitle start"));
	m_oldStart = m_model.getItem(trackIndex, itemIndex).start;
}

void SetStartCommand::Redo() {
	const int64_t endTime = m_model.getItem(m_trackIndex, m_itemIndex).end;
	m_model.doSetTime(m_trackIndex, m_itemIndex, m_newStart, endTime);
}

void SetStartCommand::Undo() {
	const int64_t endTime = m_model.getItem(m_trackIndex, m_itemIndex).end;
	m_model.doSetTime(m_trackIndex, m_itemIndex, m_oldStart, endTime);
}

auto SetStartCommand::mergeWith(const QUndoCommand* other) -> bool {
	const auto* that = dynamic_cast<const SetStartCommand*>(other);
	if (m_trackIndex != that->m_trackIndex || m_itemIndex != that->m_itemIndex) {
		return false;
	}
	LOG_DEBUG() << "track" << m_trackIndex << "item" << m_itemIndex;
	m_newStart = that->m_newStart;
	return true;
}

SetEndCommand::SetEndCommand(SubtitlesModel& model, int trackIndex, int itemIndex, int64_t msTime)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_itemIndex(itemIndex), m_newEnd(msTime) {
	setText(QObject::tr("Change subtitle end"));
	m_oldEnd = m_model.getItem(trackIndex, itemIndex).end;
}

void SetEndCommand::Redo() {
	const int64_t startTime = m_model.getItem(m_trackIndex, m_itemIndex).start;
	m_model.doSetTime(m_trackIndex, m_itemIndex, startTime, m_newEnd);
}

void SetEndCommand::Undo() {
	const int64_t startTime = m_model.getItem(m_trackIndex, m_itemIndex).start;
	m_model.doSetTime(m_trackIndex, m_itemIndex, startTime, m_oldEnd);
}

auto SetEndCommand::mergeWith(const QUndoCommand* other) -> bool {
	const auto* that = dynamic_cast<const SetEndCommand*>(other);
	if (m_trackIndex != that->m_trackIndex || m_itemIndex != that->m_itemIndex) {
		return false;
	}
	LOG_DEBUG() << "track" << m_trackIndex << "item" << m_itemIndex;
	m_newEnd = that->m_newEnd;
	return true;
}

MoveSubtitlesCommand::MoveSubtitlesCommand(SubtitlesModel& model, int trackIndex, const QList<Subtitles::SubtitleItem>& items, int64_t msTime)
	: QUndoCommand(nullptr), m_model(model), m_trackIndex(trackIndex), m_oldSubtitles(items) {
	if (m_oldSubtitles.size() <= 0) {
		return;
	}
	if (m_oldSubtitles.size() == 1) {
		setText(QObject::tr("Move subtitle"));
	} else {
		setText(QObject::tr("Move %n subtitles", nullptr, m_oldSubtitles.size()));
	}
	// Create a list of subtitles with the new times
	const int64_t delta = msTime - m_oldSubtitles[0].start;
	for (int i = 0; i < m_oldSubtitles.size(); i++) {
		m_newSubtitles.push_back(m_oldSubtitles[i]);
		m_newSubtitles[i].start += delta;
		m_newSubtitles[i].end += delta;
	}
}

void MoveSubtitlesCommand::Redo() {
	LOG_DEBUG() << m_oldSubtitles.size();
	m_model.doRemoveSubtitleItems(m_trackIndex, m_oldSubtitles);
	m_model.doInsertSubtitleItems(m_trackIndex, m_newSubtitles);
}

void MoveSubtitlesCommand::Undo() {
	LOG_DEBUG() << m_oldSubtitles.size();
	m_model.doRemoveSubtitleItems(m_trackIndex, m_newSubtitles);
	m_model.doInsertSubtitleItems(m_trackIndex, m_oldSubtitles);
}

auto MoveSubtitlesCommand::mergeWith(const QUndoCommand* other) -> bool {
	const auto* that = dynamic_cast<const MoveSubtitlesCommand*>(other);
	if (m_trackIndex != that->m_trackIndex) {
		return false;
	}
	if (m_oldSubtitles.size() != that->m_oldSubtitles.size()) {
		return false;
	}
	if (m_newSubtitles[0].start != that->m_oldSubtitles[0].start) {
		return false;
	}
	LOG_DEBUG() << "track" << m_trackIndex;
	m_newSubtitles = that->m_newSubtitles;
	return true;
}

} // namespace Subtitles
