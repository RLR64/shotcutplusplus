/*
 * Copyright (c) 2013-2024 Meltytech, LLC
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

#ifndef PLAYLISTCOMMANDS_HPP
#define PLAYLISTCOMMANDS_HPP

#include "models/playlistmodel.hpp"

#include <QString>
#include <QUndoCommand>
#include <QUuid>

class QTreeWidget;

namespace Playlist {

enum { UndoIdTrimClipIn = 0, UndoIdTrimClipOut, UndoIdUpdate };

class AppendCommand : public QUndoCommand {
  public:
	AppendCommand(PlaylistModel& Model, const QString& Xml, bool EmitModified = true, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QString        m_xml;
	bool           m_emitModified;
	QUuid          m_uuid;
};

class InsertCommand : public QUndoCommand {
  public:
	InsertCommand(PlaylistModel& Model, const QString& Xml, int Row, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QString        m_xml;
	int            m_row;
	QUuid          m_uuid;
};

class UpdateCommand : public QUndoCommand {
  public:
	UpdateCommand(PlaylistModel& Model, const QString& Xml, int Row, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdUpdate;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	PlaylistModel& m_model;
	QString        m_newXml;
	QString        m_oldXml;
	int            m_row;
	QUuid          m_uuid;
};

class RemoveCommand : public QUndoCommand {
  public:
	RemoveCommand(PlaylistModel& Model, int Row, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QString        m_xml;
	int            m_row;
	QUuid          m_uuid;
};

class MoveCommand : public QUndoCommand {
  public:
	MoveCommand(PlaylistModel& Model, int From, int To, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	int            m_from;
	int            m_to;
};

class ClearCommand : public QUndoCommand {
  public:
	ClearCommand(PlaylistModel& Model, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QString        m_xml;
	QVector<QUuid> m_uuids;
};

class SortCommand : public QUndoCommand {
  public:
	SortCommand(PlaylistModel& Model, int Column, Qt::SortOrder Order, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	int            m_column;
	Qt::SortOrder  m_order;
	QString        m_xml;
	QVector<QUuid> m_uuids;
};

class TrimClipInCommand : public QUndoCommand {
  public:
	TrimClipInCommand(PlaylistModel& Model, int Row, int In, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimClipIn;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	PlaylistModel& m_model;
	int            m_row;
	int            m_oldIn;
	int            m_newIn;
	int            m_out;
};

class TrimClipOutCommand : public QUndoCommand {
  public:
	TrimClipOutCommand(PlaylistModel& Model, int Row, int Out, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdTrimClipOut;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	PlaylistModel& m_model;
	int            m_row;
	int            m_in;
	int            m_oldOut;
	int            m_newOut;
};

class ReplaceCommand : public QUndoCommand {
  public:
	ReplaceCommand(PlaylistModel& Model, const QString& Xml, int Row, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QString        m_newXml;
	QString        m_oldXml;
	int            m_row;
	QUuid          m_uuid;
};

class NewBinCommand : public QUndoCommand {
  public:
	NewBinCommand(PlaylistModel& Model, QTreeWidget* Tree, const QString& Bin, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel&  m_model;
	QTreeWidget*    m_binTree;
	QString         m_bin;
	Mlt::Properties m_oldBins;
};

class MoveToBinCommand : public QUndoCommand {
  public:
	MoveToBinCommand(PlaylistModel& Model, QTreeWidget* Tree, const QString& Bin, const QList<int>& Rows,
	                 QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	PlaylistModel& m_model;
	QTreeWidget*   m_binTree;
	QString        m_bin;

	using oldData = struct {
		int     row;
		QString bin;
	};

	QList<oldData> m_oldData;
};

class RenameBinCommand : public QUndoCommand {
  public:
	RenameBinCommand(PlaylistModel& Model, QTreeWidget* Tree, const QString& Bin, const QString& NewName = QString(),
	                 QUndoCommand* Parent = nullptr);
	void        Redo();
	void        Undo();
	static void RebuildBinList(PlaylistModel& Model, QTreeWidget* BinTree);

  private:
	PlaylistModel& m_model;
	QTreeWidget*   m_binTree;
	QString        m_bin;
	QString        m_newName;
	QList<int>     m_removedRows;
};

} // namespace Playlist

#endif // PLAYLISTCOMMANDS_HPP
