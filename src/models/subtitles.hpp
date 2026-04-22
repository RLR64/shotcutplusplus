/*
 * Copyright (c) 2024 Meltytech, LLC
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

#ifndef SUBTITLES_HPP
#define SUBTITLES_HPP

// STL
#include <cstdint>
#include <string>
#include <vector>

namespace Subtitles {

struct SubtitleItem {
	int64_t     start;
	int64_t     end;
	std::string text;
};

typedef std::vector<Subtitles::SubtitleItem> SubtitleVector;

auto readFromSrtFile(const std::string& path) -> SubtitleVector;
auto writeToSrtFile(const std::string& path, const SubtitleVector& items) -> bool;
auto readFromSrtString(const std::string& text) -> SubtitleVector;
auto writeToSrtString(std::string& text, const SubtitleVector& items) -> bool;
auto indexForTime(const SubtitleVector& items, int64_t msTime, int searchStart, int msMargin) -> int;
} // namespace Subtitles

#endif // SUBTITLES_HPP
