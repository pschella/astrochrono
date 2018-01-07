/*
 * LSST Data Management System
 * Copyright 2008-2018  AURA/LSST.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
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
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <https://www.lsstcorp.org/LegalNotices/>.
 */

#ifndef ASTROCHRONO_H
#define ASTROCHRONO_H

#include <chrono>
#include <ctime>
#include <limits>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace astrochrono {

// Epoch = 1970 JAN  1 00:00:00 = JD 2440587.5 = MJD 40587.0
static double constexpr MJD_TO_JD = 2400000.5;
static double constexpr EPOCH_IN_MJD = 40587.0;

/// Nanoseconds per day.
static double constexpr NSEC_PER_DAY = 86.4e12;

class utc_clock {
public:
    using duration = std::chrono::nanoseconds;
    using rep = typename duration::rep;
    using period = typename duration::rep;
    using time_point = std::chrono::time_point<utc_clock>;
    static constexpr bool is_steady = false;
    static time_point now();
    static time_point from_mjd(double mjd);
    static time_point from_jd(double jd);
    static time_point from_calendar(int year, int month, int day, int hr, int min, int sec);
    static time_point from_string(std::string const &iso8601);
};

class tai_clock {
public:
    using duration = std::chrono::nanoseconds;
    using rep = typename duration::rep;
    using period = typename duration::rep;
    using time_point = std::chrono::time_point<tai_clock>;
    static constexpr bool is_steady = false;
    static time_point now();
    static time_point from_mjd(double mjd);
    static time_point from_jd(double jd);
    static time_point from_calendar(int year, int month, int day, int hr, int min, int sec);
    static time_point from_string(std::string const &iso8601);
};

class tt_clock {
public:
    using duration = std::chrono::nanoseconds;
    using rep = typename duration::rep;
    using period = typename duration::rep;
    using time_point = std::chrono::time_point<tt_clock>;
    static constexpr bool is_steady = false;
    static time_point now();
    static time_point from_mjd(double mjd);
    static time_point from_jd(double jd);
    static time_point from_calendar(int year, int month, int day, int hr, int min, int sec);
    static time_point from_string(std::string const &iso8601);
};

template <typename ToClock, typename TimePoint>
typename ToClock::time_point timescale_cast(TimePoint const &);

template <>
tai_clock::time_point timescale_cast<tai_clock>(utc_clock::time_point const &);

template <>
tai_clock::time_point timescale_cast<tai_clock>(tt_clock::time_point const &);

template <>
utc_clock::time_point timescale_cast<utc_clock>(tai_clock::time_point const &);

template <>
utc_clock::time_point timescale_cast<utc_clock>(tt_clock::time_point const &);

template <>
tt_clock::time_point timescale_cast<tt_clock>(tai_clock::time_point const &);

template <>
tt_clock::time_point timescale_cast<tt_clock>(utc_clock::time_point const &);

template <typename TimePoint>
struct tm to_gmtime(TimePoint const &tp);

template <typename TimePoint>
struct timespec to_timespec(TimePoint const &tp);

template <typename TimePoint>
struct timeval to_timeval(TimePoint const &tp);

template <typename TimePoint>
std::string to_string(TimePoint const &);

template <typename TimePoint>
constexpr double to_mjd(TimePoint const &tp) noexcept {
    double nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
    return nsecs / NSEC_PER_DAY + EPOCH_IN_MJD;
}

template <typename TimePoint>
constexpr double to_jd(TimePoint const &tp) noexcept {
    return to_mjd(tp) + MJD_TO_JD;
}

}  // namespace astrochrono

#endif  // ASTROCHRONO_H
