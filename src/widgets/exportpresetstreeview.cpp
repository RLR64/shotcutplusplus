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
#include "exportpresetstreeview.h"

// Qt
#include <qabstractitemmodel.h>
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#include <qtreeview.h>
#include <qwidget.h>

ExportPresetsTreeView::ExportPresetsTreeView(QWidget* parent) : QTreeView(parent) {
}

void ExportPresetsTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous) {
	Q_UNUSED(previous)
	emit activated(current);
}
