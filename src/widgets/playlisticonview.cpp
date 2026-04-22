/*
 * Copyright (c) 2016-2024 Meltytech, LLC
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
#include "playlisticonview.h"
#include "Logger.hpp"
#include "models/playlistmodel.hpp"
#include "settings.hpp"

// Qt
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QtMath>
#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qcontainerfwd.h>
#include <qimage.h>
#include <qitemselectionmodel.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qpaintdevice.h>
#include <qpalette.h>
#include <qregion.h>
#include <qtpreprocessorsupport.h>
#include <qwidget.h>

// Number constants
static constexpr int setPlaylistIconViewNumber{40};
static constexpr int setSingleStepNumber{100};
static constexpr int setPageStepNumber{400};
static constexpr int setGridSizeNumber{170};

static constexpr int kPaddingPx{10};
static constexpr float kFilesSizeFactor{1.5f};

PlaylistIconView::PlaylistIconView(QWidget* parent)
	: QAbstractItemView(parent), m_gridSize(setGridSizeNumber, setSingleStepNumber), m_draggingOverPos(QPoint()), m_itemsPerRow(3),
      m_iconRole(Qt::DecorationRole) {
	verticalScrollBar()->setSingleStep(setSingleStepNumber);
	verticalScrollBar()->setPageStep(setPageStepNumber);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&Settings, SIGNAL(playlistThumbnailsChanged()), SLOT(updateSizes()));
}

auto PlaylistIconView::_visualRect(const QModelIndex& index) const -> QRect {
	if (!index.isValid())
		return {};
	const int row = index.row() / m_itemsPerRow;
	const int col = index.row() % m_itemsPerRow;
	return {col * m_gridSize.width(), row * m_gridSize.height(), m_gridSize.width(), m_gridSize.height()};
}

auto PlaylistIconView::visualRect(const QModelIndex& /*index*/) const -> QRect {
	// TODO: this was causing a performance problem
	// return _visualRect(index);
	return {};
}

void PlaylistIconView::rowsInserted(const QModelIndex& parent, int start, int end) {
	QAbstractItemView::rowsInserted(parent, start, end);
	updateSizes();
}

void PlaylistIconView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
	QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
	updateSizes();
}

void PlaylistIconView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                   const QVector<int>& roles) {
	QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
	updateSizes();
}

void PlaylistIconView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	QAbstractItemView::selectionChanged(selected, deselected);
	viewport()->update();
}

void PlaylistIconView::scrollTo(const QModelIndex& index, ScrollHint hint) {
	Q_UNUSED(index);
	Q_UNUSED(hint);
}

auto PlaylistIconView::indexAt(const QPoint& point) const -> QModelIndex {
	if (!model())
		return {};

	if (point.x() / m_gridSize.width() >= m_itemsPerRow)
		return {};

	const int row = (point.y() + verticalScrollBar()->value()) / m_gridSize.height();
	const int col = (point.x() / m_gridSize.width()) % m_itemsPerRow;
	return model()->index(row * m_itemsPerRow + col, 0, rootIndex());
}

auto PlaylistIconView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) -> QModelIndex {
	Q_UNUSED(cursorAction);
	Q_UNUSED(modifiers);
	return {};
}

auto PlaylistIconView::horizontalOffset() const -> int {
	return 0;
}

auto PlaylistIconView::verticalOffset() const -> int {
	return 0;
}

auto PlaylistIconView::isIndexHidden(const QModelIndex& index) const -> bool {
	Q_UNUSED(index);
	return false;
}

void PlaylistIconView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) {
	QModelIndex topLeft;
	if (!selectionModel()->selectedIndexes().isEmpty()) {
		topLeft = selectionModel()->selectedIndexes().first();
	} else if (!m_isRangeSelect) {
		selectionModel()->select(indexAt(rect.topLeft()), command);
		return;
	}
	if (m_isToggleSelect) {
		command = QItemSelectionModel::Select;
		selectionModel()->select(indexAt(rect.bottomRight()), command);
		return;
	} else if (m_isRangeSelect && topLeft.isValid()) {
		QModelIndex const bottomRight = indexAt(rect.bottomRight());
		selectionModel()->select(QItemSelection(topLeft, bottomRight), command);
		return;
	} else if (topLeft.isValid()) {
		selectionModel()->select(indexAt(rect.topLeft()), command);
		return;
	}
	m_pendingSelect = indexAt(rect.topLeft());
}

auto PlaylistIconView::visualRegionForSelection(const QItemSelection& selection) const -> QRegion {
	Q_UNUSED(selection);
	return {};
}

void PlaylistIconView::currentChanged(const QModelIndex& current, const QModelIndex& previous) {
	viewport()->update();
	QAbstractItemView::currentChanged(current, previous);
}

void PlaylistIconView::paintEvent(QPaintEvent*) {
	QPainter painter(viewport());
	QPalette const pal(palette());
	const auto proxy = tr("P", "The first letter or symbol of \"proxy\"");
	const auto oldFont = painter.font();
	auto boldFont(oldFont);

	painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	boldFont.setBold(true);
	painter.fillRect(rect(), pal.base());

	if (!model())
		return;

	auto  proxyModel = dynamic_cast<QSortFilterProxyModel*>(model());
	QRect dragIndicator;

	for (int row = 0; row <= proxyModel->rowCount(rootIndex()) / m_itemsPerRow; row++) {
		for (int col = 0; col < m_itemsPerRow; col++) {
			const int rowIdx = row * m_itemsPerRow + col;

			QModelIndex const idx = proxyModel->index(rowIdx, 0, rootIndex());
			if (!idx.isValid())
				break;

			QRect const itemRect(col * m_gridSize.width(), row * m_gridSize.height() - verticalScrollBar()->value(),
			               m_gridSize.width(), m_gridSize.height());

			if (itemRect.bottom() < 0 || itemRect.top() > this->height())
				continue;

			const bool selected = selectedIndexes().contains(idx);
			auto thumb = proxyModel->mapToSource(idx).data(m_iconRole).value<QImage>();

			if (m_iconRole != Qt::DecorationRole) { // Files
				thumb = thumb.scaled(PlaylistModel::THUMBNAIL_WIDTH * kFilesSizeFactor,
				                     PlaylistModel::THUMBNAIL_HEIGHT * kFilesSizeFactor, Qt::KeepAspectRatio,
				                     Qt::SmoothTransformation);
			}

			QRect imageBoundingRect = itemRect;
			imageBoundingRect.setHeight(0.7 * imageBoundingRect.height());
			imageBoundingRect.adjust(0, kPaddingPx, 0, 0);

			QRect imageRect(QPoint(), thumb.size());
			imageRect.moveCenter(imageBoundingRect.center());

			QRect textRect = itemRect;
			textRect.setTop(imageBoundingRect.bottom());
			textRect.adjust(3, 0, -3, 0);

			QRect const buttonRect = itemRect.adjusted(2, 2, -2, -2);

			if (selected) {
				painter.fillRect(buttonRect, pal.highlight());
			} else {
				painter.fillRect(buttonRect, pal.button());

				painter.setPen(pal.color(QPalette::Button).lighter());
				painter.drawLine(buttonRect.topLeft(), buttonRect.topRight());
				painter.drawLine(buttonRect.topLeft(), buttonRect.bottomLeft());

				painter.setPen(pal.color(QPalette::Button).darker());
				painter.drawLine(buttonRect.topRight(), buttonRect.bottomRight());
				painter.drawLine(buttonRect.bottomLeft(), buttonRect.bottomRight());
			}

			painter.drawImage(imageRect, thumb);
			QStringList nameParts = proxyModel->mapToSource(idx).data(Qt::DisplayRole).toString().split('\n');
			if (nameParts.size() > 1) {
				const auto indexPos = imageRect.topLeft() + QPoint(5, 15);
				painter.setFont(boldFont);
				painter.setPen(pal.color(QPalette::Dark).darker());
				painter.drawText(indexPos, proxy);
				painter.setPen(pal.color(QPalette::WindowText));
				painter.drawText(indexPos - QPoint(1, 1), proxy);
				painter.setFont(oldFont);
			}
			painter.setPen(pal.color(QPalette::WindowText));
			painter.drawText(textRect, Qt::AlignCenter,
			                 painter.fontMetrics().elidedText(nameParts.first(), Qt::ElideMiddle, textRect.width()));

			if (!m_draggingOverPos.isNull() && itemRect.contains(m_draggingOverPos)) {
				QAbstractItemView::DropIndicatorPosition const dropPos = position(m_draggingOverPos, itemRect, idx);
				dragIndicator.setSize(QSize(4, itemRect.height()));
				if (dropPos == QAbstractItemView::AboveItem)
					dragIndicator.moveTopLeft(itemRect.topLeft() - QPoint(dragIndicator.width() / 2, 0));
				else
					dragIndicator.moveTopLeft(itemRect.topRight() - QPoint(dragIndicator.width() / 2 - 1, 0));
			}
		}
	}
	if (!dragIndicator.isNull()) {
		painter.fillRect(dragIndicator, pal.buttonText());
	}
}

void PlaylistIconView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		if (m_draggingOverPos.isNull() && m_pendingSelect.isValid()) {
			selectionModel()->select(m_pendingSelect, QItemSelectionModel::ClearAndSelect);
			viewport()->update();
		}
		m_pendingSelect = QModelIndex();
	}
	QAbstractItemView::mouseReleaseEvent(event);
}

void PlaylistIconView::dragMoveEvent(QDragMoveEvent* e) {
	m_draggingOverPos = e->position().toPoint();
	QAbstractItemView::dragMoveEvent(e);
}

void PlaylistIconView::dragLeaveEvent(QDragLeaveEvent* e) {
	m_draggingOverPos = QPoint();
	QAbstractItemView::dragLeaveEvent(e);
}

void PlaylistIconView::dropEvent(QDropEvent* event) {
	m_draggingOverPos = QPoint();

	QModelIndex index           = indexAt(event->position().toPoint());
	QRect const rectAtDropPoint = _visualRect(index);

	QAbstractItemView::DropIndicatorPosition const dropPos = position(event->position().toPoint(), rectAtDropPoint, index);
	if (dropPos == QAbstractItemView::BelowItem)
		index = index.sibling(index.row() + 1, index.column());

	const Qt::DropAction action = event->dropAction();
	const int row = (index.row() != -1) ? index.row() : model()->rowCount(rootIndex());
	if (model()->dropMimeData(event->mimeData(), action, row, index.column(), index))
		event->acceptProposedAction();

	stopAutoScroll();
	setState(NoState);
	viewport()->update();
}

void PlaylistIconView::resizeEvent(QResizeEvent* event) {
	updateSizes();
	QAbstractItemView::resizeEvent(event);
}

void PlaylistIconView::setModel(QAbstractItemModel* model) {
	QAbstractItemView::setModel(model);
	updateSizes();
}

void PlaylistIconView::keyPressEvent(QKeyEvent* event) {
	QAbstractItemView::keyPressEvent(event);
	event->ignore();
	m_isToggleSelect = (event->modifiers() & Qt::ControlModifier);
	m_isRangeSelect  = (event->modifiers() & Qt::ShiftModifier);
}

void PlaylistIconView::keyReleaseEvent(QKeyEvent* event) {
	QAbstractItemView::keyPressEvent(event);
	event->ignore();
	resetMultiSelect();
}

auto PlaylistIconView::position(const QPoint& pos, const QRect& rect, const QModelIndex& index) const -> QAbstractItemView::DropIndicatorPosition {
	Q_UNUSED(index);
	if (pos.x() < rect.center().x())
		return QAbstractItemView::AboveItem;
	else
		return QAbstractItemView::BelowItem;
}

void PlaylistIconView::updateSizes() {
	if (!model() || !model()->rowCount(rootIndex())) {
		verticalScrollBar()->setRange(0, 0);
		return;
	}

	QSize size;
	if (m_iconRole != Qt::DecorationRole) // Files
		size = QSize(PlaylistModel::THUMBNAIL_WIDTH * kFilesSizeFactor,
		             PlaylistModel::THUMBNAIL_HEIGHT * kFilesSizeFactor);
	else if (Settings.playlistThumbnails() == "tall")
		size = QSize(PlaylistModel::THUMBNAIL_WIDTH, PlaylistModel::THUMBNAIL_HEIGHT * 2);
	else if (Settings.playlistThumbnails() == "large")
		size = QSize(PlaylistModel::THUMBNAIL_WIDTH * 2, PlaylistModel::THUMBNAIL_HEIGHT * 2);
	else if (Settings.playlistThumbnails() == "wide")
		size = QSize(PlaylistModel::THUMBNAIL_WIDTH * 2, PlaylistModel::THUMBNAIL_HEIGHT);
	else
		size = QSize(PlaylistModel::THUMBNAIL_WIDTH, PlaylistModel::THUMBNAIL_HEIGHT);

	size.setWidth(size.width() + kPaddingPx);

	m_itemsPerRow = qMax(1, viewport()->width() / size.width());
	m_gridSize = QSize(viewport()->width() / m_itemsPerRow, size.height() + setPlaylistIconViewNumber);

	if (!verticalScrollBar())
		return;

	verticalScrollBar()->setRange(0, m_gridSize.height() * model()->rowCount(rootIndex()) / m_itemsPerRow - height() +
	                                     m_gridSize.height());
	viewport()->update();
}

void PlaylistIconView::resetMultiSelect() {
	m_isToggleSelect = false;
	m_isRangeSelect  = false;
}

void PlaylistIconView::setIconRole(int role) {
	m_iconRole = role;
}
