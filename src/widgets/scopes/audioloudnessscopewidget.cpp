/*
 * Copyright (c) 2016-2026 Meltytech, LLC
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

// local
#include "audioloudnessscopewidget.h"
#include "Logger.hpp"
#include "mltcontroller.hpp"
#include "qmltypes/qmlutilities.hpp"
#include "settings.hpp"
#include "sharedframe.hpp"
#include "widgets/scopes/scopewidget.h"

// Qt
#include <MltFilter.h>
#include <MltFrame.h>
#include <QDir>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <framework/mlt_types.h>
#include <qaction.h>
#include <qcoreevent.h>
#include <qicon.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qsize.h>

// STL
#include <algorithm>
#include <cmath>

// Number constants

// Onedec
constexpr double setOnedecNumber{10.0};

// AudioLoudnessScopeWidget
constexpr int setPeakNumber{-100};
constexpr int setPeakTrueNumber{-100};
constexpr int setMaximumWidthNumber{100};
constexpr int setTimer{100};
constexpr int setSingleShotNumber{300};
constexpr int setMaxMeterWidthX{200};
constexpr int setMaxMeterWidthY{80};
constexpr int setMinimumSizeNumber{250};
constexpr int setMaximumSizeNumber{600};
constexpr int setYNumber{32};

static constexpr auto onedec(double in) -> double {
	return round(in * setOnedecNumber) / setOnedecNumber;
}

AudioLoudnessScopeWidget::AudioLoudnessScopeWidget()
	: ScopeWidget("AudioLoudnessMeter"), m_loudnessFilter(nullptr), m_peak(setPeakNumber), m_true_peak(setPeakTrueNumber), m_newData(false),
      m_orientation((Qt::Orientation)-1), m_qview(new QQuickWidget(QmlUtilities::sharedEngine(), this)),
      m_timeLabel(new QLabel(this)) {
	LOG_DEBUG() << "begin";
	m_loudnessFilter = new Mlt::Filter(MLT.profile(), "loudness_meter");
	m_loudnessFilter->set("calc_program", Settings.loudnessScopeShowMeter("integrated"));
	m_loudnessFilter->set("calc_shortterm", Settings.loudnessScopeShowMeter("shortterm"));
	m_loudnessFilter->set("calc_momentary", Settings.loudnessScopeShowMeter("momentary"));
	m_loudnessFilter->set("calc_range", Settings.loudnessScopeShowMeter("range"));
	m_loudnessFilter->set("calc_peak", Settings.loudnessScopeShowMeter("peak"));
	m_loudnessFilter->set("calc_true_peak", Settings.loudnessScopeShowMeter("truepeak"));

	setAutoFillBackground(true);
	setWhatsThis("https://forum.shotcut.org/t/audio-loudness-scope/12917/1");

	// Use a timer to update the meters for two reasons:
	// 1) The spec requires 10Hz updates
	// 2) Minimize QML GUI updates
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateMeters()));
	m_timer->start(setTimer);

	m_qview->setFocusPolicy(Qt::StrongFocus);
	QmlUtilities::setCommonProperties(m_qview->rootContext());

	auto* vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(4, 4, 4, 4);
	vlayout->addWidget(m_qview);

	auto* hlayout = new QHBoxLayout();
	vlayout->addLayout(hlayout);

	// Create config menu
	QMenu*   configMenu = new QMenu(this);
	QAction* action = nullptr;
	action = configMenu->addAction(tr("Momentary Loudness"), this, SLOT(onMomentaryToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("momentary"));
	action = configMenu->addAction(tr("Short Term Loudness"), this, SLOT(onShorttermToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("shortterm"));
	action = configMenu->addAction(tr("Integrated Loudness"), this, SLOT(onIntegratedToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("integrated"));
	action = configMenu->addAction(tr("Loudness Range"), this, SLOT(onRangeToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("range"));
	action = configMenu->addAction(tr("Peak"), this, SLOT(onPeakToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("peak"));
	action = configMenu->addAction(tr("True Peak"), this, SLOT(onTruePeakToggled(bool)));
	action->setCheckable(true);
	action->setChecked(Settings.loudnessScopeShowMeter("truepeak"));

	// Add config button
	auto* configButton = new QToolButton(this);
	configButton->setToolTip(tr("Configure Graphs"));
	configButton->setIcon(QIcon::fromTheme("show-menu", QIcon(":/icons/oxygen/32x32/actions/show-menu.png")));
	configButton->setPopupMode(QToolButton::InstantPopup);
	configButton->setMenu(configMenu);
	hlayout->addWidget(configButton);

	// Add reset button
	auto* resetButton = new QPushButton(tr("Reset"), this);
	resetButton->setToolTip(tr("Reset the measurement."));
	resetButton->setCheckable(false);
	resetButton->setMaximumWidth(setMaximumWidthNumber);
	hlayout->addWidget(resetButton);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));

	// Add time label
	m_timeLabel->setToolTip(tr("Time Since Reset"));
	m_timeLabel->setText("00:00:00:00");
	m_timeLabel->setFixedSize(this->fontMetrics().horizontalAdvance("HH:MM:SS:MM"), this->fontMetrics().height());
	hlayout->addWidget(m_timeLabel);

	hlayout->addStretch();

	QTimer::singleShot(setSingleShotNumber, this, &AudioLoudnessScopeWidget::resetQview);

	connect(this, &ScopeWidget::moved, this, &AudioLoudnessScopeWidget::resetQview);

	LOG_DEBUG() << "end";
}

AudioLoudnessScopeWidget::~AudioLoudnessScopeWidget() {
	m_timer->stop();
	delete m_loudnessFilter;
}

void AudioLoudnessScopeWidget::refreshScope(const QSize& /*size*/, bool /*full*/) {
	SharedFrame sFrame;
	while (m_queue.count() > 0) {
		sFrame = m_queue.pop();
		if (sFrame.is_valid() && sFrame.get_audio_samples() > 0) {
			mlt_audio_format format = mlt_audio_f32le;
			int channels = sFrame.get_audio_channels();
			int frequency = sFrame.get_audio_frequency();
			int samples = sFrame.get_audio_samples();
			if (channels && frequency && samples) {
				Mlt::Frame mFrame = sFrame.clone(true, false, false);
				m_loudnessFilter->process(mFrame);
				mFrame.get_audio(format, frequency, channels, samples);
				if (m_peak < m_loudnessFilter->get_double("peak")) {
					m_peak = m_loudnessFilter->get_double("peak");
				}
				if (m_true_peak < m_loudnessFilter->get_double("true_peak")) {
					m_true_peak = m_loudnessFilter->get_double("true_peak");
				}
				m_newData = true;
			}
		}
	}

	// Update the time with every frame.
	QString const time = m_loudnessFilter->get_time("frames_processed");
	QMetaObject::invokeMethod(m_timeLabel, "setText", Qt::QueuedConnection, Q_ARG(const QString&, time));
}

auto AudioLoudnessScopeWidget::getTitle() -> QString {
	return tr("Audio Loudness");
}

void AudioLoudnessScopeWidget::setOrientation(Qt::Orientation orientation) {
	setOrientation(orientation, false);
}

void AudioLoudnessScopeWidget::setOrientation(Qt::Orientation orientation, bool force) {
	if (force || orientation != m_orientation) {
		if (orientation == Qt::Vertical) {
			// Calculate the minimum width
			int x = 0;
			constexpr int meterWidth = 54;
			if (Settings.loudnessScopeShowMeter("momentary"))
				x += meterWidth;
			if (Settings.loudnessScopeShowMeter("shortterm"))
				x += meterWidth;
			if (Settings.loudnessScopeShowMeter("integrated"))
				x += meterWidth;
			if (Settings.loudnessScopeShowMeter("range"))
				x += meterWidth;
			if (Settings.loudnessScopeShowMeter("peak"))
				x += meterWidth;
			if (Settings.loudnessScopeShowMeter("truepeak"))
				x += meterWidth;
			x = std::max(x, setMaxMeterWidthX);
			setMinimumSize(x, setMinimumSizeNumber);
			setMaximumSize(x, setMaximumSizeNumber);
		} else {
			// Calculate the minimum height
			int y = setYNumber;
			constexpr int meterHeight = 47;
			if (Settings.loudnessScopeShowMeter("momentary"))
				y += meterHeight;
			if (Settings.loudnessScopeShowMeter("shortterm"))
				y += meterHeight;
			if (Settings.loudnessScopeShowMeter("integrated"))
				y += meterHeight;
			if (Settings.loudnessScopeShowMeter("range"))
				y += meterHeight;
			if (Settings.loudnessScopeShowMeter("peak"))
				y += meterHeight;
			if (Settings.loudnessScopeShowMeter("truepeak"))
				y += meterHeight;
			y = std::max(y, setMaxMeterWidthY);
			setMinimumSize(setMinimumSizeNumber, y);
			setMaximumSize(setMaximumSizeNumber, y);
		}
		updateGeometry();
		m_orientation = orientation;
		resetQview();
	}
}

void AudioLoudnessScopeWidget::onResetButtonClicked() {
	m_loudnessFilter->set("reset", 1);
	m_timeLabel->setText("00:00:00:00");
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onIntegratedToggled(bool checked) {
	m_loudnessFilter->set("calc_program", checked);
	Settings.setLoudnessScopeShowMeter("integrated", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onShorttermToggled(bool checked) {
	m_loudnessFilter->set("calc_shortterm", checked);
	Settings.setLoudnessScopeShowMeter("shortterm", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onMomentaryToggled(bool checked) {
	m_loudnessFilter->set("calc_momentary", checked);
	Settings.setLoudnessScopeShowMeter("momentary", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onRangeToggled(bool checked) {
	m_loudnessFilter->set("calc_range", checked);
	Settings.setLoudnessScopeShowMeter("range", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onPeakToggled(bool checked) {
	m_loudnessFilter->set("calc_peak", checked);
	Settings.setLoudnessScopeShowMeter("peak", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::onTruePeakToggled(bool checked) {
	m_loudnessFilter->set("calc_true_peak", checked);
	Settings.setLoudnessScopeShowMeter("truepeak", checked);
	setOrientation(m_orientation, true);
	resetQview();
}

void AudioLoudnessScopeWidget::updateMeters() {
	if (!m_newData || !m_qview->rootObject())
		return;
	if (m_loudnessFilter->get_int("calc_program"))
		m_qview->rootObject()->setProperty("integrated", onedec(m_loudnessFilter->get_double("program")));
	if (m_loudnessFilter->get_int("calc_shortterm"))
		m_qview->rootObject()->setProperty("shortterm", onedec(m_loudnessFilter->get_double("shortterm")));
	if (m_loudnessFilter->get_int("calc_momentary"))
		m_qview->rootObject()->setProperty("momentary", onedec(m_loudnessFilter->get_double("momentary")));
	if (m_loudnessFilter->get_int("calc_range"))
		m_qview->rootObject()->setProperty("range", onedec(m_loudnessFilter->get_double("range")));
	if (m_loudnessFilter->get_int("calc_peak"))
		m_qview->rootObject()->setProperty("peak", onedec(m_peak));
	if (m_loudnessFilter->get_int("calc_true_peak"))
		m_qview->rootObject()->setProperty("truePeak", onedec(m_true_peak));
	m_peak      = setPeakNumber;
	m_true_peak = setPeakTrueNumber;
	m_newData   = false;
}

auto AudioLoudnessScopeWidget::event(QEvent* event) -> bool {
	const bool result = ScopeWidget::event(event);
	if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
		resetQview();
	}
	return result;
}

void AudioLoudnessScopeWidget::resetQview() {
	LOG_DEBUG() << "begin"
	            << "isSceneGraphInitialized" << m_qview->quickWindow()->isSceneGraphInitialized();
	if (!m_qview->quickWindow()->isSceneGraphInitialized()) {
		return;
	}
	if (m_qview->status() != QQuickWidget::Null) {
		m_qview->setSource(QUrl(""));
	}

	QDir viewPath = QmlUtilities::qmlDir();
	viewPath.cd("scopes");
	viewPath.cd("audioloudness");
	m_qview->engine()->addImportPath(viewPath.path());

	QDir modulePath = QmlUtilities::qmlDir();
	modulePath.cd("modules");
	m_qview->engine()->addImportPath(modulePath.path());

	m_qview->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_qview->quickWindow()->setColor(palette().window().color());
	QUrl const source = QUrl::fromLocalFile(viewPath.absoluteFilePath("audioloudnessscope.qml"));
	m_qview->setSource(source);

	if (m_qview->rootObject()) {
		m_qview->rootObject()->setProperty("orientation", m_orientation);
		m_qview->rootObject()->setProperty("enableIntegrated", Settings.loudnessScopeShowMeter("integrated"));
		m_qview->rootObject()->setProperty("enableShortterm", Settings.loudnessScopeShowMeter("shortterm"));
		m_qview->rootObject()->setProperty("enableMomentary", Settings.loudnessScopeShowMeter("momentary"));
		m_qview->rootObject()->setProperty("enableRange", Settings.loudnessScopeShowMeter("range"));
		m_qview->rootObject()->setProperty("enablePeak", Settings.loudnessScopeShowMeter("peak"));
		m_qview->rootObject()->setProperty("enableTruePeak", Settings.loudnessScopeShowMeter("truepeak"));
	}
}
