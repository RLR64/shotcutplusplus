/*
 * Copyright (c) 2017-2024 Meltytech, LLC
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

#ifndef TRANSCODEDIALOG_HPP
#define TRANSCODEDIALOG_HPP

// Qt
#include <QDialog>
#include <qobject.h>
#include <qtmetamacros.h>

namespace Ui {
class TranscodeDialog;
}

class TranscodeDialog : public QDialog {
	Q_OBJECT

  public:
	explicit TranscodeDialog(const QString& message, bool isProgressive, QWidget* parent = nullptr);
	~TranscodeDialog() override;

	[[nodiscard]] constexpr auto format() -> int const {
		return m_format;
	}

	void showCheckBox();

	[[nodiscard]] auto isCheckBoxChecked() -> bool const {
		return m_isChecked;
	}

	[[nodiscard]] auto    deinterlace() const -> bool;
	[[nodiscard]] auto    fpsOverride() const -> bool;
	[[nodiscard]] auto  fps() const -> double;
	[[nodiscard]] auto frc() const -> QString;
	auto    get709Convert() -> bool;
	void    set709Convert(bool enable);
	[[nodiscard]] auto sampleRate() const -> QString;
	void    showSubClipCheckBox();
	[[nodiscard]] auto    isSubClip() const -> bool;
	void    setSubClipChecked(bool checked);
	void    setFrameRate(double fps);

  private slots:
	void on_horizontalSlider_valueChanged(int position);

	void on_checkBox_clicked(bool checked);

	void on_advancedCheckBox_clicked(bool checked);

  private:
	Ui::TranscodeDialog* ui;
	int                  m_format;
	bool                 m_isChecked;
	bool                 m_isProgressive;
};

#endif // TRANSCODEDIALOG_HPP
