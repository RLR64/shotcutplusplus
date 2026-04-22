/*
 * Copyright (c) 2012-2018 Meltytech, LLC
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

#ifndef ALSAWIDGET_H
#define ALSAWIDGET_H

// Local
#include "abstractproducerwidget.hpp"

// Qt
#include <MltProducer.h>
#include <MltProperties.h>
#include <QWidget>
#include <qtmetamacros.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>

namespace Ui {
class AlsaWidget;
}

class AlsaWidget : public QWidget, public AbstractProducerWidget {
	Q_OBJECT

  public:
	explicit AlsaWidget(QWidget* parent = nullptr);
	~AlsaWidget() override;

	// AbstractProducerWidget overrides
	auto newProducer(Mlt::Profile& profile) -> Mlt::Producer* override;
	[[nodiscard]] auto getPreset() const -> Mlt::Properties override;
	void loadPreset(Mlt::Properties&) override;
	void setProducer(Mlt::Producer*) override;

  private slots:
	void on_preset_selected(void* p);
	void on_preset_saveClicked();
	void on_applyButton_clicked();

  private:
	Ui::AlsaWidget* ui;
};

#endif // ALSAWIDGET_H
