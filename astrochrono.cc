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

#include "astrochrono.h"

#include <regex>
#include <string>
#include <cstdlib>

namespace astrochrono {

namespace {

// Maximum number of days expressible as signed 64-bit nanoseconds.
// 2^64 / 2 / 1e9 / 86400
// NOTE: std::int64_t nsecs will wrap:
// -- earliest date representable = sep 21, 1677 00:00:00
// -- latest date representable   = apr 12, 2262 00:00:00
static auto constexpr MAX_DAYS = days{106751.99};

/// Nanoseconds per day.
// static double constexpr NSEC_PER_DAY = 86.4e12;

// Difference between Terrestrial Time and TAI.
static auto constexpr TT_MINUS_TAI = std::chrono::nanoseconds{32184000000LL};

/* Leap second table as string.
 *
 * Source: http://maia.usno.navy.mil/ser7/tai-utc.dat
 */
static std::string leap_string =
        "\
1961 JAN  1 =JD 2437300.5  TAI-UTC=   1.4228180 S + (MJD - 37300.) X 0.001296 S\n\
1961 AUG  1 =JD 2437512.5  TAI-UTC=   1.3728180 S + (MJD - 37300.) X 0.001296 S\n\
1962 JAN  1 =JD 2437665.5  TAI-UTC=   1.8458580 S + (MJD - 37665.) X 0.0011232S\n\
1963 NOV  1 =JD 2438334.5  TAI-UTC=   1.9458580 S + (MJD - 37665.) X 0.0011232S\n\
1964 JAN  1 =JD 2438395.5  TAI-UTC=   3.2401300 S + (MJD - 38761.) X 0.001296 S\n\
1964 APR  1 =JD 2438486.5  TAI-UTC=   3.3401300 S + (MJD - 38761.) X 0.001296 S\n\
1964 SEP  1 =JD 2438639.5  TAI-UTC=   3.4401300 S + (MJD - 38761.) X 0.001296 S\n\
1965 JAN  1 =JD 2438761.5  TAI-UTC=   3.5401300 S + (MJD - 38761.) X 0.001296 S\n\
1965 MAR  1 =JD 2438820.5  TAI-UTC=   3.6401300 S + (MJD - 38761.) X 0.001296 S\n\
1965 JUL  1 =JD 2438942.5  TAI-UTC=   3.7401300 S + (MJD - 38761.) X 0.001296 S\n\
1965 SEP  1 =JD 2439004.5  TAI-UTC=   3.8401300 S + (MJD - 38761.) X 0.001296 S\n\
1966 JAN  1 =JD 2439126.5  TAI-UTC=   4.3131700 S + (MJD - 39126.) X 0.002592 S\n\
1968 FEB  1 =JD 2439887.5  TAI-UTC=   4.2131700 S + (MJD - 39126.) X 0.002592 S\n\
1972 JAN  1 =JD 2441317.5  TAI-UTC=  10.0       S + (MJD - 41317.) X 0.0      S\n\
1972 JUL  1 =JD 2441499.5  TAI-UTC=  11.0       S + (MJD - 41317.) X 0.0      S\n\
1973 JAN  1 =JD 2441683.5  TAI-UTC=  12.0       S + (MJD - 41317.) X 0.0      S\n\
1974 JAN  1 =JD 2442048.5  TAI-UTC=  13.0       S + (MJD - 41317.) X 0.0      S\n\
1975 JAN  1 =JD 2442413.5  TAI-UTC=  14.0       S + (MJD - 41317.) X 0.0      S\n\
1976 JAN  1 =JD 2442778.5  TAI-UTC=  15.0       S + (MJD - 41317.) X 0.0      S\n\
1977 JAN  1 =JD 2443144.5  TAI-UTC=  16.0       S + (MJD - 41317.) X 0.0      S\n\
1978 JAN  1 =JD 2443509.5  TAI-UTC=  17.0       S + (MJD - 41317.) X 0.0      S\n\
1979 JAN  1 =JD 2443874.5  TAI-UTC=  18.0       S + (MJD - 41317.) X 0.0      S\n\
1980 JAN  1 =JD 2444239.5  TAI-UTC=  19.0       S + (MJD - 41317.) X 0.0      S\n\
1981 JUL  1 =JD 2444786.5  TAI-UTC=  20.0       S + (MJD - 41317.) X 0.0      S\n\
1982 JUL  1 =JD 2445151.5  TAI-UTC=  21.0       S + (MJD - 41317.) X 0.0      S\n\
1983 JUL  1 =JD 2445516.5  TAI-UTC=  22.0       S + (MJD - 41317.) X 0.0      S\n\
1985 JUL  1 =JD 2446247.5  TAI-UTC=  23.0       S + (MJD - 41317.) X 0.0      S\n\
1988 JAN  1 =JD 2447161.5  TAI-UTC=  24.0       S + (MJD - 41317.) X 0.0      S\n\
1990 JAN  1 =JD 2447892.5  TAI-UTC=  25.0       S + (MJD - 41317.) X 0.0      S\n\
1991 JAN  1 =JD 2448257.5  TAI-UTC=  26.0       S + (MJD - 41317.) X 0.0      S\n\
1992 JUL  1 =JD 2448804.5  TAI-UTC=  27.0       S + (MJD - 41317.) X 0.0      S\n\
1993 JUL  1 =JD 2449169.5  TAI-UTC=  28.0       S + (MJD - 41317.) X 0.0      S\n\
1994 JUL  1 =JD 2449534.5  TAI-UTC=  29.0       S + (MJD - 41317.) X 0.0      S\n\
1996 JAN  1 =JD 2450083.5  TAI-UTC=  30.0       S + (MJD - 41317.) X 0.0      S\n\
1997 JUL  1 =JD 2450630.5  TAI-UTC=  31.0       S + (MJD - 41317.) X 0.0      S\n\
1999 JAN  1 =JD 2451179.5  TAI-UTC=  32.0       S + (MJD - 41317.) X 0.0      S\n\
2006 JAN  1 =JD 2453736.5  TAI-UTC=  33.0       S + (MJD - 41317.) X 0.0      S\n\
2009 JAN  1 =JD 2454832.5  TAI-UTC=  34.0       S + (MJD - 41317.) X 0.0      S\n\
2012 JUL  1 =JD 2456109.5  TAI-UTC=  35.0       S + (MJD - 41317.) X 0.0      S\n\
2015 JUL  1 =JD 2457204.5  TAI-UTC=  36.0       S + (MJD - 41317.) X 0.0      S\n\
2017 JAN  1 =JD 2457754.5  TAI-UTC=  37.0       S + (MJD - 41317.) X 0.0      S\n\
";

/// Leap second descriptor.
struct Leap {
    std::int64_t when_utc;  ///< UTC nanosecs of change
    std::int64_t when_tai;  ///< TAI nanosecs of change
    double offset;          ///< TAI - UTC
    double mjd_ref;         ///< Intercept for MJD interpolation
    double drift;           ///< Slope of MJD interpolation
};

class LeapTable : public std::vector<Leap> {
public:
    LeapTable(const char* leap_string);
};

LeapTable leap_table(leap_string.c_str());

LeapTable::LeapTable(const char* leap_string) {
    Leap l;
    clear();
    std::regex re(
            "\\d{4}.*?=JD\\s*([\\d.]+)\\s+TAI-UTC=\\s+([\\d.]+)\\s+S"
            " \\+ \\(MJD - ([\\d.]+)\\) X ([\\d.]+)\\s*S\n");
    for (auto i = std::cregex_iterator(leap_string, leap_string + strlen(leap_string), re);
         i != std::cregex_iterator(); ++i) {
        auto mjd_utc = static_cast<days>(strtod((*i)[1].first, 0)) - MJD_TO_JD;
        l.offset = strtod((*i)[2].first, 0);
        l.mjd_ref = strtod((*i)[3].first, 0);
        l.drift = strtod((*i)[4].first, 0);
        l.when_utc = std::chrono::duration_cast<std::chrono::nanoseconds>(mjd_utc - EPOCH_IN_MJD).count();
        l.when_tai = l.when_utc +
                     static_cast<std::int64_t>(1.0e9 * (l.offset + (mjd_utc.count() - l.mjd_ref) * l.drift));
        push_back(l);
    }
}

std::chrono::nanoseconds mjd_to_ns(days mjd) {
    if (mjd > EPOCH_IN_MJD + MAX_DAYS || mjd < EPOCH_IN_MJD - MAX_DAYS) {
        throw std::domain_error("MJD out of valid range");
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(mjd - EPOCH_IN_MJD);
}

std::chrono::nanoseconds calendar_datetime_to_ns(int year, int month, int day, int hr, int min, int sec) {
    int constexpr minYear = 1902;
    int constexpr maxYear = 2261;
    if ((year < minYear) || (year > maxYear)) {
        throw std::domain_error("Year out of valid range");
    }

    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hr;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_wday = 0;
    tm.tm_yday = 0;
    tm.tm_isdst = 0;
    tm.tm_gmtoff = 0;

    // Convert to seconds since the epoch, correcting to UTC.
    // Although timegm() is non-standard, it is a commonly-supported
    // extension and is much safer/more reliable than mktime(3) in that
    // it doesn't try to deal with the anomalies of local time zones.
    time_t secs = timegm(&tm);

    // std::int64_t nsecs will blow out beyond 1677-09-21T00:00:00 and 2262-04-12T00:00:00
    // (refering to the values of EPOCH_IN_MJD +/- MAX_DAYS ... exceeds 64 bits.)
    // On older machines a tm struct is only 32 bits, and saturates at:
    //    low end - 1901-12-13, 20:45:52
    //    hi end  - 2038-01-19, 03:14:07
    // On newer machines the upper limit is a date in 2262, but the low end is unchanged,
    // and a unit test will show the problem for dates later than 2038-01-19

    // timegm returns -1 on error, but the date at unix epoch -1 second also returns a valid value of -1,
    // so be sure to test for that

    if (secs == -1) {
        bool is_bad = true;  // assume the worst
        if (year == 1969) {
            // date may be the one date at which unix sec = -1; try a different year
            tm.tm_year = 70;
            if (timegm(&tm) != -1) {
                is_bad = false;
            }
        }
        if (is_bad) {
            throw std::domain_error("Unconvertible date");
        }
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(static_cast<std::chrono::seconds>(secs));
}

// UTC has a "Z" suffix, TAI and TT do not
template <typename Clock>
constexpr const char* iso8601_re =
        "(\\d{4})-?(\\d{2})-?(\\d{2})"
        "T"
        "(\\d{2}):?(\\d{2}):?(\\d{2})"
        "([.,](\\d*))?";

template <>
constexpr const char* iso8601_re<utc_clock> =
        "(\\d{4})-?(\\d{2})-?(\\d{2})"
        "T"
        "(\\d{2}):?(\\d{2}):?(\\d{2})"
        "([.,](\\d*))?"
        "Z";

template <typename Clock>
typename Clock::time_point time_point_from_string(std::string const& iso8601) {
    // time zone "Z" required for UTC
    std::regex re{iso8601_re<Clock>};

    std::smatch matches;
    if (!std::regex_match(iso8601, matches, re)) {
        throw std::invalid_argument("Not in acceptable ISO8601 format: " + iso8601);
    }
    // determine TAI nsec truncated to integer seconds
    // by constructing a DateTime from year, month, day...
    auto tp = Clock::from_calendar(atoi(matches.str(1).c_str()), atoi(matches.str(2).c_str()),
                                   atoi(matches.str(3).c_str()), atoi(matches.str(4).c_str()),
                                   atoi(matches.str(5).c_str()), atoi(matches.str(6).c_str()));
    // add fractional seconds, if any
    if (matches[7].matched) {
        std::string frac = matches.str(8);
        int places = frac.size();
        if (places > 9) {  // truncate fractional nanosec
            frac.erase(9);
        }
        int value = atoi(frac.c_str());
        while (places < 9) {
            value *= 10;
            ++places;
        }
        tp += static_cast<std::chrono::nanoseconds>(value);
    }
    return tp;
}

template <typename TimePoint>
std::string to_string(TimePoint const& tp, const char* suffix) {
    using namespace std::chrono_literals;
    struct tm gmt(to_gmtime(tp));

    std::chrono::nanoseconds fracnsecs = tp.time_since_epoch() % 1s;
    if (fracnsecs.count() < 0) {
        fracnsecs += 1s;
    }

    std::ostringstream os;
    os << std::setw(4) << std::setfill('0') << gmt.tm_year + 1900 << "-";
    os << std::setw(2) << std::setfill('0') << gmt.tm_mon + 1 << "-";
    os << std::setw(2) << std::setfill('0') << gmt.tm_mday << "T";
    os << std::setw(2) << std::setfill('0') << gmt.tm_hour << ":";
    os << std::setw(2) << std::setfill('0') << gmt.tm_min << ":";
    os << std::setw(2) << std::setfill('0') << gmt.tm_sec << ".";
    os << std::setw(9) << std::setfill('0') << fracnsecs.count() << suffix;
    return os.str();
}

}  // namespace

template <>
tai_clock::time_point timescale_cast<tai_clock>(utc_clock::time_point const& tp) {
    std::int64_t nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
    size_t i;
    for (i = 0; i < leap_table.size(); ++i) {
        if (nsecs < leap_table[i].when_utc) break;
    }
    if (i == 0) {
        throw std::domain_error("DateTime value too early for UTC->TAI conversion");
    }
    Leap const& l(leap_table[i - 1]);
    double mjd = to_mjd(tp).count();
    double leap_secs = l.offset + (mjd - l.mjd_ref) * l.drift;
    std::int64_t leap_nsecs = static_cast<std::int64_t>(leap_secs * 1.0e9 + 0.5);
    return tai_clock::time_point{static_cast<std::chrono::nanoseconds>(nsecs + leap_nsecs)};
}

template <>
tai_clock::time_point timescale_cast<tai_clock>(tt_clock::time_point const& tp) {
    return tai_clock::time_point{tp.time_since_epoch() - TT_MINUS_TAI};
}

template <>
utc_clock::time_point timescale_cast<utc_clock>(tai_clock::time_point const& tp) {
    std::int64_t nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
    size_t i;
    for (i = 0; i < leap_table.size(); ++i) {
        if (nsecs < leap_table[i].when_tai) break;
    }
    if (i == 0) {
        throw std::domain_error("DateTime value too early for TAI->UTC conversion");
    }
    Leap const& l(leap_table[i - 1]);
    double mjd = to_mjd(tp).count();
    double leap_secs = l.offset + (mjd - l.mjd_ref) * l.drift;
    // Correct for TAI MJD vs. UTC MJD.
    constexpr double SECONDS_PER_DAY = 24. * 3600.;
    leap_secs /= 1.0 + l.drift / SECONDS_PER_DAY;
    std::int64_t leap_nsecs = static_cast<std::int64_t>(leap_secs * 1.0e9 + 0.5);
    return utc_clock::time_point{static_cast<std::chrono::nanoseconds>(nsecs - leap_nsecs)};
}

template <>
utc_clock::time_point timescale_cast<utc_clock>(tt_clock::time_point const& tp) {
    return timescale_cast<utc_clock>(timescale_cast<tai_clock>(tp));
}

template <>
tt_clock::time_point timescale_cast<tt_clock>(tai_clock::time_point const& tp) {
    return tt_clock::time_point{tp.time_since_epoch() + TT_MINUS_TAI};
}

template <>
tt_clock::time_point timescale_cast<tt_clock>(utc_clock::time_point const& tp) {
    return timescale_cast<tt_clock>(timescale_cast<tai_clock>(tp));
}

utc_clock::time_point utc_clock::now() {
    struct timeval tv;
    if (gettimeofday(&tv, 0) == 0) {
        return time_point{static_cast<std::chrono::seconds>(tv.tv_sec) +
                          static_cast<std::chrono::microseconds>(tv.tv_usec)};
    } else {
        throw std::runtime_error("Failed to get current time");
    }
};

tai_clock::time_point tai_clock::now() { return timescale_cast<tai_clock>(utc_clock::now()); }

tt_clock::time_point tt_clock::now() { return timescale_cast<tt_clock>(utc_clock::now()); }

utc_clock::time_point utc_clock::from_mjd(days mjd) { return time_point{mjd_to_ns(mjd)}; }

utc_clock::time_point utc_clock::from_jd(days jd) { return utc_clock::from_mjd(jd - MJD_TO_JD); }

tai_clock::time_point tai_clock::from_mjd(days mjd) { return time_point{mjd_to_ns(mjd)}; }

tai_clock::time_point tai_clock::from_jd(days jd) { return tai_clock::from_mjd(jd - MJD_TO_JD); }

tt_clock::time_point tt_clock::from_mjd(days mjd) { return time_point{mjd_to_ns(mjd)}; }

tt_clock::time_point tt_clock::from_jd(days jd) { return tt_clock::from_mjd(jd - MJD_TO_JD); }

utc_clock::time_point utc_clock::from_calendar(int year, int month, int day, int hr, int min, int sec) {
    return time_point{calendar_datetime_to_ns(year, month, day, hr, min, sec)};
}

tai_clock::time_point tai_clock::from_calendar(int year, int month, int day, int hr, int min, int sec) {
    return time_point{calendar_datetime_to_ns(year, month, day, hr, min, sec)};
}

tt_clock::time_point tt_clock::from_calendar(int year, int month, int day, int hr, int min, int sec) {
    return time_point{calendar_datetime_to_ns(year, month, day, hr, min, sec)};
}

utc_clock::time_point utc_clock::from_string(std::string const& iso8601) {
    return time_point_from_string<utc_clock>(iso8601);
}

tai_clock::time_point tai_clock::from_string(std::string const& iso8601) {
    return time_point_from_string<tai_clock>(iso8601);
}

tt_clock::time_point tt_clock::from_string(std::string const& iso8601) {
    return time_point_from_string<tt_clock>(iso8601);
}

template <typename TimePoint>
struct tm to_gmtime(TimePoint const& tp) {
    using namespace std::chrono_literals;
    struct tm gmt;
    // Round to negative infinity
    auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
    auto frac = nsecs % 1s;
    if (nsecs.count() < 0 && frac.count() < 0) {
        nsecs -= 1s + frac;
    } else {
        nsecs -= frac;
    }
    time_t secs = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(nsecs).count());
    gmtime_r(&secs, &gmt);
    return gmt;
}

template <typename TimePoint>
struct timespec to_timespec(TimePoint const& tp) {
    using namespace std::chrono_literals;
    struct timespec ts;
    auto t = tp.time_since_epoch();
    ts.tv_sec = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(t).count());
    ts.tv_nsec = static_cast<int>((std::chrono::duration_cast<std::chrono::nanoseconds>(t) % 1s).count());
    return ts;
}

template <typename TimePoint>
struct timeval to_timeval(TimePoint const& tp) {
    using namespace std::chrono_literals;
    struct timeval tv;
    auto t = tp.time_since_epoch();
    tv.tv_sec = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(t).count());
    tv.tv_usec = static_cast<int>((std::chrono::duration_cast<std::chrono::microseconds>(t) % 1s).count());
    return tv;
}

template <>
std::string to_string<tai_clock::time_point>(tai_clock::time_point const& tp) {
    return to_string(tp, "");
}

template <>
std::string to_string<tt_clock::time_point>(tt_clock::time_point const& tp) {
    return to_string(tp, "");
}

template <>
std::string to_string<utc_clock::time_point>(utc_clock::time_point const& tp) {
    return to_string(tp, "Z");
}

// Explicit instantiations
template struct timespec to_timespec<utc_clock::time_point>(utc_clock::time_point const& tp);
template struct timespec to_timespec<tai_clock::time_point>(tai_clock::time_point const& tp);
template struct timespec to_timespec<tt_clock::time_point>(tt_clock::time_point const& tp);

template struct timeval to_timeval<utc_clock::time_point>(utc_clock::time_point const& tp);
template struct timeval to_timeval<tai_clock::time_point>(tai_clock::time_point const& tp);
template struct timeval to_timeval<tt_clock::time_point>(tt_clock::time_point const& tp);

}  // namespace astrochrono