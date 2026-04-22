/*
 * Copyright (c) 2012-2023 Meltytech, LLC
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

#ifndef OPENOTHERDIALOG_HPP
#define OPENOTHERDIALOG_HPP

// Qt
#include <QDialog>
#include <QTreeWidgetItem>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>

namespace Ui {
class OpenOtherDialog;
}

namespace Mlt {
class Properties;
class Producer;
class Profile;
} // namespace Mlt
class QPushButton;

class OpenOtherDialog : public QDialog {
	Q_OBJECT

  public:
	explicit OpenOtherDialog(QWidget* parent = nullptr);
	~OpenOtherDialog() override;

	Mlt::Producer* newProducer(Mlt::Profile&) const;
	void load(Mlt::Producer*);

	[[nodiscard]] QWidget* currentWidget() const {
		return m_current;
	}

  private slots:
	void on_treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

  private:
	Ui::OpenOtherDialog* ui;
	QWidget* m_current;
	QPushButton* m_addTimelineButton;

	Mlt::Producer* newProducer(Mlt::Profile&, QObject* widget) const;
	void selectTreeWidget(const QString& s);
};

#endif // OPENOTHERDIALOG_HPP
