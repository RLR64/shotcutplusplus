/*
 * Copyright (c) 2013-2022 Meltytech, LLC
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
#include "colorwheelitem.hpp"
#include "mainwindow.hpp"

// Qt
#include <QCursor>
#include <QPainter>
#include <qbrush.h>
#include <qevent.h>
#include <qmath.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qquickitem.h>
#include <qquickpainteditem.h>
#include <qrgb.h>
#include <qtmetamacros.h>
#include <qtypes.h>

// STL
#include <cmath>
#include <cstdio>

static constexpr qreal WHEEL_SLIDER_RATIO = {10.0};

ColorWheelItem::ColorWheelItem(QQuickItem* parent)
    : QQuickPaintedItem(parent), m_image(), m_isMouseDown(false), m_lastPoint(0, 0), m_size(0, 0), m_margin(5),
      m_color(0, 0, 0, 0), m_isInWheel(false), m_isInSquare(false), m_step(1 / 256) {
	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptHoverEvents(true);
}

auto ColorWheelItem::color() -> QColor {
	return m_color;
}

void ColorWheelItem::setColor(const QColor& color) {
	if (m_color != color) {
		m_color = color;
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::red() -> int {
	return m_color.red();
}

void ColorWheelItem::setRed(int red) {
	if (m_color.red() != red) {
		m_color.setRed(red);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::green() -> int {
	return m_color.green();
}

void ColorWheelItem::setGreen(int green) {
	if (m_color.green() != green) {
		m_color.setGreen(green);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::blue() -> int {
	return m_color.blue();
}

void ColorWheelItem::setBlue(int blue) {
	if (m_color.blue() != blue) {
		m_color.setBlue(blue);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::redF() -> qreal {
	return m_color.redF();
}

void ColorWheelItem::setRedF(qreal red) {
	if (m_color.redF() != red) {
		m_color.setRedF(red);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::greenF() -> qreal {
	return m_color.greenF();
}

void ColorWheelItem::setGreenF(qreal green) {
	if (m_color.greenF() != green) {
		m_color.setGreenF(green);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::blueF() -> qreal {
	return m_color.blueF();
}

void ColorWheelItem::setBlueF(qreal blue) {
	if (m_color.blueF() != blue) {
		m_color.setBlueF(blue);
		update();
		emit colorChanged(m_color);
	}
}

auto ColorWheelItem::step() -> qreal {
	return m_step;
}

void ColorWheelItem::setStep(qreal step) {
	m_step = step;
}

auto ColorWheelItem::wheelSize() const -> int {
	const qreal ws = (qreal)width() / (1.0 + 1.0 / WHEEL_SLIDER_RATIO);
	return qMin(ws, height());
}

auto ColorWheelItem::colorForPoint(const QPoint& point) -> QColor {
	if (!m_image.valid(point))
		return {};
	if (m_isInWheel) {
		const qreal w = wheelSize() - m_margin * 2;
		const qreal xf = qreal(point.x() - m_margin) / w;
		const qreal yf = 1.0 - qreal(point.y() - m_margin) / w;
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

void ColorWheelItem::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		m_lastPoint = event->pos();
		if (m_wheelRegion.contains(m_lastPoint)) {
			m_isInWheel = true;
			m_isInSquare = false;
			QColor const color = colorForPoint(m_lastPoint);
			setColor(color);
		} else if (m_sliderRegion.contains(m_lastPoint)) {
			m_isInWheel  = false;
			m_isInSquare = true;
			QColor const color = colorForPoint(m_lastPoint);
			setColor(color);
		}
		m_isMouseDown = true;
	}
}

void ColorWheelItem::mouseMoveEvent(QMouseEvent* event) {
	updateCursor(event->pos());

	if (!m_isMouseDown)
		return;
	m_lastPoint = event->pos();
	if (m_wheelRegion.contains(m_lastPoint) && m_isInWheel) {
		QColor const color = colorForPoint(m_lastPoint);
		setColor(color);
	} else if (m_sliderRegion.contains(m_lastPoint) && m_isInSquare) {
		QColor const color = colorForPoint(m_lastPoint);
		setColor(color);
	}
}

void ColorWheelItem::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		m_isMouseDown = false;
		m_isInWheel = false;
		m_isInSquare = false;
	}
}

void ColorWheelItem::hoverMoveEvent(QHoverEvent* event) {
	updateCursor(event->position().toPoint());
}

void ColorWheelItem::wheelEvent(QWheelEvent* event) {
	QPoint const steps = event->angleDelta() / 8 / 15;
	qreal  const delta = (qreal)steps.y() * m_step;
	QColor currentColor = color();
	qreal c;

	// Increment/decrement RGB values by delta
	c = currentColor.redF();
	c += delta;
	if (c < 0)
		c = 0;
	if (c > 1)
		c = 1;
	currentColor.setRedF(c);

	c = currentColor.greenF();
	c += delta;
	if (c < 0)
		c = 0;
	if (c > 1)
		c = 1;
	currentColor.setGreenF(c);

	c = currentColor.blueF();
	c += delta;
	if (c < 0)
		c = 0;
	if (c > 1)
		c = 1;
	currentColor.setBlueF(c);

	setColor(currentColor);

	event->accept();
}

void ColorWheelItem::paint(QPainter* painter) {
	QSize const size(width(), height());

	if (m_size != size) {
		m_image = QImage(QSize(width(), height()), QImage::Format_ARGB32_Premultiplied);
		m_image.fill(qRgba(0, 0, 0, 0));
		drawWheel();
		drawSlider();
		m_size = size;
	}

	painter->setRenderHint(QPainter::Antialiasing);
	painter->drawImage(0, 0, m_image);
	drawWheelDot(*painter);
	drawSliderBar(*painter);
}

void ColorWheelItem::drawWheel() {
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

void ColorWheelItem::drawWheelDot(QPainter& painter) {
	const int r = wheelSize() / 2;
	QPen pen(Qt::white);
	pen.setWidth(2);
	painter.setPen(pen);
	painter.setBrush(Qt::black);
	painter.translate(r, r);
	painter.rotate(360.0 - m_color.hue());
	painter.rotate(-105);
	painter.drawEllipse(QPointF(m_color.saturationF() * r - m_margin, 0.0), 4, 4);
	painter.resetTransform();
}

void ColorWheelItem::drawSliderBar(QPainter& painter) {
	const qreal value = 1.0 - m_color.valueF();
	const int ws = wheelSize() * MAIN.devicePixelRatioF();
	const int w = (qreal)ws / WHEEL_SLIDER_RATIO;
	const int h = ws - m_margin * 2;
	QPen  pen(Qt::white);
	pen.setWidth(qRound(2 * MAIN.devicePixelRatioF()));
	painter.setPen(pen);
	painter.setBrush(Qt::black);
	painter.translate(ws, m_margin + value * h);
	painter.drawRect(0, 0, w, 4);
	painter.resetTransform();
}

void ColorWheelItem::drawSlider() {
	QPainter painter(&m_image);
	painter.setRenderHint(QPainter::Antialiasing);
	const int ws = wheelSize();
	const int w = (qreal)ws / WHEEL_SLIDER_RATIO;
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

void ColorWheelItem::updateCursor(const QPoint& pos) {
	if (m_wheelRegion.contains(pos) || m_sliderRegion.contains(pos)) {
		setCursor(QCursor(Qt::CrossCursor));
	} else {
		unsetCursor();
	}
}
