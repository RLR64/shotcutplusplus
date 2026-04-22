/*
 * Copyright (c) 2014-2026 Meltytech, LLC
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
#include "util.hpp"
#include "FlatpakWrapperGenerator.hpp"
#include "Logger.hpp"
#include "dialogs/transcodedialog.hpp"
#include "mainwindow.hpp"
#include "mltcontroller.hpp"
#include "proxymanager.hpp"
#include "qmltypes/qmlapplication.hpp"
#include "settings.hpp"
#include "shotcut_mlt_properties.hpp"
#include "transcoder.hpp"

// Qt
#include <MltChain.h>
#include <MltProducer.h>
#include <MltProperties.h>
#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QMap>
#include <QMediaDevices>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>
#include <QWidget>
#include <QtGlobal>
#include <cstdint>
#include <framework/mlt_types.h>
#include <functional>
#include <limits>
#include <qbytearrayalgorithms.h>
#include <qcolordialog.h>
#include <qcontainerfwd.h>
#include <qfiledialog.h>
#include <qforeach.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qobject.h>
#include <qoverload.h>
#include <qpair.h>
#include <qscopedpointer.h>
#include <qtypes.h>

// STL
#include <climits>
#include <cmath>
#include <memory>
#include <utility>
#include <sysinfoapi.h>

// Number constants
// File hash
static constexpr qint64 kHashBlockSize{1000000};      // 1 MB
static constexpr qint64 kHashFileSizeLimit{1000000 * 2};  // 2 MB

// FPS
static constexpr double  kFpsMultiplier6{1000000.0};
static constexpr double  kFpsMultiplier5{100000.0};

static constexpr int kFps2398Rounded{23976024};
static constexpr int kFps2997Rounded{2997003};
static constexpr int kFps4795Rounded{47952048};
static constexpr int kFps5994Rounded{5994006};

static constexpr int kFps2398Numerator{24000};
static constexpr int kFps2997Numerator{30000};
static constexpr int kFps4795Numerator{48000};
static constexpr int kFps5994Numerator{60000};
static constexpr int kFpsDenominator1001{1001};
static constexpr int kFpsPrecision{1000000};

// Bytes available
static constexpr int setBytesAvailableNumber{1024};

// clang-format off
#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_MAC
static constexpr unsigned int kLowMemoryThresholdPercent{10U};
#else
static constexpr unsigned int kLowMemoryThresholdKB{256U * 1024U};
#endif
static constexpr qint64 kFreeSpaceThesholdGB{25LL * 1024 * 1024 * 1024};
// clang-format on

auto Util::baseName(const QString& filePath, bool trimQuery) -> QString {
	QString s = filePath;
	// Only if absolute path and not a URI.
	if (s.startsWith('/') || s.mid(1, 2) == ":/" || s.mid(1, 2) == ":\\")
		s = QFileInfo(s).fileName();
	if (trimQuery) {
		return removeQueryString(s);
	}
	return s;
}

void Util::setColorsToHighlight(QWidget* widget, QPalette::ColorRole role) {
	if (role == QPalette::Base) {
		widget->setStyleSheet("QLineEdit {"
		                      "font-weight: bold;"
		                      "background-color: palette(highlight);"
		                      "color: palette(highlighted-text);"
		                      "selection-background-color: palette(alternate-base);"
		                      "selection-color: palette(text);"
		                      "}"
		                      "QLineEdit:hover {"
		                      "border: 2px solid palette(button-text);"
		                      "}");
	} else {
		QPalette palette = QApplication::palette();
		palette.setColor(role, palette.color(palette.Highlight));
		palette.setColor(role == QPalette::Button ? QPalette::ButtonText : QPalette::WindowText,
		                 palette.color(palette.HighlightedText));
		widget->setPalette(palette);
		widget->setAutoFillBackground(true);
	}
}

void Util::showInFolder(const QString& path) {
	QFileInfo const info(removeQueryString(path));
#if defined(Q_OS_WIN)
	QStringList args;
	if (!info.isDir())
		args << "/select,";
	args << QDir::toNativeSeparators(path);
	if (QProcess::startDetached("explorer", args))
		return;
#elif defined(Q_OS_MAC)
	QStringList args;
	args << "-e";
	args << "tell application \"Finder\"";
	args << "-e";
	args << "activate";
	args << "-e";
	args << "select POSIX file \"" + path + "\"";
	args << "-e";
	args << "end tell";
#if !defined(QT_DEBUG)
	args << "-e";
	args << "return";
#endif
	if (!QProcess::execute("/usr/bin/osascript", args))
		return;
#endif
	Util::openUrl(QUrl::fromLocalFile(info.isDir() ? path : info.path()));
}

auto Util::warnIfNotWritable(const QString& filePath, QWidget* parent, const QString& caption) -> bool {
	// Returns true if not writable.
	if (!filePath.isEmpty() && !filePath.contains("://")) {
		QFileInfo info(filePath);
		if (!info.isDir()) {
			info = QFileInfo(info.dir().path());
		}
		if (!info.isWritable()) {
			info = QFileInfo(filePath);
			QMessageBox::warning(parent, caption,
			                     QObject::tr("Unable to write file %1\n"
			                                 "Perhaps you do not have permission.\n"
			                                 "Try again with a different folder.")
			                         .arg(info.fileName()));
			return true;
		}
	}
	return false;
}

auto Util::producerTitle(const Mlt::Producer& producer) -> QString {
	QString result;
	auto& p = const_cast<Mlt::Producer&>(producer);
	if (!p.is_valid() || p.is_blank())
		return result;
	if (p.get(kShotcutTransitionProperty))
		return QObject::tr("Transition");
	if (p.get(kTrackNameProperty))
		return QObject::tr("Track: %1").arg(QString::fromUtf8(p.get(kTrackNameProperty)));
	if (mlt_service_tractor_type == p.type())
		return QObject::tr("Output");
	if (p.get(kShotcutCaptionProperty))
		return QString::fromUtf8(p.get(kShotcutCaptionProperty));
	return Util::baseName(ProxyManager::resource(p));
}

auto Util::removeFileScheme(QUrl& url, bool fromPercentEncoding) -> QString {
	QString path = url.url();
	if (url.scheme() == "file")
		path = url.toString(QUrl::PreferLocalFile);
	if (fromPercentEncoding)
		return QUrl::fromPercentEncoding(path.toUtf8());
	return path;
}

static inline auto isValidGoProFirstFilePrefix(const QFileInfo& info) -> bool {
	QStringList const list{"GOPR", "GH01", "GL01", "GM01", "GS01", "GX01"};
	return list.contains(info.baseName().left(4).toUpper());
}

static inline auto isValidGoProPrefix(const QFileInfo& info) -> bool {
	QStringList const list{"GP", "GH", "GL", "GM", "GS", "GX"};
	return list.contains(info.baseName().left(2).toUpper());
}

static inline auto isValidGoProSuffix(const QFileInfo& info) -> bool {
	QStringList const list{"MP4", "LRV", "360", "WAV"};
	return list.contains(info.suffix().toUpper());
}

auto Util::sortedFileList(const QList<QUrl>& urls) -> const QStringList {
	QStringList result;
	QMap<QString, QStringList> goproFiles;

	// First look for GoPro main files.
	foreach (QUrl url, urls) {
		QFileInfo const fi(removeFileScheme(url, false));
		if (fi.baseName().size() == 8 && isValidGoProSuffix(fi) && isValidGoProFirstFilePrefix(fi)) {
			goproFiles[fi.baseName().mid(4)] << fi.filePath();
		}
	}
	// Then, look for GoPro split files.
	foreach (QUrl url, urls) {
		QFileInfo const fi(removeFileScheme(url, false));
		if (fi.baseName().size() == 8 && isValidGoProSuffix(fi) && isValidGoProPrefix(fi) &&
		    !isValidGoProFirstFilePrefix(fi)) {
			QString const goproNumber = fi.baseName().mid(4);
			// Only if there is a matching main GoPro file.
			if (goproFiles.contains(goproNumber) && goproFiles[goproNumber].size()) {
				goproFiles[goproNumber] << fi.filePath();
			}
		}
	}
	// Next, sort the GoPro files.
	auto keys = goproFiles.keys();
	for (auto& goproNumber : keys) {
		goproFiles[goproNumber].sort(Qt::CaseSensitive);
	}
	// Finally, build the list of all files.
	// Add all the GoPro files first.
	for (auto& paths : goproFiles) {
		result << paths;
	}
	// Add all the non-GoPro files.
	for (auto url : urls) {
		QFileInfo const fi(removeFileScheme(url, false));
		if (fi.baseName().size() == 8 && isValidGoProSuffix(fi) &&
		    (isValidGoProFirstFilePrefix(fi) || isValidGoProPrefix(fi))) {
			QString const goproNumber = fi.baseName().mid(4);
			if (goproFiles.contains(goproNumber) && goproFiles[goproNumber].contains(fi.filePath()))
				continue;
		}
		result << fi.filePath();
	}
	return result;
}

auto Util::coerceMultiple(int value, int multiple) -> int {
	return (value + multiple - 1) / multiple * multiple;
}

auto Util::expandDirectories(const QList<QUrl>& urls) -> QList<QUrl> {
	QList<QUrl> result;
	foreach (QUrl url, urls) {
		QString const path = Util::removeFileScheme(url, false);
		QFileInfo const fi(path);
		if (fi.isDir()) {
			QDir const dir(path);
			foreach (QFileInfo const fi, dir.entryInfoList(QDir::Files | QDir::Readable, QDir::Name))
				result << fi.filePath();
		} else {
			result << url;
		}
	}
	return result;
}

auto Util::isDecimalPoint(QChar ch) -> bool {
	// See https://en.wikipedia.org/wiki/Decimal_separator#Unicode_characters
	return ch == '.' || ch == ',' || ch == '\'' || ch == ' ' || ch == QChar(0x00B7) || ch == QChar(0x2009) ||
	       ch == QChar(0x202F) || ch == QChar(0x02D9) || ch == QChar(0x066B) || ch == QChar(0x066C) ||
	       ch == QChar(0x2396);
}

auto Util::isNumeric(QString& str) -> bool {
	for (const auto ch : str) {
			if (ch != '+' && ch != '-' && ch.toLower() != 'e' && !isDecimalPoint(ch) && !ch.isDigit())
			return false;
	}
	return true;
}

auto Util::convertNumericString(QString& str, QChar decimalPoint) -> bool {
	// Returns true if the string was changed.
	bool result = false;
	if (isNumeric(str)) {
		for (auto ch : std::as_const(str)) {
				if (ch != decimalPoint && isDecimalPoint(ch)) {
				ch = decimalPoint;
				result = true;
			}
		}
	}
	return result;
}

auto Util::convertDecimalPoints(QString& str, QChar decimalPoint) -> bool {
	// Returns true if the string was changed.
	bool result = false;
	if (!str.contains(decimalPoint)) {
		for (auto ch : str) {
				// Space is used as a delimiter for rect fields and possibly elsewhere.
			if (ch != decimalPoint && ch != ' ' && isDecimalPoint(ch)) {
				ch     = decimalPoint;
				result = true;
			}
		}
	}
	return result;
}

void Util::showFrameRateDialog(const QString& caption, int numerator, QDoubleSpinBox* spinner, QWidget* parent) {
	const double fps = numerator / 1001.0;
	QMessageBox dialog(QMessageBox::Question, caption,
	                   QObject::tr("The value you entered is very similar to the common,\n"
	                               "more standard %1 = %2/1001.\n\n"
	                               "Do you want to use %1 = %2/1001 instead?")
	                       .arg(fps, 0, 'f', 6)
	                       .arg(numerator),
	                   QMessageBox::No | QMessageBox::Yes, parent);
	dialog.setDefaultButton(QMessageBox::Yes);
	dialog.setEscapeButton(QMessageBox::No);
	dialog.setWindowModality(QmlApplication::dialogModality());
	if (dialog.exec() == QMessageBox::Yes) {
		spinner->setValue(fps);
	}
}

auto Util::writableTemporaryFile(const QString& filePath, const QString& templateName) -> QTemporaryFile* {
	// filePath should already be checked writable.
	QFileInfo const info(filePath);
	QString const templateFileName =
	    templateName.isEmpty() ? QStringLiteral("%1.XXXXXX").arg(QCoreApplication::applicationName()) : templateName;

	// First, try the system temp dir.
	QString const templateFilePath = QDir(QDir::tempPath()).filePath(templateFileName);
	std::unique_ptr<QTemporaryFile> tmp(new QTemporaryFile(templateFilePath));

	if (!tmp->open() || tmp->write("") < 0) {
		// Otherwise, use the directory provided.
		return new QTemporaryFile(info.dir().filePath(templateFileName));
	} else {
		return tmp.release();
	}
}

void Util::applyCustomProperties(Mlt::Producer& destination, Mlt::Producer& source, int in, int out) {
	Mlt::Properties p(destination);
	p.clear("force_progressive");
	p.clear("force_tff");
	p.clear("force_aspect_ratio");
	p.clear("video_delay");
	p.clear("color_range");
	p.clear("speed");
	p.clear("warp_speed");
	p.clear("warp_pitch");
	p.clear("rotate");
	p.clear(kAspectRatioNumerator);
	p.clear(kAspectRatioDenominator);
	p.clear(kCommentProperty);
	p.clear(kShotcutProducerProperty);
	p.clear(kDefaultAudioIndexProperty);
	p.clear(kOriginalInProperty);
	p.clear(kOriginalOutProperty);
	if (!p.get_int(kIsProxyProperty))
		p.clear(kOriginalResourceProperty);
	destination.pass_list(source,
	                      "mlt_service, audio_index, video_index, astream, vstream, force_progressive, force_tff,"
	                      "force_aspect_ratio, video_delay, color_range, warp_speed, warp_pitch, "
	                      "rotate," kAspectRatioNumerator "," kAspectRatioDenominator "," kCommentProperty
	                      "," kShotcutProducerProperty "," kDefaultAudioIndexProperty "," kOriginalInProperty
	                      "," kOriginalOutProperty "," kOriginalResourceProperty "," kDisableProxyProperty
	                      "," kShotcutBinsProperty);
	if (!destination.get("_shotcut:resource")) {
		destination.set("_shotcut:resource", destination.get("resource"));
		destination.set("_shotcut:length", destination.get("length"));
	}
	QString resource = ProxyManager::resource(destination);
	if (!qstrcmp("timewarp", source.get("mlt_service"))) {
		auto speed   = qAbs(source.get_double("warp_speed"));
		auto caption = QStringLiteral("%1 (%2x)").arg(Util::baseName(resource, true)).arg(speed);
		destination.set(kShotcutCaptionProperty, caption.toUtf8().constData());

		resource = destination.get("_shotcut:resource");
		destination.set("warp_resource", resource.toUtf8().constData());
		resource = QStringLiteral("%1:%2:%3").arg("timewarp", source.get("warp_speed"), resource);
		destination.set("resource", resource.toUtf8().constData());
		const double speedRatio = 1.0 / speed;
		const int length = qRound(destination.get_length() * speedRatio);
		destination.set("length", destination.frames_to_time(length, mlt_time_clock));
	} else {
		auto caption = Util::baseName(resource, true);
		destination.set(kShotcutCaptionProperty, caption.toUtf8().constData());

		p.clear("warp_resource");
		destination.set("resource", destination.get("_shotcut:resource"));
		destination.set("length", destination.get("_shotcut:length"));
	}
	destination.set_in_and_out(in, out);
}

auto Util::getFileHash(const QString& path) -> QString {
	// This routine is intentionally copied from Kdenlive.
	QFile file(removeQueryString(path));
	if (file.open(QIODevice::ReadOnly)) {
		QByteArray fileData;
		// 1 MB = 1 second per 450 files (or faster)
		// 10 MB = 9 seconds per 450 files (or faster)
		if (file.size() > kHashFileSizeLimit) {
			fileData = file.read(kHashBlockSize);
			if (file.seek(file.size() - kHashBlockSize))
				fileData.append(file.readAll());
		} else {
			fileData = file.readAll();
		}
		file.close();
		return QCryptographicHash::hash(fileData, QCryptographicHash::Md5).toHex();
	}
	return {};
}

auto Util::getHash(Mlt::Properties& properties) -> QString {
	QString hash = properties.get(kShotcutHashProperty);
	if (hash.isEmpty()) {
		QString const service = properties.get("mlt_service");
		QString resource = QString::fromUtf8(properties.get("resource"));

		if (properties.get_int(kIsProxyProperty) && properties.get(kOriginalResourceProperty))
			resource = QString::fromUtf8(properties.get(kOriginalResourceProperty));
		else if (service == "timewarp")
			resource = QString::fromUtf8(properties.get("warp_resource"));
		else if (service == "vidstab")
			resource = QString::fromUtf8(properties.get("filename"));
		hash = getFileHash(resource);
		if (!hash.isEmpty())
			properties.set(kShotcutHashProperty, hash.toLatin1().constData());
	}
	return hash;
}

auto Util::hasDriveLetter(const QString& path) -> bool {
	auto driveSeparators = path.mid(1, 2);
	return driveSeparators == ":/" || driveSeparators == ":\\";
}

auto Util::getColorDialogOptions() -> QColorDialog::ColorDialogOptions {
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	return QColorDialog::DontUseNativeDialog;
#endif
	return {};
}

auto Util::getFileDialogOptions() -> QFileDialog::Options {
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	if (qEnvironmentVariableIsSet("SNAP")) {
		return QFileDialog::DontUseNativeDialog;
	}
#endif
	return {};
}

auto Util::isMemoryLow() -> bool {
#if defined(Q_OS_WIN)
	unsigned int availableKB = UINT_MAX;
	MEMORYSTATUSEX memory_status;
	ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
	memory_status.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&memory_status)) {
		availableKB = memory_status.ullAvailPhys / 1024UL;
	}
	LOG_INFO() << "available RAM = " << availableKB << "KB";
	return availableKB < kLowMemoryThresholdKB;
#elif defined(Q_OS_MAC)
	QProcess p;
	p.start("memory_pressure", QStringList());
	p.waitForFinished();
	auto lines = p.readAllStandardOutput();
	p.close();
	for (auto& line : lines.split('\n')) {
		if (line.startsWith("System-wide memory free")) {
			const auto fields = line.split(':');
			for (auto s : fields) {
				bool ok         = false;
				auto percentage = s.replace('%', "").toUInt(&ok);
				if (ok) {
					LOG_INFO() << percentage << '%';
					return percentage <= kLowMemoryThresholdPercent;
				}
			}
		}
	}
	return false;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	QProcess p;
	p.start("sysctl -n hw.usermem");
	p.waitForFinished();
	auto lines = p.readAllStandardOutput();
	p.close();
	bool ok          = false;
	auto availableKB = lines.toUInt(&ok);
	if (ok) {
		return availableKB < kLowMemoryThresholdKB;
	}

	return false;
#elif defined(Q_OS_LINUX)
	unsigned int availableKB = UINT_MAX;
	QFile        meminfo("/proc/meminfo");
	if (meminfo.open(QIODevice::ReadOnly)) {
		for (auto line = meminfo.readLine(setBytesAvailableNumber); availableKB == UINT_MAX && !line.isEmpty();
			 line      = meminfo.readLine(setBytesAvailableNumber)) {
			if (line.startsWith("MemAvailable")) {
				const auto& fields = line.split(' ');
				for (const auto& s : fields) {
					bool ok = false;
					auto kB = s.toUInt(&ok);
					if (ok) {
						availableKB = kB;
						break;
					}
				}
			}
		}
	}
	meminfo.close();
	LOG_INFO() << "available RAM = " << availableKB << "KB";
	return availableKB < kLowMemoryThresholdKB;
#endif
}

auto Util::removeQueryString(const QString& s) -> QString {
	auto i = s.lastIndexOf("\\?");
	if (i < 0) {
		i = s.lastIndexOf("%5C?");
	}
	if (i > 0) {
		return s.left(i);
	}
	return s;
}

auto Util::greatestCommonDivisor(int m, int n) -> int {
	int gcd, remainder;
	while (n) {
		remainder = m % n;
		m         = n;
		n         = remainder;
	}
	gcd = m;
	return gcd;
}

void Util::normalizeFrameRate(double fps, int& numerator, int& denominator) {
	// Convert some common non-integer frame rates to fractions.
	if (qRound(fps * kFpsMultiplier6) == kFps2398Rounded) {
		numerator   = kFps2398Numerator;
		denominator = kFpsDenominator1001;
	} else if (qRound(fps * kFpsMultiplier5) == kFps2997Rounded) {
		numerator   = kFps2997Numerator;
		denominator = kFpsDenominator1001;
	} else if (qRound(fps * kFpsMultiplier6) == kFps4795Rounded) {
		numerator   = kFps4795Numerator;
		denominator = kFpsDenominator1001;
	} else if (qRound(fps * kFpsMultiplier5) == kFps5994Rounded) {
		numerator   = kFps5994Numerator;
		denominator = kFpsDenominator1001;
	} else {
		// Workaround storing QDoubleSpinBox::value() loses precision.
		numerator   = qRound(fps * kFpsPrecision);
		denominator = kFpsPrecision;
		auto gcd    = greatestCommonDivisor(numerator, denominator);
		numerator /= gcd;
		denominator /= gcd;
	}
}

auto Util::textColor(const QColor& color) -> QString {
	return (color.value() < 150) ? "white" : "black";
}

void Util::cameraFrameRateSize(const QByteArray& deviceName, qreal& frameRate, QSize& size) {
	std::unique_ptr<QCamera> camera;
	for (const QCameraDevice& cameraDevice : QMediaDevices::videoInputs()) {
		if (cameraDevice.id() == deviceName) {
			camera = std::make_unique<QCamera>(cameraDevice);
			break;
		}
	}
	if (camera) {
		auto         currentFormat = camera->cameraDevice().videoFormats().first();
		QList<QSize> resolutions;
		for (const auto& format : camera->cameraDevice().videoFormats()) {
			resolutions << format.resolution();
		}
		if (resolutions.size() > 0) {
			LOG_INFO() << "resolutions:" << resolutions;
			// Get the highest resolution
			camera->setCameraFormat(currentFormat);
			for (const auto& format : camera->cameraDevice().videoFormats()) {
				if (format.resolution().width() > currentFormat.resolution().width() &&
				    format.resolution().height() > currentFormat.resolution().height()) {
					camera->setCameraFormat(format);
					currentFormat = format;
				}
			}
		}
		if (currentFormat.maxFrameRate() > 0) {
			frameRate = currentFormat.maxFrameRate();
		}
		if (currentFormat.resolution().width() > 0) {
			size = currentFormat.resolution();
		}
	}
}

auto Util::ProducerIsTimewarp(Mlt::Producer* producer) -> bool {
	return QString::fromUtf8(producer->get("mlt_service")) == "timewarp";
}

auto Util::GetFilenameFromProducer(Mlt::Producer* producer, bool useOriginal) -> QString {
	QString resource;
	if (useOriginal && producer->get(kOriginalResourceProperty)) {
		resource = QString::fromUtf8(producer->get(kOriginalResourceProperty));
	} else if (ProducerIsTimewarp(producer)) {
		resource = QString::fromUtf8(producer->get("resource"));
		auto i   = resource.indexOf(':');
		if (producer->get_int(kIsProxyProperty) && i > 0) {
			resource = resource.mid(i + 1);
		} else {
			resource = QString::fromUtf8(producer->get("warp_resource"));
		}
	} else {
		resource = QString::fromUtf8(producer->get("resource"));
	}
	if (QFileInfo(resource).isRelative()) {
		QString const basePath = QFileInfo(MAIN.fileName()).canonicalPath();
		QFileInfo const fi(basePath, resource);
		resource = fi.filePath();
	}
	return resource;
}

auto Util::GetSpeedFromProducer(Mlt::Producer* producer) -> double {
	double speed = 1.0;
	if (ProducerIsTimewarp(producer)) {
		speed = fabs(producer->get_double("warp_speed"));
	}
	return speed;
}

auto Util::updateCaption(Mlt::Producer* producer) -> QString {
	const double warpSpeed = GetSpeedFromProducer(producer);
	QString const resource = GetFilenameFromProducer(producer);
	QString const name = Util::baseName(resource, true);
	QString caption   = producer->get(kShotcutCaptionProperty);
	if (caption.isEmpty() || caption.startsWith(name)) {
		// compute the caption
		if (warpSpeed != 1.0)
			caption = QStringLiteral("%1 (%2x)").arg(name).arg(warpSpeed);
		else
			caption = name;
		producer->set(kShotcutCaptionProperty, caption.toUtf8().constData());
	}
	return caption;
}

void Util::passProducerProperties(Mlt::Producer* src, Mlt::Producer* dst) {
	dst->pass_list(*src,
	               "audio_index, video_index, astream, vstream, force_aspect_ratio,"
	               "video_delay, force_progressive, force_tff, force_full_range, color_range, "
	               "warp_pitch, rotate," kAspectRatioNumerator "," kAspectRatioDenominator "," kShotcutHashProperty
	               "," kPlaylistIndexProperty "," kShotcutSkipConvertProperty "," kCommentProperty
	               "," kDefaultAudioIndexProperty "," kShotcutCaptionProperty "," kOriginalResourceProperty
	               "," kDisableProxyProperty "," kIsProxyProperty "," kShotcutProducerProperty);
	QString const shotcutProducer(src->get(kShotcutProducerProperty));
	QString const service(src->get("mlt_service"));
	if (service.startsWith("avformat") || shotcutProducer == "avformat")
		dst->set(kShotcutProducerProperty, "avformat");
}

auto Util::warnIfLowDiskSpace(const QString& path) -> bool {
	// Check if the drive this file will be on is getting low on space.
	if (Settings.encodeFreeSpaceCheck()) {
		QStorageInfo const si(QFileInfo(path).path());
		LOG_DEBUG() << si.bytesAvailable() << "bytes available on" << si.displayName();
		if (si.isValid() && si.bytesAvailable() < kFreeSpaceThesholdGB) {
			QMessageBox dialog(QMessageBox::Question, QApplication::applicationDisplayName(),
			                   QObject::tr("The drive you chose only has %1 MiB of free space.\n"
			                               "Do you still want to continue?")
								   .arg(si.bytesAvailable() / setBytesAvailableNumber / setBytesAvailableNumber),
			                   QMessageBox::No | QMessageBox::Yes);
			dialog.setWindowModality(QmlApplication::dialogModality());
			dialog.setDefaultButton(QMessageBox::Yes);
			dialog.setEscapeButton(QMessageBox::No);
			dialog.setCheckBox(new QCheckBox(QObject::tr("Do not show this anymore.", "Export free disk space warning dialog")));
			const int result = dialog.exec();
			if (dialog.checkBox()->isChecked())
				Settings.setEncodeFreeSpaceCheck(false);
			if (result == QMessageBox::No) {
				return true;
			}
		}
	}
	return false;
}

auto Util::isFpsDifferent(double a, double b) -> bool {
	return qAbs(a - b) > 0.001;
}

auto Util::getNextFile(const QString& filePath) -> QString {
	QFileInfo const info(filePath);
	QString basename = info.completeBaseName();
	QString extension = info.suffix();
	if (extension.isEmpty()) {
		extension = basename;
		basename  = QString();
	}
	for (unsigned i = 1; i < std::numeric_limits<unsigned>::max(); i++) {
		QString const filename = QString::fromLatin1("%1%2.%3").arg(basename).arg(i).arg(extension);
		if (!info.dir().exists(filename))
			return info.dir().filePath(filename);
	}
	return filePath;
}

auto Util::trcString(int trc) -> QString {
	QString trcString = QObject::tr("unknown (%1)").arg(trc);
	switch (trc) {
	case 0:
		trcString = QObject::tr("NA");
		break;
	case 1:
		trcString = "ITU-R BT.709";
		break;
	case 6:
		trcString = "ITU-R BT.601";
		break;
	case 7:
		trcString = "SMPTE ST240";
		break;
	case 11:
		trcString = "IEC 61966-2-4";
		break;
	case 13:
		trcString = "sRGB";
		break;
	case 14:
		trcString = "ITU-R BT.2020";
		break;
	case 15:
		trcString = "ITU-R BT.2020";
		break;
	case 16:
		trcString = "SMPTE ST2084 (PQ)";
		break;
	case 17:
		trcString = "SMPTE ST428";
		break;
	case 18:
		trcString = "ARIB B67 (HLG)";
		break;
	}
	return trcString;
}

auto Util::trcIsCompatible(int trc) -> bool {
	// Transfer characteristics > SMPTE240M Probably need conversion except IEC61966-2-4 is OK
	return trc <= 7 || trc == 11 || trc == 13 || trc == 14 || trc == 15 || trc == 18;
}

auto Util::getConversionAdvice(Mlt::Producer* producer) -> QString {
	QString advice;
	producer->probe();
	QString const resource = Util::GetFilenameFromProducer(producer);
	const int trc = producer->get_int("meta.media.color_trc");
	if (!Util::trcIsCompatible(trc)) {
		QString const trcString = Util::trcString(trc);
		LOG_INFO() << resource << "Probable HDR" << trcString;
		advice = QObject::tr("This file uses color transfer characteristics %1, which may result "
		                     "in incorrect colors or brightness in Shotcut.")
		             .arg(trcString);
	} else if (producer->get_int("meta.media.variable_frame_rate")) {
		LOG_INFO() << resource << "is variable frame rate";
		advice = QObject::tr("This file is variable frame rate, which is not reliable for editing.");
	} else if (QFile::exists(resource) && !MLT.isSeekable(producer)) {
		LOG_INFO() << resource << "is not seekable";
		advice = QObject::tr("This file does not support seeking and cannot be used for editing.");
	} else if (QFile::exists(resource) && resource.endsWith(".m2t")) {
		LOG_INFO() << resource << "is HDV";
		advice = QObject::tr("This file format (HDV) is not reliable for editing.");
	}
	return advice;
}

auto Util::mltColorFromQColor(const QColor& color) -> mlt_color {
	return mlt_color{static_cast<uint8_t>(color.red()), static_cast<uint8_t>(color.green()),
	                 static_cast<uint8_t>(color.blue()), static_cast<uint8_t>(color.alpha())};
}

void Util::offerSingleFileConversion(QString& message, Mlt::Producer* producer, QWidget* parent) {
	TranscodeDialog dialog(message.append(QObject::tr(
	                           " Do you want to convert it to an edit-friendly format?\n\n"
	                           "If yes, choose a format below and then click OK to choose a file name. "
	                           "After choosing a file name, a job is created. "
	                           "When it is done, it automatically replaces clips, or you can double-click the "
	                           "job to open it.\n")),
	                       producer->get_int("progressive"), parent);
	dialog.setWindowModality(QmlApplication::dialogModality());
	dialog.showCheckBox();
	dialog.set709Convert(!Util::trcIsCompatible(producer->get_int("meta.media.color_trc")));
	dialog.showSubClipCheckBox();
	LOG_DEBUG() << "in" << producer->get_in() << "out" << producer->get_out() << "length" << producer->get_length() - 1;
	dialog.setSubClipChecked(producer->get_in() > 0 || producer->get_out() < producer->get_length() - 1);
	auto fps = Util::getAndroidFrameRate(producer);
	if (fps > 0.0)
		dialog.setFrameRate(fps);
	Transcoder transcoder;
	transcoder.addProducer(producer);
	transcoder.convert(dialog);
}

auto Util::getAndroidFrameRate(Mlt::Producer* producer) -> double {
	auto fps = producer->get_double("meta.attr.com.android.capture.fps.markup");
	if (!qIsFinite(fps))
		fps = 0.0;
	return fps;
}

auto Util::getSuggestedFrameRate(Mlt::Producer* producer) -> double {
	auto fps = producer->get_double("meta.attr.com.android.capture.fps.markup");
	if (!qIsFinite(fps))
		fps = 0.0;
	if (fps <= 0.0) {
		fps = producer->get_double("meta.media.frame_rate_num");
		if (producer->get_double("meta.media.frame_rate_den") > 0)
			fps /= producer->get_double("meta.media.frame_rate_den");
		if (producer->get("force_fps"))
			fps = producer->get_double("fps");
	}
	return fps;
}

auto Util::openMltVirtualClip(const QString& path) -> Mlt::Producer {
	Mlt::Producer xmlProducer(nullptr, "xml-clip", path.toUtf8().constData());
	QScopedPointer<Mlt::Profile> const testProfile(xmlProducer.profile());
	if (Settings.playerGPU() && MLT.profile().is_explicit()) {
		if (testProfile->width() != MLT.profile().width() || testProfile->height() != MLT.profile().height() ||
		    Util::isFpsDifferent(MLT.profile().fps(), testProfile->fps())) {
			return {};
		}
	}
	if (xmlProducer.is_valid()) {
		Mlt::Chain chain(MLT.profile());
		chain.set_source(xmlProducer);
		chain.attach_normalizers();
		chain.get_length_time(mlt_time_clock);
		chain.set(kShotcutVirtualClip, 1);
		chain.set("resource", path.toUtf8().constData());
		return chain;
	}
	return {};
}

auto Util::hasiPhoneAmbisonic(Mlt::Producer* producer) -> bool {
	// iPhone 16 Pro has a 4 channel (spatial) audio stream with codec "apac" that causes failure.
	// This is not limited to only iPhone 16 Pro, but I think most iPhones only record one usable audio track.
	return producer && producer->is_valid() && !::qstrcmp(producer->get("meta.media.1.stream.type"), "audio") &&
	       QString(producer->get("meta.attr.com.apple.quicktime.model.markup")).contains("iPhone");
}

auto Util::installFlatpakWrappers(QWidget* parent) -> bool {
	if (!Settings.askFlatpakWrappers())
		return false;
	QMessageBox dialog(QMessageBox::Question, qApp->applicationName(),
	                   parent->tr("<p>Do you want use a Flatpak?</p>"
	                              "<p>Click <b>Yes</b> to install/update the Flatpak wrapper scripts "
	                              "in\n<b>Files > Home > Flatpaks</b>.</p>"
	                              "<p>Tip: Add <b><tt>~/Flatpaks</tt></b> to your <b><tt>$PATH</tt></b> "
	                              "to make them more "
	                              "convenient on the command line.</p>"),
	                   QMessageBox::No | QMessageBox::Yes, parent);
	dialog.setCheckBox(new QCheckBox(parent->tr("Do not show this anymore.")));
	dialog.setWindowModality(QmlApplication::dialogModality());
	dialog.setDefaultButton(QMessageBox::Yes);
	dialog.setEscapeButton(QMessageBox::No);
	const int r = dialog.exec();
	if (dialog.checkBox()->isChecked())
		Settings.setAskFlatpakWrappers(false);
	if (r == QMessageBox::Yes) {
		FlatpakWrapperGenerator flatpaks;
		const auto ls = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
		auto home = QDir(ls.first());
		const auto subdir = QStringLiteral("Flatpaks");
		if (!home.cd(subdir)) {
			if (home.mkdir(subdir))
				home.cd(subdir);
			else
				return false;
		}
		flatpaks.setOutputDir(home.absolutePath());
		flatpaks.setForce(true);
		flatpaks.generateAllInstalled();
		return true;
	}
	return false;
}

auto Util::getExecutable(QWidget* parent) -> QString {
	QString dir;
	QString filter;
#if defined(Q_OS_WIN)
	dir    = QStringLiteral("C:/ProgramData/Microsoft/Windows/Start Menu/Programs");
	filter = parent->tr("Executable Files (*.exe);;All Files (*)");
#elif defined(Q_OS_MAC)
	const auto ls = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	LOG_DEBUG() << ls;
	dir = ls.last();
#elif defined(Q_OS_LINUX)
	if (Util::installFlatpakWrappers(parent)) {
		const auto ls   = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
		auto       home = QDir(ls.first());
		home.cd("Flatpaks");
		dir = home.absolutePath();
	} else {
		dir = QStringLiteral("/usr/bin");
	}
#endif
	return QFileDialog::getOpenFileName(MAIN.window(), parent->tr("Choose Executable"), dir, filter, nullptr,
	                                    Util::getFileDialogOptions());
}

auto Util::dockerStatus(const QString& imageName) -> QPair<bool, bool> {
	// Check if docker executable is available by running 'docker --version'.
	QProcess proc;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	auto env = QProcessEnvironment::systemEnvironment();
	env.remove("LD_LIBRARY_PATH");
	proc.setProcessEnvironment(env);
#endif
	proc.start(Settings.dockerPath(), {"--version"});
	if (!proc.waitForStarted(1000)) {
		return qMakePair(false, false);
	}
	// Keep the timeout short to avoid UI stall.
	if (!proc.waitForFinished(2000)) {
		proc.kill();
		return qMakePair(false, false);
	}
	const bool dockerOk = proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0;
	if (!dockerOk) {
		return qMakePair(false, false);
	}
	if (imageName.isEmpty()) {
		return qMakePair(true, false);
	}

	// Query local images for the given name (may include tag). We avoid pulling.
	// Use 'docker image inspect <imageName>' which returns 0 if present.
	QProcess procImage;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	env = QProcessEnvironment::systemEnvironment();
	env.remove("LD_LIBRARY_PATH");
	procImage.setProcessEnvironment(env);
#endif
	procImage.start(Settings.dockerPath(), {"image", "inspect", imageName});
	if (!procImage.waitForStarted(1000)) {
		return qMakePair(true, false);
	}
	if (!procImage.waitForFinished(5000)) { // allow a bit more time here
		procImage.kill();
		return qMakePair(true, false);
	}
	const bool imageOk = procImage.exitStatus() == QProcess::NormalExit && procImage.exitCode() == 0;
	return qMakePair(true, imageOk);
}

// Helper to extract digest from 'docker manifest inspect' output.
static QString digestFromManifestInspect(const QByteArray& json) {
	auto s   = QString::fromUtf8(json);
	auto idx = s.indexOf("sha256:");
	if (idx >= 0 && idx + 71 <= s.size()) {
		return s.mid(idx, 71);
	}
	return {};
}

static QSet<QString> dockerCurrentState;

auto Util::isDockerImageCurrent(const QString& imageRef) -> bool {
	if (imageRef.isEmpty()) {
		return false;
	}

	// TODO: This comparison is not working outside of the machine on which I built the image.
	// In the meantime, use check once per session per image ref.
	if (dockerCurrentState.contains(imageRef))
		return true;
	dockerCurrentState << imageRef;
	return false;

	// Inspect (local/remote combined behavior) - this is a simplistic approach.
	QProcess localProc;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	auto env = QProcessEnvironment::systemEnvironment();
	env.remove("LD_LIBRARY_PATH");
	localProc.setProcessEnvironment(env);
#endif
	localProc.start(Settings.dockerPath(), {"image", "inspect", imageRef});
	if (!localProc.waitForStarted(1000) || !localProc.waitForFinished(4000)) {
		if (localProc.state() != QProcess::NotRunning)
			localProc.kill();
	}
	QString localDigest;
	if (localProc.exitStatus() == QProcess::NormalExit && localProc.exitCode() == 0) {
		localDigest = digestFromManifestInspect(localProc.readAllStandardOutput());
	}

	// Run again expecting remote freshness (will hit registry) without pulling.
	QProcess remoteProc;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	env = QProcessEnvironment::systemEnvironment();
	env.remove("LD_LIBRARY_PATH");
	remoteProc.setProcessEnvironment(env);
#endif
	remoteProc.start(Settings.dockerPath(), {"manifest", "inspect", imageRef});
	if (!remoteProc.waitForStarted(1000) || !remoteProc.waitForFinished(15000)) {
		if (remoteProc.state() != QProcess::NotRunning)
			remoteProc.kill();
		return false;
	}
	if (remoteProc.exitStatus() != QProcess::NormalExit || remoteProc.exitCode() != 0) {
		return false;
	}
	auto remoteDigest = digestFromManifestInspect(remoteProc.readAllStandardOutput());
	return !remoteDigest.isEmpty() && !localDigest.isEmpty() && localDigest == remoteDigest;
}

void Util::isDockerImageCurrentAsync(const QString& imageRef, QObject* receiver, const std::function<void(bool)>& callback) {
	if (!callback) {
		return; // nothing to do
	}
	if (imageRef.isEmpty()) {
		callback(false);
		return;
	}

	// TODO: This comparison is not working outside of the machine on which I built the image.
	// In the meantime, use check once per session per image ref.
	if (dockerCurrentState.contains(imageRef)) {
		callback(true);
		return;
	}
	dockerCurrentState << imageRef;
	callback(false);
	return;

	auto emitResult = [callback](bool result) -> void {
		QMetaObject::invokeMethod(qApp, [callback, result]() -> void { callback(result); }, Qt::QueuedConnection);
	};

	// Start with local inspect.
	auto* localProc = new QProcess(receiver);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	auto env = QProcessEnvironment::systemEnvironment();
	env.remove("LD_LIBRARY_PATH");
	localProc->setProcessEnvironment(env);
#endif

	QObject::connect(localProc, &QProcess::errorOccurred, localProc, [localProc, emitResult](QProcess::ProcessError) -> void {
		localProc->deleteLater();
		emitResult(false);
	});
	QObject::connect(localProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), localProc,
					 [imageRef, receiver, emitResult, localProc](int, QProcess::ExitStatus) -> void {
		                 QString localDigest;
		                 if (localProc->exitStatus() == QProcess::NormalExit && localProc->exitCode() == 0) {
			                 localDigest = digestFromManifestInspect(localProc->readAllStandardOutput());
		                 }
		                 localProc->deleteLater();
		                 // Remote manifest inspect regardless; if docker missing it will fail.
		                 auto* remoteProc = new QProcess(receiver);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
		                 auto env = QProcessEnvironment::systemEnvironment();
		                 env.remove("LD_LIBRARY_PATH");
		                 remoteProc->setProcessEnvironment(env);
#endif

		                 QObject::connect(remoteProc, &QProcess::errorOccurred, remoteProc,
										  [remoteProc, emitResult](QProcess::ProcessError) -> void {
			                                  remoteProc->deleteLater();
			                                  emitResult(false);
		                                  });
		                 QObject::connect(remoteProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
										  remoteProc, [emitResult, remoteProc, localDigest](int, QProcess::ExitStatus) -> void {
			                                  QString remoteDigest;
			                                  if (remoteProc->exitStatus() == QProcess::NormalExit &&
			                                      remoteProc->exitCode() == 0) {
				                                  remoteDigest =
				                                      digestFromManifestInspect(remoteProc->readAllStandardOutput());
			                                  }
			                                  remoteProc->deleteLater();
											  const bool current = !remoteDigest.isEmpty() && !localDigest.isEmpty() && localDigest == remoteDigest;
			                                  emitResult(current);
		                                  });
		                 LOG_DEBUG() << "docker manifest inspect" << imageRef;
		                 remoteProc->start(Settings.dockerPath(), {"manifest", "inspect", imageRef});
	                 });
	LOG_DEBUG() << "docker image inspect" << imageRef;
	localProc->start(Settings.dockerPath(), {"image", "inspect", imageRef});
}

auto Util::isChromiumAvailable() -> bool {
	// Check if Chromium executable file exists and is executable
	QFileInfo const fileInfo(Settings.chromiumPath());
	return fileInfo.exists() && fileInfo.isExecutable();
}

auto Util::startDetached(const QString& program, const QStringList& arguments) -> bool {
	QProcess const process;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Remove parts of environment variables that our launch script has set
	auto env = QProcessEnvironment::systemEnvironment();
	// Get the parent of bin/shotcut
	QString appDir = QFileInfo(QCoreApplication::applicationDirPath()).dir().absolutePath();

	auto filterEnvVar = [&env, &appDir](const QString& varName) {
		QString value = env.value(varName);
		if (!value.isEmpty()) {
			QStringList paths = value.split(':');
			QStringList filtered;
			for (QString& path : paths) {
				if (!path.contains(appDir)) {
					filtered << path;
				} else {
					LOG_DEBUG() << "removing" << path << "from env var" << varName;
				}
			}
			env.insert(varName, filtered.join(':'));
		}
	};

	filterEnvVar("FREI0R_PATH");
	filterEnvVar("LADSPA_PATH");
	filterEnvVar("LD_LIBRARY_PATH");
	filterEnvVar("MANPATH");
	filterEnvVar("PKG_CONFIG_PATH");
	filterEnvVar("PYTHONHOME");

	// These are relative paths and not cleanable with filterEnvVar()
	env.remove("QML2_IMPORT_PATH");
	env.remove("QT_PLUGIN_PATH");

	process.setProcessEnvironment(env);
#endif

	return process.startDetached(program, arguments);
}

auto Util::openUrl(const QUrl& url) -> bool {
	auto success = QDesktopServices::openUrl(url);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	if (!success)
		success = startDetached("xdg-open", {url.toString()});
#endif
	return success;
}
