/*
 * Copyright (c) 2023-2024 Meltytech, LLC
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
#include "bitratedialog.hpp"
#include "dialogs/saveimagedialog.hpp"
#include "settings.hpp"

// Qt
#include <QDialogButtonBox>
#include <QJsonObject>
#include <QPushButton>
#include <QQueue>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QValueAxis>
#include <qdialog.h>
#include <qjsonarray.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qobject.h>
#include <qpainter.h>
#include <qwidget.h>

// STL
#include <cmath>
#include <limits>

// Minimum dialog dimensions
constexpr double bitsPerByte{8.0};
constexpr double bytesPerKilobit{1000.0};
constexpr int setMinimumSizeNumber{400};
constexpr int setMinimumSizeSecondNumber{200};
constexpr double subtractNumber{0.5};

// Chart axis configuration
constexpr double axisRangeStart{0.0};
constexpr int tickIntervalThreshold{100};
constexpr double tickIntervalLarge{10.0};
constexpr double tickIntervalSmall{5.0};

// Layout margins
constexpr int layoutMargin{0};
constexpr int layoutSpacing{8};

// Chart view dimensions
constexpr int chartMinWidth{1010};
constexpr int chartWidthPerPeriod{5};
constexpr int chartMinHeight{520};

// 1024x576 - default window size (16:9 ratio)
constexpr int dialogWidth{1024};
constexpr int dialogHeight{576};

static constexpr auto kSlidingWindowSize = 30;

BitrateDialog::BitrateDialog(const QString& resource, double fps, const QJsonArray& data, QWidget* parent)
    : QDialog(parent) {
	setMinimumSize(setMinimumSizeNumber, setMinimumSizeSecondNumber);
	setModal(true);
	setWindowTitle(tr("Bitrate Viewer"));
	setSizeGripEnabled(true);

	double time	= 0.0;
	constexpr double maxSize = 0.0;
	double firstTime = 0.0;
	double keySubtotal = 0.0;
	double interSubtotal = 0.0;
	double totalKbps = 0.0;
	double minKbps = std::numeric_limits<double>().max();
	double maxKbps = 0.0;
	int periodCount = 0;
	double previousSecond = 0.0;

	QQueue<double> window;
	auto barSeries = new QStackedBarSeries;
	auto averageLine = new QSplineSeries;
	QBarSet* interSet = nullptr;
	auto keySet = new QBarSet(fps > 0.0 ? "I" : tr("Audio"));

	barSeries->setBarWidth(1.0);
	if (fps > 0.0) {
		interSet = new QBarSet("P/B");
		barSeries->append(interSet);
	}
	barSeries->append(keySet);
	averageLine->setName(tr("Average"));

	for (int i = 0; i < data.size(); ++i) {
		auto o = data[i].toObject();
		auto pts = o["pts_time"].toString().toDouble();
		auto duration = o["duration_time"].toString().toDouble();
		auto size = o["size"].toString().toDouble() * bitsPerByte / bytesPerKilobit; // Kb

		if (pts > 0.0)
			time = pts + qMax(0.0, duration);
		if (i == 0)
			firstTime = time;
		time -= firstTime;
		if (o["flags"].toString()[0] == 'K') {
			keySubtotal += size;
		} else {
			interSubtotal += size;
		}
		totalKbps += size;

		// Every second as the period
		if (time >= (previousSecond + 1.0) || (i + 1) == data.size()) {
			// For the min, max, and overall average
			auto kbps = interSubtotal + keySubtotal;
			if (kbps < minKbps)
				minKbps = kbps;
			if (kbps > maxKbps)
				maxKbps = kbps;

			// Add a bar to the graph for each period
			const int n = qMax(1, int(time - previousSecond));
			for (auto j = 0; j < n; ++j) {
				if (interSet)
					interSet->append(interSubtotal);
				keySet->append(keySubtotal);
				++periodCount;
			}

			// For the smoothed average
			while (window.size() >= kSlidingWindowSize)
				window.dequeue();
			window.enqueue(kbps);
			double sum = 0.0;
			for (const auto& v : window)
				sum += v;
			// subtract 0.5 from the time because the X axis tick marks are centered
			// under the bar such that "0s" is actually at 0.5s
			averageLine-> append(time - subtractNumber, sum / window.size());

			// Reset counters
			interSubtotal = 0.0;
			keySubtotal = 0.0;
			previousSecond = std::floor(time);
		}
	}

	auto chart = new QChart();
	chart->addSeries(barSeries);
	chart->addSeries(averageLine);
	chart->setTheme(Settings.theme() == "dark" ? QChart::ChartThemeDark : QChart::ChartThemeLight);
	averageLine->setColor(Qt::yellow);
	chart->setTitle(tr("Bitrates for %1 ~~ Avg. %2 Min. %3 Max. %4 Kb/s")
	                    .arg(resource)
	                    .arg(qRound(totalKbps / time))
	                    .arg(qRound(minKbps))
	                    .arg(qRound(maxKbps)));

	auto axisX = new QValueAxis();
	chart->addAxis(axisX, Qt::AlignBottom);
	barSeries->attachAxis(axisX);
	averageLine->attachAxis(axisX);
	axisX->setRange(axisRangeStart, time);
	axisX->setLabelFormat("%.0f s");
	axisX->setTickType(QValueAxis::TicksDynamic);
	axisX->setTickInterval(periodCount > tickIntervalThreshold ? tickIntervalLarge : tickIntervalSmall);

	auto* axisY = new QValueAxis();
	chart->addAxis(axisY, Qt::AlignLeft);
	barSeries->attachAxis(axisY);
	averageLine->attachAxis(axisY);
	axisY->setRange(axisRangeStart, maxKbps);
	axisY->setLabelFormat("%.0f Kb/s");

	chart->legend()->setVisible(true);
	chart->legend()->setAlignment(Qt::AlignBottom);

	auto* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(layoutMargin, layoutMargin, layoutSpacing, layoutSpacing);
	layout->setSpacing(layoutSpacing);
	auto scrollArea = new QScrollArea(this);
	scrollArea->setWidget(chartView);
	layout->addWidget(scrollArea);
	auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close, this);
	buttons->button(QDialogButtonBox::Close)->setDefault(true);
	layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::accepted, this, [chartView, this] () -> void {
		QImage image(chartView->size(), QImage::Format_RGB32);
		QPainter painter(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		chartView->render(&painter);
		painter.end();
		SaveImageDialog(this, tr("Save Bitrate Graph"), image).exec();
	});
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	chartView->setMinimumWidth(qMax(chartMinWidth, periodCount * chartWidthPerPeriod));
	chartView->setMinimumHeight(chartMinHeight);
	resize(dialogWidth, dialogHeight);
	show();
}
