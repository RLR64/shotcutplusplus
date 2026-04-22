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

#ifndef MULTIFILEEXPORTDIALOG_HPP
#define MULTIFILEEXPORTDIALOG_HPP

// Qt
#include <QDialog>
#include <QStringList>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QListWidget;
class QLineEdit;

namespace Mlt {
class Playlist;
}

class MultiFileExportDialog : public QDialog {
	Q_OBJECT
  public:
	explicit MultiFileExportDialog(const QString& title, Mlt::Playlist* playlist, const QString& directory,
	                               const QString& prefix, const QString& extension, QWidget* parent = nullptr);
	auto getExportFiles() -> QStringList;

  private slots:
	void rebuildList();
	void browse();

  private:
	auto appendField(QString text, QComboBox* combo, int clipIndex) -> QString;
	void    fillCombo(QComboBox* combo);

	Mlt::Playlist*    m_playlist;
	QLineEdit*        m_dir;
	QLineEdit*        m_prefix;
	QComboBox*        m_field1;
	QComboBox*        m_field2;
	QComboBox*        m_field3;
	QLineEdit*        m_ext;
	QLabel*           m_errorIcon;
	QLabel*           m_errorText;
	QListWidget*      m_list;
	QDialogButtonBox* m_buttonBox;
	QStringList       m_stringList;
};

#endif // MULTIFILEEXPORTDIALOG_HPP
