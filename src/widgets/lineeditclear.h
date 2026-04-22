/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
** Copyright (c) 2024 Meltytech, LLC
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef LINEEDIT_H
#define LINEEDIT_H

// Qt
#include <QLineEdit>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class QToolButton;

class LineEditClear : public QLineEdit {
	Q_OBJECT

  public:
	LineEditClear(QWidget* parent = nullptr);

  protected:
	void resizeEvent(QResizeEvent*) override;
	bool eventFilter(QObject* target, QEvent* event) override;

  private slots:
	void updateCloseButton(const QString& text);

  private:
	QToolButton* clearButton;
};

#endif // LIENEDIT_H
