/*
 * Copyright (c) 2016-2021 Meltytech, LLC
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

#ifndef COUNTPRODUCERWIDGET_H
#define COUNTPRODUCERWIDGET_H

// Local
#include "abstractproducerwidget.hpp"

// Qt
#include <MltProducer.h>
#include <MltProperties.h>
#include <QWidget>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>

namespace Ui {
class CountProducerWidget;
}

class CountProducerWidget : public QWidget, public AbstractProducerWidget {
	Q_OBJECT

  public:
	explicit CountProducerWidget(QWidget* parent = nullptr);
	~CountProducerWidget() override;

	// AbstractProducerWidget overrides
	Mlt::Producer*  newProducer(Mlt::Profile&) override;
	[[nodiscard]] Mlt::Properties getPreset() const override;
	void loadPreset(Mlt::Properties&) override;

  signals:
	void producerChanged(Mlt::Producer*);
	void producerReopened(bool play);

  private slots:
	void on_directionCombo_activated(int index);
	void on_styleCombo_activated(int index);
	void on_soundCombo_activated(int index);
	void on_backgroundCombo_activated(int index);
	void on_dropCheckBox_clicked(bool checked);
	void on_durationSpinBox_editingFinished();
	void on_preset_selected(void* p);
	void on_preset_saveClicked();

  private:
	[[nodiscard]] QString detail() const;
	[[nodiscard]] QString currentDirection() const;
	[[nodiscard]] QString currentStyle() const;
	[[nodiscard]] QString currentSound() const;
	[[nodiscard]] QString currentBackground() const;
	Ui::CountProducerWidget* ui;
};

#endif // COUNTPRODUCERWIDGET_H
