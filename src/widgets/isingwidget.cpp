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

// Local
#include "isingwidget.h"
#include "shotcut_mlt_properties.hpp"
#include "ui_isingwidget.h"
#include "util.hpp"

// Qt
#include <MltProducer.h>
#include <MltProperties.h>
#include <qtmetamacros.h>
#include <qlatin1stringview.h>
#include <qobject.h>

// Number constants
constexpr int setIsingWidgetNumberInt{100};
constexpr double setIsingWidgetNumberDouble{100.0};


static constexpr const char* kParamTemperature  = "0";
static constexpr const char* kParamBorderGrowth = "1";
static constexpr const char* kParamSpontaneous  = "2";

IsingWidget::IsingWidget(QWidget* parent) : QWidget(parent), ui(new Ui::IsingWidget) {
	ui->setupUi(this);
	Util::setColorsToHighlight(ui->nameLabel);
	ui->preset->saveDefaultPreset(getPreset());
	ui->preset->loadPresets();
}

IsingWidget::~IsingWidget() {
	delete ui;
}

void IsingWidget::on_tempDial_valueChanged(int value) {
	if (m_producer) {
		m_producer->set(kParamTemperature, value / setIsingWidgetNumberDouble);
		emit producerChanged(m_producer.data());
	}
	ui->tempSpinner->setValue(value / setIsingWidgetNumberDouble);
}

void IsingWidget::on_tempSpinner_valueChanged(double value) {
	ui->tempDial->setValue(value * setIsingWidgetNumberInt);
}

void IsingWidget::on_borderGrowthDial_valueChanged(int value) {
	if (m_producer) {
		m_producer->set(kParamBorderGrowth, value / setIsingWidgetNumberDouble);
		emit producerChanged(m_producer.data());
	}
	ui->borderGrowthSpinner->setValue(value / setIsingWidgetNumberDouble);
}

void IsingWidget::on_borderGrowthSpinner_valueChanged(double value) {
	ui->borderGrowthDial->setValue(value * setIsingWidgetNumberInt);
}

void IsingWidget::on_spontGrowthDial_valueChanged(int value) {
	if (m_producer) {
		m_producer->set(kParamSpontaneous, value / setIsingWidgetNumberDouble);
		emit producerChanged(producer());
	}
	ui->spontGrowthSpinner->setValue(value / setIsingWidgetNumberDouble);
}

void IsingWidget::on_spontGrowthSpinner_valueChanged(double value) {
	ui->spontGrowthDial->setValue(value * setIsingWidgetNumberInt);
}

auto IsingWidget::newProducer(Mlt::Profile& profile) -> Mlt::Producer* {
	auto* p = new Mlt::Producer(profile, "frei0r.ising0r");
	p->set(kParamTemperature, ui->tempSpinner->text().toLatin1().constData());
	p->set(kParamBorderGrowth, ui->borderGrowthSpinner->text().toLatin1().constData());
	p->set(kParamSpontaneous, ui->spontGrowthSpinner->text().toLatin1().constData());
	p->set(kShotcutCaptionProperty, ui->nameLabel->text().toUtf8().constData());
	p->set(kShotcutDetailProperty, ui->nameLabel->text().toUtf8().constData());
	return p;
}

auto IsingWidget::getPreset() const -> Mlt::Properties {
	Mlt::Properties p;
	p.set(kParamTemperature, ui->tempSpinner->text().toLatin1().constData());
	p.set(kParamBorderGrowth, ui->borderGrowthSpinner->text().toLatin1().constData());
	p.set(kParamSpontaneous, ui->spontGrowthSpinner->text().toLatin1().constData());
	return p;
}

void IsingWidget::loadPreset(Mlt::Properties& p) {
	ui->tempSpinner->setValue(p.get_double(kParamTemperature));
	ui->borderGrowthSpinner->setValue(p.get_double(kParamBorderGrowth));
	ui->spontGrowthSpinner->setValue(p.get_double(kParamSpontaneous));
}

void IsingWidget::on_preset_selected(void* p) {
	auto* properties = (Mlt::Properties*)p;
	loadPreset(*properties);
	delete properties;
}

void IsingWidget::on_preset_saveClicked() {
	ui->preset->savePreset(getPreset());
}
