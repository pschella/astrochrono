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

#include <iostream>
#include <chrono>
#include <unistd.h>

#include "astrochrono.h"

#define BOOST_TEST_MODULE BasicTest
#include <boost/test/unit_test.hpp>

using namespace astrochrono;

namespace sc = std::chrono;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_SUITE(AstrochronoTestSuite)

BOOST_AUTO_TEST_CASE(Gmtime) {
    auto tp = utc_clock::from_string("20090402T072639.314159265Z");
    auto t = to_gmtime(tp);
    BOOST_CHECK_EQUAL(t.tm_sec, 39);
    BOOST_CHECK_EQUAL(t.tm_min, 26);
    BOOST_CHECK_EQUAL(t.tm_hour, 7);
    BOOST_CHECK_EQUAL(t.tm_mday, 2);
    BOOST_CHECK_EQUAL(t.tm_mon, 4 - 1);
    BOOST_CHECK_EQUAL(t.tm_year, 2009 - 1900);
    BOOST_CHECK_EQUAL(t.tm_wday, 4);
    BOOST_CHECK_EQUAL(t.tm_yday, 31 + 28 + 31 + 2 - 1);
    BOOST_CHECK_EQUAL(t.tm_isdst, 0);
}

BOOST_AUTO_TEST_CASE(Timespec) {
    auto tp = utc_clock::from_string("20090402T072639.314159265Z");
    auto ts = to_timespec(tp);
    BOOST_CHECK_EQUAL(ts.tv_sec, 1238657199);
    BOOST_CHECK_EQUAL(ts.tv_nsec, 314159265);
}

BOOST_AUTO_TEST_CASE(Timeval) {
    auto tp = utc_clock::from_string("20090402T072639.314159265Z");
    auto tv = to_timeval(tp);
    BOOST_CHECK_EQUAL(tv.tv_sec, 1238657199);
    BOOST_CHECK_EQUAL(tv.tv_usec, 314159);
}

BOOST_AUTO_TEST_CASE(MJD) {
    auto ts = utc_clock::from_mjd(45205.125);
    BOOST_TEST(ts.time_since_epoch().count() == 399006000000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 399006021000000000LL);
    BOOST_TEST(to_mjd(ts).count() == 45205.125, tt::tolerance(1.e-5));
    BOOST_TEST(to_mjd(timescale_cast<tai_clock>(ts)).count() == 45205.125 + 21.0 / 86400.0, tt::tolerance(1.e-5));
}

BOOST_AUTO_TEST_CASE(LeapSecond) {
    auto t0 = utc_clock::from_mjd(45205.);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t0).time_since_epoch() -
                                              t0.time_since_epoch())
                       .count() == 21);
    auto t1 = utc_clock::from_mjd(41498.99);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t1).time_since_epoch() -
                                              t1.time_since_epoch())
                       .count() == 10);
    auto t2 = utc_clock::from_mjd(41499.01);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t2).time_since_epoch() -
                                              t2.time_since_epoch())
                       .count() == 11);
    auto t3 = utc_clock::from_mjd(57203.99);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t3).time_since_epoch() -
                                              t3.time_since_epoch())
                       .count() == 35);
    auto t4 = utc_clock::from_mjd(57204.01);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t4).time_since_epoch() -
                                              t4.time_since_epoch())
                       .count() == 36);
    auto t5 = utc_clock::from_mjd(57000.);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t5).time_since_epoch() -
                                              t5.time_since_epoch())
                       .count() == 35);
    auto t6 = utc_clock::from_mjd(57210.);
    BOOST_TEST(sc::duration_cast<sc::seconds>(timescale_cast<tai_clock>(t6).time_since_epoch() -
                                              t6.time_since_epoch())
                       .count() == 36);
}

BOOST_AUTO_TEST_CASE(Nsecs) {
    auto ts = utc_clock::time_point{sc::nanoseconds{1192755473000000000LL}};
    BOOST_TEST(ts.time_since_epoch().count() == 1192755473000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 1192755506000000000LL);
    BOOST_TEST(to_mjd(ts).count() == 54392.040196759262, tt::tolerance(1.e-5));
}

BOOST_AUTO_TEST_CASE(BoundaryMJD) {
    auto ts = utc_clock::from_mjd(47892.0);
    BOOST_TEST(ts.time_since_epoch().count() == 631152000000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 631152025000000000LL);
    BOOST_TEST(to_mjd(ts).count() == 47892.0, tt::tolerance(1.e-9));
}

BOOST_AUTO_TEST_CASE(CrossBoundaryNsecs) {
    auto ts = utc_clock::time_point{sc::nanoseconds{631151998000000000LL}};
    BOOST_TEST(ts.time_since_epoch().count() == 631151998000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 631152022000000000LL);
}

BOOST_AUTO_TEST_CASE(NsecsTAI) {
    auto ts = tai_clock::time_point{sc::nanoseconds{1192755506000000000LL}};
    BOOST_TEST(timescale_cast<utc_clock>(ts).time_since_epoch().count() == 1192755473000000000LL);
    BOOST_TEST(ts.time_since_epoch().count() == 1192755506000000000LL);
    BOOST_TEST(to_mjd(timescale_cast<utc_clock>(ts)).count() == 54392.040196759262, tt::tolerance(1.e-9));
}

BOOST_AUTO_TEST_CASE(NsecsDefault) {
    auto ts = tai_clock::time_point{sc::nanoseconds{1192755506000000000LL}};
    BOOST_TEST(timescale_cast<utc_clock>(ts).time_since_epoch().count() == 1192755473000000000LL);
    BOOST_TEST(ts.time_since_epoch().count() == 1192755506000000000LL);
    BOOST_TEST(to_mjd(timescale_cast<utc_clock>(ts)).count() == 54392.040196759262, tt::tolerance(1.e-5));
}

BOOST_AUTO_TEST_CASE(IsoEpoch) {
    auto ts = utc_clock::from_string("19700101T000000Z");
    BOOST_TEST(ts.time_since_epoch().count() == 0L);
    BOOST_TEST(to_string(ts) == "1970-01-01T00:00:00.000000000Z");
}

// Test basic ISO string input and output of UTC dates
BOOST_AUTO_TEST_CASE(IsoUTCBasic) {
    // "-" date separator is optional
    // ":" time separator is optional
    // "." or "," may be used as decimal point
    auto ts0 = utc_clock::from_string("2009-04-02T07:26:39.314159265Z");
    BOOST_TEST(ts0.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts0).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts0).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts0) == "2009-04-02T07:26:39.314159265Z");

    auto ts1 = utc_clock::from_string("2009-04-02T07:26:39,314159265Z");
    BOOST_TEST(ts1.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts1).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts1).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts1) == "2009-04-02T07:26:39.314159265Z");

    auto ts2 = utc_clock::from_string("2009-04-02T072639.314159265Z");
    BOOST_TEST(ts2.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts2).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts2).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts2) == "2009-04-02T07:26:39.314159265Z");

    auto ts3 = utc_clock::from_string("2009-04-02T072639,314159265Z");
    BOOST_TEST(ts3.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts3).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts3).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts3) == "2009-04-02T07:26:39.314159265Z");

    auto ts4 = utc_clock::from_string("20090402T07:26:39.314159265Z");
    BOOST_TEST(ts4.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts4).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts4).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts4) == "2009-04-02T07:26:39.314159265Z");

    auto ts5 = utc_clock::from_string("20090402T07:26:39,314159265Z");
    BOOST_TEST(ts5.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts5).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts5).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts5) == "2009-04-02T07:26:39.314159265Z");

    auto ts6 = utc_clock::from_string("20090402T072639.314159265Z");
    BOOST_TEST(ts6.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts6).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts6).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts6) == "2009-04-02T07:26:39.314159265Z");

    auto ts7 = utc_clock::from_string("20090402T072639,314159265Z");
    BOOST_TEST(ts7.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts7).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(timescale_cast<tt_clock>(ts7).time_since_epoch().count() == 1238657265498159265LL);
    BOOST_TEST(to_string(ts7) == "2009-04-02T07:26:39.314159265Z");
}

// Test basic ISO string input and output of TAI and TT dates
BOOST_AUTO_TEST_CASE(IsoNonUTCBasics) {
    // "-" date separator is optional
    // ":" time separator is optional
    // "." or "," may be used as decimal point
    auto ts0 = tai_clock::from_string("2009-04-02T07:26:39.314159265");
    BOOST_TEST(to_string(ts0) == "2009-04-02T07:26:39.314159265");

    auto ts1 = tai_clock::from_string("2009-04-02T07:26:39,314159265");
    BOOST_TEST(to_string(ts1) == "2009-04-02T07:26:39.314159265");

    auto ts2 = tai_clock::from_string("2009-04-02T072639.314159265");
    BOOST_TEST(to_string(ts2) == "2009-04-02T07:26:39.314159265");

    auto ts3 = tai_clock::from_string("2009-04-02T072639,314159265");
    BOOST_TEST(to_string(ts3) == "2009-04-02T07:26:39.314159265");

    auto ts4 = tai_clock::from_string("20090402T07:26:39.314159265");
    BOOST_TEST(to_string(ts4) == "2009-04-02T07:26:39.314159265");

    auto ts5 = tai_clock::from_string("20090402T07:26:39,314159265");
    BOOST_TEST(to_string(ts5) == "2009-04-02T07:26:39.314159265");

    auto ts6 = tai_clock::from_string("20090402T072639.314159265");
    BOOST_TEST(to_string(ts6) == "2009-04-02T07:26:39.314159265");

    auto ts7 = tai_clock::from_string("20090402T072639,314159265");
    BOOST_TEST(to_string(ts7) == "2009-04-02T07:26:39.314159265");

    auto ts8 = tt_clock::from_string("2009-04-02T07:26:39.314159265");
    BOOST_TEST(to_string(ts8) == "2009-04-02T07:26:39.314159265");

    auto ts9 = tt_clock::from_string("2009-04-02T07:26:39,314159265");
    BOOST_TEST(to_string(ts9) == "2009-04-02T07:26:39.314159265");

    auto ts10 = tt_clock::from_string("2009-04-02T072639.314159265");
    BOOST_TEST(to_string(ts10) == "2009-04-02T07:26:39.314159265");

    auto ts11 = tt_clock::from_string("2009-04-02T072639,314159265");
    BOOST_TEST(to_string(ts11) == "2009-04-02T07:26:39.314159265");

    auto ts12 = tt_clock::from_string("20090402T07:26:39.314159265");
    BOOST_TEST(to_string(ts12) == "2009-04-02T07:26:39.314159265");

    auto ts13 = tt_clock::from_string("20090402T07:26:39,314159265");
    BOOST_TEST(to_string(ts13) == "2009-04-02T07:26:39.314159265");

    auto ts14 = tt_clock::from_string("20090402T072639.314159265");
    BOOST_TEST(to_string(ts14) == "2009-04-02T07:26:39.314159265");

    auto ts15 = tt_clock::from_string("20090402T072639,314159265");
    BOOST_TEST(to_string(ts15) == "2009-04-02T07:26:39.314159265");
}

BOOST_AUTO_TEST_CASE(IsoExpanded) {
    auto ts = utc_clock::from_string("2009-04-02T07:26:39.314159265Z");
    BOOST_TEST(ts.time_since_epoch().count() == 1238657199314159265LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 1238657233314159265LL);
    BOOST_TEST(to_string(ts) == "2009-04-02T07:26:39.314159265Z");
}

BOOST_AUTO_TEST_CASE(IsoNoNSecs) {
    auto ts = utc_clock::from_string("2009-04-02T07:26:39Z");
    BOOST_TEST(ts.time_since_epoch().count() == 1238657199000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 1238657233000000000LL);
    BOOST_TEST(to_string(ts) == "2009-04-02T07:26:39.000000000Z");
}

// Test that a date later than 2038-01-19, 03:14:07 does not wrap around
// This will fail on old versions of unix, and indicates that astrochrono is not safe
BOOST_AUTO_TEST_CASE(Wraparound) {
    auto date = "2040-01-01T00:00:00.000000000";
    BOOST_TEST_REQUIRE(to_string(tai_clock::from_string(date)) == date);
}

// Date with unix time = -1 seconds must be usable

// Note that the call in question parses the ISO string without paying
// attention to the scale (it applies the scale later),
// so the same ISO string is wanted in all cases
// (except with a trailing Z for UTC, and without for TAI and TT)
BOOST_AUTO_TEST_CASE(UnixMinusOne) {
    BOOST_TEST_REQUIRE(sc::duration_cast<sc::seconds>(
                               utc_clock::from_string("1969-12-31T23:59:59.000000000Z").time_since_epoch())
                               .count() == -1);
    BOOST_TEST_REQUIRE(sc::duration_cast<sc::seconds>(
                               tai_clock::from_string("1969-12-31T23:59:59.000000000").time_since_epoch())
                               .count() == -1);
    BOOST_TEST_REQUIRE(sc::duration_cast<sc::seconds>(
                               tt_clock::from_string("1969-12-31T23:59:59.000000000").time_since_epoch())
                               .count() == -1);
}

BOOST_AUTO_TEST_CASE(Str) {
    auto timeStr1 = "2004-03-01T12:39:45.1";
    auto fullTimeStr1 = "2004-03-01T12:39:45.100000000";
    auto dt1 = tai_clock::from_string(timeStr1);
    BOOST_TEST(to_string(dt1) == fullTimeStr1);

    auto timeStr2 = "2004-03-01T12:39:45.000000001";
    auto dt2 = tai_clock::from_string(timeStr2);
    BOOST_TEST(to_string(dt2) == timeStr2);
}

BOOST_AUTO_TEST_CASE(NsecsTT) {
    auto ts = tt_clock::time_point{sc::nanoseconds{1192755538184000000}};
    BOOST_TEST(timescale_cast<utc_clock>(ts).time_since_epoch().count() == 1192755473000000000LL);
    BOOST_TEST(timescale_cast<tai_clock>(ts).time_since_epoch().count() == 1192755506000000000LL);
    BOOST_TEST(ts.time_since_epoch().count() == 1192755538184000000LL);
    BOOST_TEST(to_mjd(timescale_cast<utc_clock>(ts)).count() == 54392.040196759262, tt::tolerance(1.e-5));
}

BOOST_AUTO_TEST_CASE(FracSecs) {
    auto ts0 = utc_clock::from_string("2004-03-01T12:39:45.1Z");
    BOOST_TEST(to_string(ts0) == "2004-03-01T12:39:45.100000000Z");
    auto ts1 = utc_clock::from_string("2004-03-01T12:39:45.01Z");
    BOOST_TEST(to_string(ts1) == "2004-03-01T12:39:45.010000000Z");
    auto ts2 = utc_clock::from_string("2004-03-01T12:39:45.000000001Z");  // nanosecond
    BOOST_TEST(to_string(ts2) == "2004-03-01T12:39:45.000000001Z");
    auto ts3 = utc_clock::from_string("2004-03-01T12:39:45.0000000001Z");  // too small
    BOOST_TEST(to_string(ts3) == "2004-03-01T12:39:45.000000000Z");
}

BOOST_AUTO_TEST_CASE(Negative) {
    auto ts0 = utc_clock::from_string("1969-03-01T00:00:32Z");
    BOOST_TEST(to_string(ts0) == "1969-03-01T00:00:32.000000000Z");
    auto ts1 = utc_clock::from_string("1969-01-01T00:00:00Z");
    BOOST_TEST(to_string(ts1) == "1969-01-01T00:00:00.000000000Z");
    auto ts2 = utc_clock::from_string("1969-01-01T00:00:40Z");
    BOOST_TEST(to_string(ts2) == "1969-01-01T00:00:40.000000000Z");
    auto ts3 = utc_clock::from_string("1969-01-01T00:00:38Z");
    BOOST_TEST(to_string(ts3) == "1969-01-01T00:00:38.000000000Z");
    auto ts4 = utc_clock::from_string("1969-03-01T12:39:45Z");
    BOOST_TEST(to_string(ts4) == "1969-03-01T12:39:45.000000000Z");
    auto ts5 = utc_clock::from_string("1969-03-01T12:39:45.000000001Z");
    BOOST_TEST(to_string(ts5) == "1969-03-01T12:39:45.000000001Z");

    // UTC-TAI-UTC round-trip
    // Note that in contrast to LSST DateTime, round tripping is exact (don't know why yet)
    auto ts6 = timescale_cast<tai_clock>(utc_clock::from_string("1969-03-01T12:39:45.12345Z"));
    BOOST_TEST(ts6.time_since_epoch().count() == -26392807668252446LL);
    BOOST_TEST(to_string(timescale_cast<utc_clock>(ts6)) == "1969-03-01T12:39:45.123450000Z");
    auto ts7 = timescale_cast<tai_clock>(utc_clock::from_string("1969-03-01T12:39:45.123456Z"));
    BOOST_TEST(ts7.time_since_epoch().count() == -26392807668246446LL);
    BOOST_TEST(to_string(timescale_cast<utc_clock>(ts7)) == "1969-03-01T12:39:45.123456000Z");

    auto ts8 = tai_clock::time_point{sc::nanoseconds{-1}};
    BOOST_TEST(to_string(timescale_cast<utc_clock>(ts8)) == "1969-12-31T23:59:51.999918239Z");
    auto ts9 = tai_clock::time_point{sc::nanoseconds{0}};
    BOOST_TEST(to_string(timescale_cast<utc_clock>(ts9)) == "1969-12-31T23:59:51.999918240Z");
    auto ts10 = tai_clock::time_point{sc::nanoseconds{1}};
    BOOST_TEST(to_string(timescale_cast<utc_clock>(ts10)) == "1969-12-31T23:59:51.999918241Z");

    auto ts11 = utc_clock::time_point{sc::nanoseconds{-1}};
    BOOST_TEST(to_string(ts11) == "1969-12-31T23:59:59.999999999Z");
    auto ts12 = utc_clock::time_point{sc::nanoseconds{0}};
    BOOST_TEST(to_string(ts12) == "1970-01-01T00:00:00.000000000Z");
    auto ts13 = utc_clock::time_point{sc::nanoseconds{1}};
    BOOST_TEST(to_string(ts13) == "1970-01-01T00:00:00.000000001Z");
}

BOOST_AUTO_TEST_CASE(IsoThrow) {
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04-01T23:36:05"),
                      std::invalid_argument);  // Z time zone required for UTC
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04-01T23:36:05Z"),
                      std::invalid_argument);  // Z time zone forbidden for TAI
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04-01T23:36:05Z"),
                      std::invalid_argument);  // Z time zone forbidden for TT

    BOOST_CHECK_THROW(utc_clock::from_string("20090401"), std::invalid_argument);     // time required
    BOOST_CHECK_THROW(utc_clock::from_string("20090401T"), std::invalid_argument);    // time required
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04-01T"), std::invalid_argument);  // time required
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04-01T23:36:05-0700"),
                      std::invalid_argument);  // time zone offset not supported
    BOOST_CHECK_THROW(utc_clock::from_string("2009/04/01T23:36:05Z"),
                      std::invalid_argument);                                              // "/" not valid
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04-01T23:36"), std::invalid_argument);  // partial time
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04"), std::invalid_argument);  // partial date without time
    BOOST_CHECK_THROW(utc_clock::from_string("2009-04T23:36.05"),
                      std::invalid_argument);  // partial date with time
    BOOST_CHECK_THROW(utc_clock::from_string("09-04-01T23:36:05"), std::invalid_argument);  // 2 digit year

    BOOST_CHECK_THROW(tai_clock::from_string("20090401"), std::invalid_argument);     // time required
    BOOST_CHECK_THROW(tai_clock::from_string("20090401T"), std::invalid_argument);    // time required
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04-01T"), std::invalid_argument);  // time required
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04-01T23:36:05-0700"),
                      std::invalid_argument);  // time zone offset not supported
    BOOST_CHECK_THROW(tai_clock::from_string("2009/04/01T23:36:05Z"),
                      std::invalid_argument);                                              // "/" not valid
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04-01T23:36"), std::invalid_argument);  // partial time
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04"), std::invalid_argument);  // partial date without time
    BOOST_CHECK_THROW(tai_clock::from_string("2009-04T23:36.05"),
                      std::invalid_argument);  // partial date with time
    BOOST_CHECK_THROW(tai_clock::from_string("09-04-01T23:36:05"), std::invalid_argument);  // 2 digit year

    BOOST_CHECK_THROW(tt_clock::from_string("20090401"), std::invalid_argument);     // time required
    BOOST_CHECK_THROW(tt_clock::from_string("20090401T"), std::invalid_argument);    // time required
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04-01T"), std::invalid_argument);  // time required
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04-01T23:36:05-0700"),
                      std::invalid_argument);  // time zone offset not supported
    BOOST_CHECK_THROW(tt_clock::from_string("2009/04/01T23:36:05Z"), std::invalid_argument);  // "/" not valid
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04-01T23:36"), std::invalid_argument);      // partial time
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04"), std::invalid_argument);  // partial date without time
    BOOST_CHECK_THROW(tt_clock::from_string("2009-04T23:36.05"),
                      std::invalid_argument);  // partial date with time
    BOOST_CHECK_THROW(tt_clock::from_string("09-04-01T23:36:05"), std::invalid_argument);  // 2 digit year

    // earliest allowed UTC to TAI conversion is the earliest date in the leap second table
    BOOST_CHECK_NO_THROW(timescale_cast<tai_clock>(utc_clock::from_string("1961-01-01T00:00:00Z")));
    // just before leap second table starts
    BOOST_CHECK_THROW(timescale_cast<tai_clock>(utc_clock::from_string("1960-01-01T23:59:59Z")),
                      std::domain_error);

    // earliest allowed date for TAI and TT is year = 1902
    BOOST_CHECK_NO_THROW(to_string(tai_clock::from_string("1902-01-01T00:00:00")));
    BOOST_CHECK_NO_THROW(to_string(tt_clock::from_string("1902-01-01T00:00:00")));

    // dates before the leap second table can be created using TAI or TT, but not converted to UTC
    BOOST_CHECK_NO_THROW(tai_clock::from_string("1960-01-01T00:00:00"));
    BOOST_CHECK_NO_THROW(tt_clock::from_string("1960-01-01T00:00:00"));
    BOOST_CHECK_THROW(timescale_cast<utc_clock>(tai_clock::from_string("1960-01-01T00:00:00")),
                      std::domain_error);
    BOOST_CHECK_THROW(timescale_cast<utc_clock>(tt_clock::from_string("1960-01-01T00:00:00")),
                      std::domain_error);

    BOOST_CHECK_THROW(tai_clock::from_string("1901-12-12T23:59:59Z"), std::invalid_argument);  // too early
    BOOST_CHECK_THROW(tai_clock::from_string("1700-01-01T00:00:00Z"),
                      std::invalid_argument);  // way too early
    BOOST_CHECK_THROW(tai_clock::from_string("2262-01-01T00:00:00Z"), std::invalid_argument);  // too late
    BOOST_CHECK_THROW(tai_clock::from_string("3200-01-01T00:00:00Z"), std::invalid_argument);  // way too late
}

BOOST_AUTO_TEST_CASE(InvalidDate) {
    // Date before UTC->TAI conversion is valid
    BOOST_CHECK_THROW(
            timescale_cast<tai_clock>(utc_clock::time_point{sc::nanoseconds{-500000000 * 1000000000LL}}),
            std::domain_error);
    // Date before UTC->TAI conversion is valid and too far in the past for
    // 32-bit Unix mktime()
    BOOST_CHECK_THROW(timescale_cast<tai_clock>(utc_clock::from_string("1901-01-01T12:34:56Z")),
                      std::domain_error);
    if (sizeof(time_t) == 4) {
        // Date too far in the past for Unix mktime()
        BOOST_CHECK_THROW(utc_clock::from_calendar(1901, 1, 1, 12, 34, 56), std::domain_error);
        // Date too far in the future for Unix mktime()
        BOOST_CHECK_THROW(utc_clock::from_calendar(2039, 1, 1, 12, 34, 56), std::domain_error);
        BOOST_CHECK_THROW(timescale_cast<tai_clock>(utc_clock::from_string("2039-01-01T12:34:56Z")),
                          std::domain_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()
