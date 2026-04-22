/*
 * Copyright (c) 2020 Meltytech, LLC
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
#include "widgets/frameratewidget.h"
#include "util.hpp"

// Qt
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <qobjectdefs.h>
#include <qtmetamacros.h>
#include <qwidget.h>

// Number constants
constexpr int setComboBoxSizeNumber{20};
constexpr int setComboBoxMaximumSizeNumber{16777215};

constexpr double setFpsSpinnerNumber24FPS{23.98};
constexpr double setFpsSpinnerSecondNumber24FPS{23.976};
constexpr double setFpsSpinnerNumber30FPS{29.97};
constexpr double setFpsSpinnerNumber48FPS{47.95};
constexpr double setFpsSpinnerNumber59FPS{59.94};

constexpr int showFrameRateDialog24FPS{24000};
constexpr int showFrameRateDialog30FPS{30000};
constexpr int showFrameRateDialog48FPS{48000};
constexpr int showFrameRateDialog60FPS{60000};


FrameRateWidget::FrameRateWidget(QWidget* parent) : QWidget(parent), m_fps(0.0) {
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	m_fpsSpinner = new QDoubleSpinBox();
	m_fpsSpinner->setDecimals(6);
	m_fpsSpinner->setMinimum(1.0);
	m_fpsSpinner->setMaximum(1000);
	m_fpsSpinner->setValue(25.0);
	connect(m_fpsSpinner, SIGNAL(editingFinished()), this, SLOT(on_fpsSpinner_editingFinished()));
	layout->addWidget(m_fpsSpinner);

	m_fpsComboBox = new QComboBox();
	m_fpsComboBox->setMaximumSize(setComboBoxSizeNumber, setComboBoxMaximumSizeNumber);
	m_fpsComboBox->addItem("");
	m_fpsComboBox->addItem("23.976024");
	m_fpsComboBox->addItem("24");
	m_fpsComboBox->addItem("25");
	m_fpsComboBox->addItem("29.970030");
	m_fpsComboBox->addItem("30");
	m_fpsComboBox->addItem("48");
	m_fpsComboBox->addItem("50");
	m_fpsComboBox->addItem("59.940060");
	m_fpsComboBox->addItem("60");
	connect(m_fpsComboBox, SIGNAL(currentTextChanged(const QString&)), this,
	        SLOT(on_fpsComboBox_activated(const QString&)));
	layout->addWidget(m_fpsComboBox);
}

auto FrameRateWidget::fps() -> double {
	return m_fpsSpinner->value();
}

void FrameRateWidget::setFps(double fps) {
	m_fpsSpinner->setValue(fps);
}

void FrameRateWidget::on_fpsSpinner_editingFinished() {
	if (m_fpsSpinner->value() != m_fps) {
		const QString caption(tr("Convert Frames/sec"));
		if (m_fpsSpinner->value() == setFpsSpinnerNumber24FPS || m_fpsSpinner->value() == setFpsSpinnerSecondNumber24FPS) {
			Util::showFrameRateDialog(caption, showFrameRateDialog24FPS, m_fpsSpinner, this);
		} else if (m_fpsSpinner->value() == setFpsSpinnerNumber30FPS) {
			Util::showFrameRateDialog(caption, showFrameRateDialog30FPS, m_fpsSpinner, this);
		} else if (m_fpsSpinner->value() == setFpsSpinnerNumber48FPS) {
			Util::showFrameRateDialog(caption, showFrameRateDialog48FPS, m_fpsSpinner, this);
		} else if (m_fpsSpinner->value() == setFpsSpinnerNumber59FPS) {
			Util::showFrameRateDialog(caption, showFrameRateDialog60FPS, m_fpsSpinner, this);
		}
		m_fps = m_fpsSpinner->value();
		emit fpsChanged(m_fps);
	}
}

void FrameRateWidget::on_fpsComboBox_activated(const QString& arg1) {
	if (!arg1.isEmpty())
		m_fpsSpinner->setValue(arg1.toDouble());
}
