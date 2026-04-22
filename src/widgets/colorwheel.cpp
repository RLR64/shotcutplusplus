/*
 * Copyright (c) 2013-2015 Meltytech, LLC
 * Author: Dan Dennedy <dan@dennedy.org>
 * Some ideas came from Qt-Plus: https://github.com/liuyanghejerry/Qt-Plus
 * and Steinar Gunderson's Movit demo app.
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
#include "colorwheel.h"

// Qt
#include <qbrush.h>
#include <qevent.h>
#include <qmath.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#include <qtypes.h>

// STl
#include <cmath>

ColorWheel::ColorWheel(QWidget* parent)
    : QWidget(parent), m_initialSize(300, 300), m_isMouseDown(false), m_margin(5), m_sliderWidth(30),
      m_color(Qt::white), m_isInWheel(false), m_isInSquare(false) {
	resize(m_initialSize);
	setMinimumSize(100, 100);
	setMaximumSize(m_initialSize);
	setCursor(Qt::CrossCursor);
}

auto ColorWheel::color() -> QColor {
	return m_color;
}

void ColorWheel::setColor(const QColor& color) {
	m_color = color;
}

auto ColorWheel::wheelSize() const -> int {
	return qMin(width() - m_sliderWidth, height());
}

auto ColorWheel::colorForPoint(const QPoint& point) -> QColor {
	if (!m_image.valid(point))
		return {};
	if (m_isInWheel) {
		const qreal w = wheelSize();
		const qreal xf = qreal(point.x()) / w;
		const qreal yf = 1.0 - qreal(point.y()) / w;
		const qreal xp = 2.0 * xf - 1.0;
		const qreal yp = 2.0 * yf - 1.0;
		const qreal rad = qMin(hypot(xp, yp), 1.0);
		qreal theta = qAtan2(yp, xp);
		theta -= 105.0 / 360.0 * 2.0 * M_PI;
		if (theta < 0.0)
			theta += 2.0 * M_PI;
		const qreal hue = (theta * 180.0 / M_PI) / 360.0;
		return QColor::fromHsvF(hue, rad, m_color.valueF());
	}
	if (m_isInSquare) {
		const qreal value = 1.0 - qreal(point.y() - m_margin) / (wheelSize() - m_margin * 2);
		return QColor::fromHsvF(m_color.hueF(), m_color.saturationF(), value);
	}
	return {};
}

auto ColorWheel::sizeHint() const -> QSize {
	return {height(), height()};
}

auto ColorWheel::minimumSizeHint() const -> QSize {
	return {100, 100};
}

void ColorWheel::mousePressEvent(QMouseEvent* event) {
	m_lastPoint = event->pos();
	if (m_wheelRegion.contains(m_lastPoint)) {
		m_isInWheel  = true;
		m_isInSquare = false;
		QColor const color = colorForPoint(m_lastPoint);
		changeColor(color);
	} else if (m_sliderRegion.contains(m_lastPoint)) {
		m_isInWheel  = false;
		m_isInSquare = true;
		QColor const color = colorForPoint(m_lastPoint);
		changeColor(color);
	}
	m_isMouseDown = true;
}

void ColorWheel::mouseMoveEvent(QMouseEvent* event) {
	m_lastPoint = event->pos();
	if (!m_isMouseDown)
		return;
	if (m_wheelRegion.contains(m_lastPoint) && m_isInWheel) {
		QColor const color = colorForPoint(m_lastPoint);
		changeColor(color);
	} else if (m_sliderRegion.contains(m_lastPoint) && m_isInSquare) {
		QColor const color = colorForPoint(m_lastPoint);
		changeColor(color);
	}
}

void ColorWheel::mouseReleaseEvent(QMouseEvent* event) {
	Q_UNUSED(event)
	m_isMouseDown = false;
	m_isInWheel   = false;
	m_isInSquare  = false;
}

void ColorWheel::resizeEvent(QResizeEvent* event) {
	m_image = QImage(event->size(), QImage::Format_ARGB32_Premultiplied);
	m_image.fill(palette().window().color().rgb());

	drawWheel();
	drawSlider();
	update();
}

void ColorWheel::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event)
	QPainter painter(this);
	//    QStyleOption opt;
	//    opt.init(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawImage(0, 0, m_image);
	drawWheelDot(painter);
	drawSliderBar(painter);
	//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ColorWheel::drawWheel() {
	const int r = wheelSize();
	QPainter painter(&m_image);
	painter.setRenderHint(QPainter::Antialiasing);
	m_image.fill(0); // transparent

	QConicalGradient conicalGradient;
	conicalGradient.setColorAt(0.0, Qt::red);
	conicalGradient.setColorAt(60.0 / 360.0, Qt::yellow);
	conicalGradient.setColorAt(135.0 / 360.0, Qt::green);
	conicalGradient.setColorAt(180.0 / 360.0, Qt::cyan);
	conicalGradient.setColorAt(240.0 / 360.0, Qt::blue);
	conicalGradient.setColorAt(315.0 / 360.0, Qt::magenta);
	conicalGradient.setColorAt(1.0, Qt::red);

	QRadialGradient radialGradient(0.0, 0.0, r / 2);
	radialGradient.setColorAt(0.0, Qt::white);
	radialGradient.setColorAt(1.0, Qt::transparent);

	painter.translate(r / 2, r / 2);
	painter.rotate(-105);

	QBrush const hueBrush(conicalGradient);
	painter.setPen(Qt::NoPen);
	painter.setBrush(hueBrush);
	painter.drawEllipse(QPoint(0, 0), r / 2 - m_margin, r / 2 - m_margin);

	QBrush const saturationBrush(radialGradient);
	painter.setBrush(saturationBrush);
	painter.drawEllipse(QPoint(0, 0), r / 2 - m_margin, r / 2 - m_margin);

	m_wheelRegion = QRegion(r / 2, r / 2, r - 2 * m_margin, r - 2 * m_margin, QRegion::Ellipse);
	m_wheelRegion.translate(-(r - 2 * m_margin) / 2, -(r - 2 * m_margin) / 2);
}

void ColorWheel::drawSlider() {
	QPainter painter(&m_image);
	painter.setRenderHint(QPainter::Antialiasing);
	const int ws = wheelSize();
	const qreal scale = qreal(ws + m_sliderWidth) / maximumWidth();
	const int w = m_sliderWidth * scale;
	const int h = ws - m_margin * 2;
	QLinearGradient gradient(0, 0, w, h);
	gradient.setColorAt(0.0, Qt::white);
	gradient.setColorAt(1.0, Qt::black);
	QBrush const brush(gradient);
	painter.setPen(Qt::NoPen);
	painter.setBrush(brush);
	painter.translate(ws, m_margin);
	painter.drawRect(0, 0, w, h);
	m_sliderRegion = QRegion(ws, m_margin, w, h);
}

void ColorWheel::drawWheelDot(QPainter& painter) {
	const int r = wheelSize() / 2;
	QPen pen(Qt::white);
	pen.setWidth(2);
	painter.setPen(pen);
	painter.setBrush(Qt::black);
	painter.translate(r, r);
	painter.rotate(360.0 - m_color.hue());
	painter.rotate(-105);
	//    r -= margin;
	painter.drawEllipse(QPointF(m_color.saturationF() * r, 0.0), 4, 4);
	painter.resetTransform();
}

void ColorWheel::drawSliderBar(QPainter& painter) {
	const qreal value = 1.0 - m_color.valueF();
	const int ws = wheelSize();
	const qreal scale = qreal(ws + m_sliderWidth) / maximumWidth();
	const int w = m_sliderWidth * scale;
	const int h = ws - m_margin * 2;
	QPen pen(Qt::white);
	pen.setWidth(2);
	painter.setPen(pen);
	painter.setBrush(Qt::black);
	painter.translate(ws, m_margin + value * h);
	painter.drawRect(0, 0, w, 4);
	painter.resetTransform();
}

void ColorWheel::changeColor(const QColor& color) {
	m_color = color;
	drawWheel();
	drawSlider();
	update();
	emit colorChanged(m_color);
}
