/*
 * Copyright (c) 2012-2022 Meltytech, LLC
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

#ifndef TIMESPINBOX_H
#define TIMESPINBOX_H

// Qt
#include <QLineEdit>
#include <QSpinBox>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class QRegularExpressionValidator;

class TimeSpinBox : public QSpinBox {
	Q_OBJECT

  public:
	explicit TimeSpinBox(QWidget* parent = nullptr);

  protected:
	QValidator::State validate(QString& input, int& pos) const override;
	[[nodiscard]] int valueFromText(const QString& text) const override;
	[[nodiscard]] QString textFromValue(int val) const override;
	void keyPressEvent(QKeyEvent* event) override;

  signals:
	void accepted();

  private:
	QRegularExpressionValidator* m_validator;
};

class TimeSpinBoxLineEdit : public QLineEdit {
	Q_OBJECT

  public:
	explicit TimeSpinBoxLineEdit(QWidget* parent = nullptr);

  protected:
	void focusInEvent(QFocusEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

  private:
	bool m_selectOnMousePress;
};

#endif // TIMESPINBOX_H
