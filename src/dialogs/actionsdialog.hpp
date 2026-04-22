/*
 * Copyright (c) 2022-2024 Meltytech, LLC
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

#ifndef ACTIONSDIALOG_HPP
#define ACTIONSDIALOG_HPP

// Local
#include "models/actionsmodel.hpp"

// Qt
#include <QDialog>
#include <qtmetamacros.h>
#include <qwidget.h>

class PrivateTreeView;
class QLineEdit;
class QSortFilterProxyModel;
class StatusLabelWidget;
class QKeySequenceEdit;

class ActionsDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ActionsDialog(QWidget* parent = nullptr);
	void saveCurrentEditor();

  public slots:
	void focusSearchResults();

  protected:
	void hideEvent(QHideEvent* event) override;
	void showEvent(QShowEvent* event) override;

  private:
	QLineEdit*             m_searchField;
	ActionsModel           m_model;
	PrivateTreeView*       m_table;
	QSortFilterProxyModel* m_proxyModel;
	StatusLabelWidget*     m_status;
};

#endif // ACTIONSDIALOG_HPP
