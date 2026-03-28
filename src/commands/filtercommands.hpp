/*
 * Copyright (c) 2021-2024 Meltytech, LLC
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

#ifndef FILTERCOMMANDS_HPP
#define FILTERCOMMANDS_HPP

#include "models/attachedfiltersmodel.hpp"

#include <MltProducer.h>
#include <MltService.h>
#include <QString>
#include <QUndoCommand>
#include <QUuid>

class QmlMetadata;
class FilterController;

namespace Filter {

enum {
	UndoIdAdd = 300,
	UndoIdMove,
	UndoIdDisable,
	UndoIdChangeParameter,
	UndoIdChangeAddKeyframe,
	UndoIdChangeRemoveKeyframe,
	UndoIdChangeKeyframe,
};

class AddCommand : public QUndoCommand {
  public:
	using AddType = enum {
		AddSingle,
		AddSet,
		AddSetLast,
	};

	AddCommand(AttachedFiltersModel& Model, const QString& Name, Mlt::Service& Service, int Row,
	           AddCommand::AddType Type = AddCommand::AddSingle, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdAdd;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	AttachedFiltersModel&     m_model;
	std::vector<int>          m_rows;
	std::vector<Mlt::Service> m_services;
	Mlt::Producer             m_producer;
	QUuid                     m_producerUuid;
	AddType                   m_type;
};

class RemoveCommand : public QUndoCommand {
  public:
	RemoveCommand(AttachedFiltersModel& Model, const QString& Name, Mlt::Service& Service, int Row,
	              QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	AttachedFiltersModel& m_model;
	int                   m_index;
	int                   m_row;
	Mlt::Producer         m_producer;
	QUuid                 m_producerUuid;
	Mlt::Service          m_service;
};

class MoveCommand : public QUndoCommand {
  public:
	MoveCommand(AttachedFiltersModel& Model, const QString& Name, int FromRow, int ToRow,
	            QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdMove;
	}

  private:
	AttachedFiltersModel& m_model;
	int                   m_fromRow;
	int                   m_toRow;
	Mlt::Producer         m_producer;
	QUuid                 m_producerUuid;
};

class DisableCommand : public QUndoCommand {
  public:
	DisableCommand(AttachedFiltersModel& Model, const QString& Name, int Row, bool Disabled,
	               QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdDisable;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	AttachedFiltersModel& m_model;
	int                   m_row;
	Mlt::Producer         m_producer;
	QUuid                 m_producerUuid;
	bool                  m_disabled;
};

class PasteCommand : public QUndoCommand {
  public:
	PasteCommand(AttachedFiltersModel& Model, const QString& FilterProducerXml, QUndoCommand* Parent = nullptr);
	void Redo();
	void Undo();

  private:
	AttachedFiltersModel& m_model;
	QString               m_xml;
	QString               m_beforeXml;
	QUuid                 m_producerUuid;
};

class UndoParameterCommand : public QUndoCommand {
  public:
	UndoParameterCommand(const QString& Name, FilterController* Controller, int Row, Mlt::Properties& Before,
	                     const QString& Desc = QString(), QUndoCommand* Parent = nullptr);
	void Update(const QString& PropertyName);
	void Redo();
	void Undo();

  protected:
	int Id() const {
		return UndoIdChangeParameter;
	}

	bool MergeWith(const QUndoCommand* Other);

  private:
	int               m_row;
	QUuid             m_producerUuid;
	Mlt::Properties   m_before;
	Mlt::Properties   m_after;
	FilterController* m_filterController;
	bool              m_firstRedo;
};

class UndoAddKeyframeCommand : public UndoParameterCommand {
  public:
	UndoAddKeyframeCommand(const QString& Name, FilterController* Controller, int Row, Mlt::Properties& Before)
	    : UndoParameterCommand(Name, Controller, Row, Before, QObject::tr("add keyframe")) {
	}

  protected:
	int id() const {
		return UndoIdChangeAddKeyframe;
	}

	bool mergeWith(const QUndoCommand* Other) {
		return false;
	}
};

class UndoRemoveKeyframeCommand : public UndoParameterCommand {
  public:
	UndoRemoveKeyframeCommand(const QString& Name, FilterController* Controller, int Row, Mlt::Properties& Before)
	    : UndoParameterCommand(Name, Controller, Row, Before, QObject::tr("remove keyframe")) {
	}

  protected:
	int id() const {
		return UndoIdChangeRemoveKeyframe;
	}

	bool mergeWith(const QUndoCommand* Other) {
		return false;
	}
};

class UndoModifyKeyframeCommand : public UndoParameterCommand {
  public:
	UndoModifyKeyframeCommand(const QString& Name, FilterController* Controller, int Row, Mlt::Properties& Before,
	                          int ParamIndex, int KeyframeIndex)
	    : UndoParameterCommand(Name, Controller, Row, Before, QObject::tr("modify keyframe")), m_paramIndex(ParamIndex),
	      m_keyframeIndex(KeyframeIndex) {
	}

  protected:
	int id() const {
		return UndoIdChangeRemoveKeyframe;
	}

	bool mergeWith(const QUndoCommand* Other) {
		auto* That = dynamic_cast<const UndoModifyKeyframeCommand*>(Other);
		if (!That || m_paramIndex != That->m_paramIndex || m_keyframeIndex != That->m_keyframeIndex) {
			return false;
		}
		return UndoParameterCommand::mergeWith(Other);
	}

  private:
	int m_paramIndex;
	int m_keyframeIndex;
};

} // namespace Filter

#endif // FILTERCOMMANDS_HPP
