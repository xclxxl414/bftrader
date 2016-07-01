/***********************************************************************************
	dtz.h - 23/08/2002, kliff

	Few simple classes to work with date, time, datetime and timezone
	All date-time algorithm is taken from SFL library (www.imatix.com)

	multithread safe is NOT IMPLEMENTED YET!!!

  2002-08-28, kliff - created

  2002-09-11, A Kirwan (AKA) Some minor fixes, including handling of Southern Hemisphere Timezones.

  2002-09-13, kliff - few bug fixes, added implementetion to_gm_timer() without _mkgmtime

  2003-1-10, Wout Louwers - Correction the function bool _tzinfo_t::is_dst(...) when (dto == d_date) 
  (with compare to TzSpecificLocalTimeToSystemTime)

  2003-1-23, kliff - added DATE

  2003-2-25, Joel Rowles - fixed
  
  2003-10-18, kliff - remove os.h dependencies, added search functionality to _tzlist_t, minor changes

***********************************************************************************/

// -------------------------------------------------------------------<Prolog>-
//    Name:       sfldate.h
//    Title:      Date and time functions
//    Package:    Standard Function Library (SFL)
//
//    Written:    1996/01/05  iMatix SFL project team <sfl@imatix.com>
//    Revised:    1998/08/05
//
//    Synopsis:   Includes functions to get the current date/time, calculate
//                the day or week, week of year and leap year.  Dates and times
//                are each stored in a 32-bit long value of 8 digits: dates are
//                CCYYMMDD; times are HHMMSSCC.  You can compare dates and times
//                directly - e.g. if (date_wanted >= date_now).
//
//    Copyright:  Copyright (c) 1996-2000 iMatix Corporation
//    License:    This is free software; you can redistribute it and/or modify
//                it under the terms of the SFL License Agreement as provided
//                in the file LICENSE.TXT.  This software is distributed in
//                the hope that it will be useful, but without any warranty.
// ------------------------------------------------------------------</Prolog>-

#pragma once

#include <time.h>
#include <tchar.h>
#include <wtypes.h>
#include <windows.h>
#include <vector>
#include <oleauto.h>

#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(*a))
#endif

//prevent conflict with _time_t::min()
#undef min

namespace dtz // namespace required in order to resolve conflicting names (e.g. while using with DbcSymbology)
{
	// Days are numbered from 0 = Sunday to 6 = Saturday
	enum
	{
		Sunday = 0,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
	};

	//month are numbered from 1
	enum
	{
		Jan = 1,
		Feb,
		Mar,
		Apr,
		May,
		Jun,
		Jul,
		Aug,
		Sep,
		Oct,
		Nov,
		Dec,
	};
}

//Interval values, specified in centiseconds

#define INTERVAL_CENTI      1
#define INTERVAL_SEC        100
#define INTERVAL_MIN        6000
#define INTERVAL_HOUR       360000L
#define INTERVAL_DAY        8640000L

#define INTERVAL_SEC_SEC	1
#define INTERVAL_MIN_SEC	60
#define INTERVAL_HOUR_SEC	3600
#define INTERVAL_DAY_SEC	86400

//return number of days calculating from some epoch
//taken from SFL
inline long to_days(int day, int month, int year)
{
	long
		_year    = year % 100,
		_century = year / 100,
		_month   = month,
		_day     = day;

	if (_month > 2)
	{
		_month -= 3;
	}
	else
	{
		_month += 9;
		if (_year)
		{
			_year--;
		}
		else
		{
			_year = 99;
			_century--;
		}
	}
	return ((146097L * _century)    / 4L +
		(1461L   * _year)       / 4L +
		(153L    * _month + 2L) / 5L +
		_day   + 1721119L);
}

//convert from number of days from previous function to normal day-month-year format
//taken from SFL
inline void from_days(long days, int &day, int &month, int &year)
{
	long
		_century,
		_year,
		_month,
		_day;

	days   -= 1721119L;
	_century = (4L * days - 1L) / 146097L;
	days    =  4L * days - 1L  - 146097L * _century;
	_day     =  days / 4L;

	_year    = (4L * _day + 3L) / 1461L;
	_day     =  4L * _day + 3L  - 1461L * _year;
	_day     = (_day + 4L) / 4L;

	_month   = (5L * _day - 3L) / 153L;
	_day     =  5L * _day - 3   - 153L * _month;
	_day     = (_day + 5L) / 5L;

	if (_month < 10)
	{
		_month += 3;
	}
	else
	{
		_month -= 9;
		if (_year++ == 99)
		{
			_year = 0;
			_century++;
		}
	}
	day = _day;
	month = _month;
	year = _year + _century * 100;
}

//////////////////////////////////////////////////////////////////////////////
// Object of type _time_t may be time or time span at the same moment.      //
// To check for valid time value use valid() (not for time span)            //
//////////////////////////////////////////////////////////////////////////////

class _time_t
{
public:
	_time_t(): _tm(0) {}
	_time_t(int hour, int min, int sec, int csec=0): _tm(0) { set_time(hour, min, sec, csec); }
	_time_t(const _time_t &to): _tm(0) { *this = to; }
	_time_t(const tm &ts): _tm(0) { *this = ts; }
	_time_t(long csecs): _tm(0) {	*this = csecs; }
	_time_t(const SYSTEMTIME &st): _tm(0) { *this = st; }
	_time_t(DATE dt): _tm(0) { *this = dt; }
public:
	//return current system time (in current system timezone)
	static _time_t now() 
	{
		SYSTEMTIME st; GetLocalTime (&st);
		return _time_t(st);
	}

	static _time_t gmt_now() 
	{
		SYSTEMTIME st; GetSystemTime (&st);
		return _time_t(st);
	}

public:
	//return current hour
	int hour() const {return (int) ( (_tm) / 1000000L);}

	//retunr current minute
	int min() const {return (int) (((_tm) % 1000000L) / 10000L);}

	//return current seconds
	int sec() const {return (int) (((_tm) % 10000L) / 100);}

	//return current centisecs
	int csec() const {return (int) ( (_tm) % 100);}

	//is valid
	bool valid() const 
	{
		return (sec() < 60
			&&  min() < 60
			&&  hour() < 24);
	}

	_time_t &set_hour(int _hour) { _tm += (_hour - hour()) * 1000000L ; return *this;	}
	_time_t &set_min(int _min) { _tm += (_min - min()) * 10000L; return *this; }
	_time_t &set_sec(int _sec) { _tm += (_sec - sec()) * 100L; return *this; }
	_time_t &set_csec(int _csec) { _tm += _csec - csec(); return *this; }
	_time_t &set_time(int hour, int min, int sec, int csec=0) 
	{
		set_hour(hour);
		set_min(min);
		set_sec(sec);
		set_csec(csec);
		return *this;
	}

	_time_t &operator =(const _time_t &to) { _tm = to._tm; return *this; }
	_time_t &operator =(long csecs) {	return from_csecs(csecs); }
	_time_t &operator =(const tm &ts) { set_time(ts.tm_hour, ts.tm_min,ts.tm_sec); return *this; }
	_time_t &operator =(const SYSTEMTIME &st) { set_time(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds / 10); return *this; }
	_time_t &operator =(DATE vt) { SYSTEMTIME st; VariantTimeToSystemTime(vt, &st); return operator =(st); }

	_time_t &operator +=(long csecs) { return from_csecs(to_csecs() + csecs); }
	_time_t &operator -=(long csecs) { return from_csecs(to_csecs() - csecs); }

	_time_t operator -(const _time_t &to) const { return to_csecs() - to.to_csecs(); }
	_time_t operator +(const _time_t &to) const	{ return to_csecs() + to.to_csecs(); }

	bool operator ==(const _time_t &to) const { return (_tm == to._tm); }
	bool operator <(const _time_t &to) const { return (_tm < to._tm); }

   // AKA 2002-09-11 Added ">" operator
	bool operator >(const _time_t &to) const { return (_tm > to._tm); }

	bool operator <=(const _time_t &to) const { return *this < to || *this == to; }
	bool operator >=(const _time_t &to) const { return *this > to || *this == to; }

	long to_csecs() const 
	{
		return ((long) (hour() * (long) INTERVAL_HOUR)
			  + (long) (min() * (long) INTERVAL_MIN)
			  + (long) (sec() * (long) INTERVAL_SEC)
			  + (long) (csec()));
	} 
	_time_t &from_csecs(long csecs, long *days = NULL)
	{
		long _hour, _min, _sec;

		if (days)
		{
			*days = csecs/INTERVAL_DAY;
			csecs = csecs % INTERVAL_DAY;
			if (csecs < 0)
				csecs += INTERVAL_DAY, (*days) --;
		}

		_hour  = csecs / INTERVAL_HOUR;
		csecs = csecs % INTERVAL_HOUR;
		_min   = csecs / INTERVAL_MIN;
		csecs = csecs % INTERVAL_MIN;
		_sec   = csecs / INTERVAL_SEC;
		csecs = csecs % INTERVAL_SEC;

		set_time(_hour, _min, _sec, csecs);

		return *this;
	}
	
	long raw_time() const {return _tm;}
	
	void get(tm &ts) const { ts.tm_hour = hour(); ts.tm_min = min(); ts.tm_sec = sec(); ts.tm_isdst = -1;}
	
	void get(SYSTEMTIME &st) const 
	{ 
		st.wHour = (WORD) hour(); 
		st.wMinute = (WORD) min(); 
		st.wSecond = (WORD) sec(); 
		st.wMilliseconds = (WORD)(csec() * 10);
	}

	time_t to_gmt_timer() const
	{
		return to_csecs() / INTERVAL_SEC;
	}
	
	time_t to_local_timer() const
	{
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		return mktime(&ts);
	}

	_time_t &from_gmt_timer(time_t timer)
	{
		tm ts;
		gmtime_s(&ts, &timer);
		
		*this = ts;
		return *this;
	}
	
	_time_t &from_local_timer(time_t timer)
	{
		tm ts;
		localtime_s(&ts, &timer);
		
		*this = ts;
		return *this;
	}

	LPCTSTR fmt(LPCTSTR sfmt = _TEXT("%H:%M:%S")) const
	{
		static TCHAR buff[1024];
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		_tcsftime(buff, _countof(buff)-1, sfmt, &ts);
		return buff;
	}
protected:
	long _tm;
};

//////////////////////////////////////////////////////////////////////////////
// Object of type _date_t may be date or date span at the same moment.      //
// To check for valid date value use valid() (not for date span)            //
//////////////////////////////////////////////////////////////////////////////
class _date_t
{
public:
	_date_t(): _dt(0) {}
	_date_t(int day, int month, int year): _dt(0) { set_date(day, month, year);}
	_date_t(const _date_t &dc): _dt(0) { *this = dc; }
	_date_t(const tm &ts): _dt(0) { *this = ts; }
	_date_t(long days): _dt(0) { *this = days; }
	_date_t(const SYSTEMTIME &st): _dt(0) { *this = st; }
	_date_t(DATE dt): _dt(0) { *this = dt; }

public:
	//return current system date (in current system timezone)
	//to convert it to GMT use _timezone object
	static _date_t now()
	{
		SYSTEMTIME st; GetLocalTime (&st);
		return _date_t(st);
	}
	//return current time in GMT
	static _date_t gmt_now()
	{
		SYSTEMTIME st; GetSystemTime (&st);
		return _date_t(st);
	}

	//return number of days in month (1 - january, 2 - february, ...)
	//for february allways return 29 - use month_days(month, year)
	static BYTE month_days(int month)
	{
		static BYTE _month_days [] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		return _month_days[month-1];
	}

	// return number of day in month
	// handle the leap year
	static BYTE month_days(int month, int year)
	{
		return (month == 2 && !leap_year(year))? month_days(month)-1: month_days(month);
	}
	
	//return month days accumulated with leap year correction
	static long month_days_acc(int month, int year)
	{
		static long
			_days_norm [13] = {
			0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
		};

		static long
			_days_leap [13] = {
			0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366
		};
		return leap_year(year) ? _days_leap[month]: _days_norm[month];

	}

	//return number of days in year
	static long year_days(int year)
	{
		return leap_year(year) ? 366: 365;
	}
	
	static bool leap_year(int year)  
	{
		return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
	}

public:
	//return current century (ex. 20)
	int century() const {return (int) ( (_dt) / 1000000L);}

	//return current year without century digits (ex. 67)
	int ccyear() const {return (int) (((_dt) % 1000000L) / 10000L);}

	//return current year (ex. 1967)
	int year() const {return (int) ( (_dt) / 10000L);}

	//return current month (0-january, etc..)
	int month() const {return (int) (((_dt) % 10000L) / 100);}

	//return current day 
	int day() const {return (int) ( (_dt) % 100);}

	//return number of days from 31 december last year
	int julian() const 
	{
		static int
			_days [12] = {
			0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
		};
		int
			_julian;

		_julian = _days [month() - 1] + day();
		if (month() > 2 && leap_year (year()))
			_julian++;

		return (_julian);
	}

	//return total number of days
	long year_julian() const { return to_days() - _date_t(0,0,0).to_days(); }

	//return current day of week (0-sunday, 1-monday, etc..)
	int day_of_week() const 
	{
		int
			_year  = year(),
			_month = month(),
			_day   = day();

		if (_month > 2)
		{
			_month -= 2;
		}
		else
		{
			_month += 10;
			_year--;
		}
		_day = ((13 * _month - 1) / 5) + _day + (_year % 100) +
			  ((_year % 100) / 4) + ((_year / 100) / 4) - 2 *
			   (_year / 100) + 77;

		return (_day - 7 * (_day / 7));
	}

	// set day according to week of day in selected month
	// day - week of day (0 - sunday, 1 - monday, ...)
	// week - number of week in month (0 - first week, 5 - last week)
	_date_t &set_day_of_week(int _day_of_week, int _week, int _month, int _year)
	{
		int _day, _first_day_of_week;

		set_date(1, _month, _year);

		//normalize
		_day_of_week %= 7;

		_first_day_of_week = day_of_week();

		// AKA 2002-09-11 Implemented slightly different approach to calculating
		// the last occurrence of a particular day.
		bool b_last_week = (_week > 4);

		if (b_last_week)
			_week = 4;

		if (_day_of_week >= _first_day_of_week)
			_week --;

		_day = _week * 7 + _day_of_week - _first_day_of_week + 1;
	
		// check month range 
		if ((_day <= 0) || 
			(b_last_week && (_day+7) <= month_days(_month, _year)))
			_day += 7;
	
		return set_day(_day);
	}

	//return number of full week in current year
	long week_of_year() const 
	{
		long
			_year = year() - 1501,
			_day  = _year * 365 + _year / 4 - 29872L + 1 - _year / 100 + (_year - 300) / 400;

		return ((julian() + (int) ((_day + 4) % 7)) / 7);
	}

	//return year quarter
	int year_quarter() const 
	{
	    return ((month() - 1) / 3 + 1);
	}

	//check if date is valid ()
	bool valid() const 
	{
		int	_month,	_day;
		bool feedback;

		_month = month();
		_day   = day();

		if (_dt == 0)
			feedback = true;                /*  Zero date is okay                */
		else
		if (_month < 1 || _month > 12)
			feedback = false;               /*  _month out of range               */
		else
		if ((_day < 1 || _day > month_days(_month))
		||  (_month == 2 && _day == 29 && !leap_year (year())))
			feedback = false;               /*  _day out of range                 */
		else
			feedback = true;                /*  Zero date is okay                */

		return (feedback);
	}

	_date_t &set_century(int _century)
	{
		_dt += (_century - century()) * 1000000L ;
		return *this;
	}
	_date_t &set_ccyear(int _ccyear)
	{
		_dt += (_ccyear - ccyear()) * 10000L ;
		return *this;
	}

	//if give it negative value - no change (ex. set(2001, -1, 3))
	_date_t &set_year(int _year)
	{
		set_century(_year / 100);
		set_ccyear(_year % 100);
		return *this;
	}
	_date_t &set_month(int _month)
	{
		_dt += (_month - month()) * 100 ;
		return *this;
	}
	_date_t &set_day(int _day)
	{
		_dt += _day - day();
		return *this;
	}
	_date_t &set_date(int day, int month, int year) 
	{
		set_year(year);
		set_month(month);
		set_day(day);
		return *this;
	}
	_date_t &set_julian(int julian, int year)
	{
		int month, day;

		while ( year_days(year)<julian )
		{
			year++;
			julian-=year_days(year);
		}

		for (month=1,day=julian; month<12; month++)
		{
			int _month_days = month_days(month, year);
			if (day <= _month_days)
				break;
			day -= _month_days;
		}

		set_date(day, month, year);

		return *this;
	}
	_date_t &operator =(const _date_t &dc) 
	{
		_dt = dc._dt;
		return *this;
	}
//	_date_t &operator =(const time_t timer) 
//	{
//		*this = *gmtime(&timer);
//		return *this;
//	}
	_date_t &operator =(DATE vt)
	{
		SYSTEMTIME st; VariantTimeToSystemTime(vt, &st); 
		return operator =(st);
	}
	_date_t &operator =(long days) 
	{
		return from_days(days);
	}
	_date_t &operator =(const tm &ts) 
	{
		set_date(ts.tm_mday, ts.tm_mon+1, ts.tm_year+1900);
		return *this;
	}
	_date_t &operator =(const SYSTEMTIME &st) 
	{
		set_date(st.wDay, st.wMonth, st.wYear);
		return *this;
	}
	_date_t &operator +=(long days)
	{
		return from_days(to_days() + days);
	}
	_date_t &operator -=(long days)
	{
		return from_days(to_days() - days);
	}
	_date_t operator -(const _date_t &dc) const
	{
		return _date_t().from_days( to_days() - dc.year_julian());
	}
	_date_t operator +(const _date_t &dc) const
	{
		return _date_t().from_days(to_days() + dc.year_julian());
	}

	bool operator ==(const _date_t &dc) const { return (_dt == dc._dt); }
	bool operator <(const _date_t &dc) const { return (_dt < dc._dt); }

	// AKA 2002-09-11 Added ">" operator
	bool operator >(const _date_t &dc) const { return (_dt > dc._dt); }

	bool operator <=(const _date_t &dc) const { return *this < dc || *this == dc; }
	bool operator >=(const _date_t &dc) const { return *this > dc || *this == dc; }

	void get(tm &ts) const
	{
		ts.tm_year = year()-1900;
		ts.tm_mon = month()-1;
		ts.tm_mday = day();
		ts.tm_wday = day_of_week();
		ts.tm_yday = julian();
		ts.tm_isdst = -1;
	}

	void get(SYSTEMTIME &st) const
	{
		st.wYear = (WORD) year();
		st.wMonth = (WORD) month();
		st.wDay = (WORD) day();
		st.wDayOfWeek = (WORD) day_of_week();
	}
	
	void get(DATE& dt)
	{
		SYSTEMTIME st = {0};
		get(st);
		dt = 0;
		SystemTimeToVariantTime(&st, &dt);
	}
	long to_days() const 
	{
		return ::to_days(day(), month(), year());
	}

	_date_t &from_days(long days) 
	{
		int
			_year,
			_month,
			_day;

		::from_days(days, _day, _month, _year);

		return set_date(_day, _month, _year);
	}

	time_t to_gmt_timer() const
	{
		return (to_days() - _date_t(1, 1, 1970).to_days()) * INTERVAL_DAY_SEC;
	}

	time_t to_local_timer() const
	{
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		return mktime(&ts);
	}

	_date_t &from_gmt_timer(time_t timer)
	{
		tm ts;
		gmtime_s(&ts, &timer);
		
		*this = ts;
		return *this;
	}

	_date_t &from_local_timer(time_t timer)
	{
		tm ts;
		localtime_s(&ts, &timer);

		*this = ts;
		return *this;
	}

	long raw_date() const 
	{ 
		return _dt; 
	}

	LPCTSTR fmt(LPCTSTR sfmt = _TEXT("%Y/%m/%d")) const
	{
		static TCHAR buff[1024];
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		_tcsftime(buff, _countof(buff)-1, sfmt, &ts);
		return buff;
	}
protected:
	long _dt;
};

class _datetime_t: public _date_t, public _time_t
{
public:
	_datetime_t() {}
	_datetime_t(int hour, int min, int sec, int day, int mon, int year)
	{
		set(hour, min, sec, day, mon, year);
	}
	_datetime_t(int hour, int min, int sec, int csec, int day, int mon, int year)
	{
		set(hour, min, sec, csec, day, mon, year);
	}
	_datetime_t(long csecs, long days)
	{
		from_csecs_days(csecs, days);
	}
	_datetime_t(const _datetime_t &dto)
	{
		*this = dto;
	}
	_datetime_t(const _time_t &to)
	{
		*this = to;
	}
	_datetime_t(const _date_t &dt)
	{
		*this = dt;
	}
	_datetime_t(const _time_t &to, const _date_t &dt)
	{
		*this = to;
		*this = dt;
	}
	_datetime_t(const tm &ts)
	{
		*this = ts;
	}
	_datetime_t(DATE vt)
	{
		*this = vt;
	}
	_datetime_t(const SYSTEMTIME & st)
	{
		*this = st;
	}
public:
	static _datetime_t now() 
	{
		_datetime_t dto; 
		dto = _time_t::now();
		dto = _date_t::now();
		return dto;
	}

	static _datetime_t gmt_now() 
	{
		_datetime_t dto; 
		dto = _time_t::gmt_now();
		dto = _date_t::gmt_now();
		return dto;
	}
public:
	bool operator ==(const _datetime_t &dto) const { return date() == dto.date() && time() == dto.time(); }
	bool operator <(const _datetime_t &dto) const { return (date() < dto.date()) || (date() == dto.date() && time() < dto.time()); }

   // AKA 2002-09-11 Added ">" operator
	bool operator >(const _datetime_t &dto) const { return (date() > dto.date()) || (date() == dto.date() && time() > dto.time()); }

	bool operator <=(const _datetime_t &dc) const { return *this < dc || *this == dc; }
	bool operator >=(const _datetime_t &dc) const { return *this > dc || *this == dc; }
	bool is_future() const 
	{
		return *this > now();
	}
	bool is_past() const 
	{
		return *this < now();
	}
	void get(tm &ts) const
	{
		_time_t::get(ts);
		_date_t::get(ts);
	}
	void get(SYSTEMTIME &st) const
	{
		_time_t::get(st);
		_date_t::get(st);
	}
	void get(DATE &dt) const
	{
		SYSTEMTIME st; get(st);
		SystemTimeToVariantTime(&st, &dt);
	}
	operator tm() const
	{
		tm ts; get(ts);
		return ts;
	}
	operator SYSTEMTIME() const
	{
		SYSTEMTIME st; get(st);
		return st;
	}
	operator DATE() const
	{
		DATE dt; get(dt);
		return dt;
	}
	_datetime_t &set(int hour, int min, int sec, int csec, int day, int mon, int year)
	{
		set_time(hour, min, sec, csec);
		set_date(day, mon, year);
		return *this;
	}
	_datetime_t &set(int hour, int min, int sec, int day, int mon, int year)
	{
		set_time(hour, min, sec);
		set_date(day, mon, year);
		return *this;
	}

	_date_t &date() const
		{return (_date_t&)*this;}

	_time_t &time() const
		{return (_time_t&)*this;}

	_date_t &date()
		{return (_date_t&)*this;}

	_time_t &time()
		{return (_time_t&)*this;}

	_datetime_t &operator =(const SYSTEMTIME &st) 
	{
		time() = st;
		date() = st;
		return *this;
	}
	_datetime_t &operator =(const tm &ts) 
	{
		time() = ts;
		date() = ts;
		return *this;
	}
	_datetime_t &operator =(const _datetime_t &dto) 
	{
		time() = dto.time();
		date() = dto.date();
		return *this;
	}
	_datetime_t &operator =(const _time_t &to) 
	{
		time() = to;
		return *this;
	}
	_datetime_t &operator =(const _date_t &dt) 
	{
		date() = dt;
		return *this;
	}
	_datetime_t &operator =(DATE vt) 
	{
		time() = vt;
		date() = vt;
		return *this;
	}
	_datetime_t operator -(const _datetime_t &dto)  const
	{
		return _datetime_t(to_csecs() - dto.to_csecs(), to_days() - dto.to_days());
	}
	_datetime_t operator -(const _date_t &dc)  const
	{
		return _datetime_t(to_csecs(), to_days() - dc.year_julian());
	}
	_datetime_t operator -(const _time_t &to)  const
	{
		return _datetime_t(to_csecs() - to.to_csecs(), to_days());
	}
	_datetime_t operator +(const _datetime_t &dto)  const
	{
		return _datetime_t(to_csecs() + dto.to_csecs(), to_days() + dto.year_julian());
	}

	_datetime_t operator +(const _date_t &dc)  const
	{
		return _datetime_t(to_csecs(), to_days() + dc.to_days());
	}

	_datetime_t operator +(const _time_t &to)  const
	{
		return _datetime_t(to_csecs() + to.to_csecs(), to_days());
	}

	_datetime_t &from_csecs_days(long csecs, long days)
	{
		long days_dlt;
		from_csecs(csecs, &days_dlt);
		from_days(days + days_dlt);
		return *this;
	}

	time_t to_gmt_timer() const
	{
		return date().to_gmt_timer() + time().to_gmt_timer();
	}

	time_t to_local_timer() const
	{
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		return mktime(&ts);
	}

	_datetime_t &from_gmt_timer(time_t timer)
	{
		tm ts;
		gmtime_s(&ts, &timer);

		*this = ts;
		return *this;
	}
	_datetime_t &from_local_timer(time_t timer)
	{
		tm ts;
		localtime_s(&ts, &timer);
		
		*this = ts;
		return *this;
	}
	_datetime_t &norm()
	{
		long csecs = to_csecs();
		long days = to_days();
		return from_csecs_days(csecs, days);
	}
	LPCTSTR fmt(LPCTSTR sfmt = _TEXT("%Y/%m/%d, %H:%M:%S")) const
	{
		static TCHAR buff[1024];
		tm ts; memset(&ts,0,sizeof(tm)); get(ts);
		_tcsftime(buff, _countof(buff)-1, sfmt, &ts);
		return buff;
	}
};


typedef struct 
{
	LONG		Bias;
	LONG		StandardBias;
	LONG		DaylightBias;
	SYSTEMTIME	StandardDate;
	SYSTEMTIME	DaylightDate; 
} TIMEZONE;

typedef struct 
{
	TCHAR		lc_name[128];
	TCHAR		st_name[128];
	TIMEZONE	tzi;
} TIMEZONE_INFO;


class _tzinfo_t: public TIMEZONE
{
public:
	_tzinfo_t() { memset(this, 0, sizeof(*this)); }
	_tzinfo_t( const TIMEZONE &tz ) { *this = tz; }
	_tzinfo_t( const TIME_ZONE_INFORMATION &tz ) { *this = tz; }
	_tzinfo_t( const _tzinfo_t &tz ) { *this = tz; }

public:
	//current timezone
	static _tzinfo_t current() 
	{
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation(&TimeZoneInfo);
		return TimeZoneInfo;
	}

	//Afghanistan
	static _tzinfo_t afghanistan()
	{
		static TIMEZONE _tz = {-270, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Alaskan
	static _tzinfo_t alaskan()
	{
		static TIMEZONE _tz = {540, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Arab
	static _tzinfo_t arab()
	{
		static TIMEZONE _tz = {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Arabian
	static _tzinfo_t arabian()
	{
		static TIMEZONE _tz = {-240, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Arabic
	static _tzinfo_t arabic()
	{
		static TIMEZONE _tz = {-180, 0, -60, {0, 10, 0, 1, 4, 0, 0, 0}, {0, 4, 0, 1, 3, 0, 0, 0} };
		return _tz;
	}
	//Atlantic
	static _tzinfo_t atlantic()
	{
		static TIMEZONE _tz = {240, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//AUS Central
	static _tzinfo_t auscentral()
	{
		static TIMEZONE _tz = {-570, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//AUS Eastern
	static _tzinfo_t auseastern()
	{
		static TIMEZONE _tz = {-600, 0, -60, {0, 3, 0, 5, 3, 0, 0, 0}, {0, 10, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Azores
	static _tzinfo_t azores()
	{
		static TIMEZONE _tz = {60, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Canada Central
	static _tzinfo_t canadacentral()
	{
		static TIMEZONE _tz = {360, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Cape Verde
	static _tzinfo_t capeverde()
	{
		static TIMEZONE _tz = {60, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Caucasus
	static _tzinfo_t caucasus()
	{
		static TIMEZONE _tz = {-240, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Cen. Australia
	static _tzinfo_t cenaustralia()
	{
		static TIMEZONE _tz = {-570, 0, -60, {0, 3, 0, 5, 3, 0, 0, 0}, {0, 10, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Central America
	static _tzinfo_t centralamerica()
	{
		static TIMEZONE _tz = {360, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Central Asia
	static _tzinfo_t centralasia()
	{
		static TIMEZONE _tz = {-360, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Central Europe
	static _tzinfo_t centraleurope()
	{
		static TIMEZONE _tz = {-60, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Central European
	static _tzinfo_t centraleuropean()
	{
		static TIMEZONE _tz = {-60, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Central Pacific
	static _tzinfo_t centralpacific()
	{
		static TIMEZONE _tz = {-660, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Central
	static _tzinfo_t central()
	{
		static TIMEZONE _tz = {360, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//China
	static _tzinfo_t china()
	{
		static TIMEZONE _tz = {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Dateline
	static _tzinfo_t dateline()
	{
		static TIMEZONE _tz = {720, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//E. Africa
	static _tzinfo_t eafrica()
	{
		static TIMEZONE _tz = {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//E. Australia
	static _tzinfo_t eaustralia()
	{
		static TIMEZONE _tz = {-600, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//E. Europe
	static _tzinfo_t eeurope()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 9, 0, 5, 1, 0, 0, 0}, {0, 3, 0, 5, 0, 0, 0, 0} };
		return _tz;
	}
	//E. South America
	static _tzinfo_t esouthamerica()
	{
		static TIMEZONE _tz = {180, 0, -60, {0, 2, 0, 2, 2, 0, 0, 0}, {0, 10, 0, 3, 2, 0, 0, 0} };
		return _tz;
	}
	//Eastern
	static _tzinfo_t eastern()
	{
		static TIMEZONE _tz = {300, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 3, 0, 2, 2, 0, 0, 0} };
		return _tz;
	}
	//Egypt
	static _tzinfo_t egypt()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 9, 3, 5, 2, 0, 0, 0}, {0, 5, 5, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Ekaterinburg
	static _tzinfo_t ekaterinburg()
	{
		static TIMEZONE _tz = {-300, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Fiji
	static _tzinfo_t fiji()
	{
		static TIMEZONE _tz = {-720, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//FLE
	static _tzinfo_t fle()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 10, 0, 5, 4, 0, 0, 0}, {0, 3, 0, 5, 3, 0, 0, 0} };
		return _tz;
	}
	//GMT
	static _tzinfo_t gmt()
	{
		static TIMEZONE _tz = {0, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 3, 0, 5, 1, 0, 0, 0} };
		return _tz;
	}
	//Greenland
	static _tzinfo_t greenland()
	{
		static TIMEZONE _tz = {180, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Greenwich
	static _tzinfo_t greenwich()
	{
		static TIMEZONE _tz = {0, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//GTB
	static _tzinfo_t gtb()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Hawaiian
	static _tzinfo_t hawaiian()
	{
		static TIMEZONE _tz = {600, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//India
	static _tzinfo_t india()
	{
		static TIMEZONE _tz = {-330, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Iran
	static _tzinfo_t iran()
	{
		static TIMEZONE _tz = {-210, 0, -60, {0, 9, 2, 4, 2, 0, 0, 0}, {0, 3, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Israel
	static _tzinfo_t israel()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Korea
	static _tzinfo_t korea()
	{
		static TIMEZONE _tz = {-540, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Mexico
	static _tzinfo_t mexico()
	{
		static TIMEZONE _tz = {360, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Mid-Atlantic
	static _tzinfo_t midatlantic()
	{
		static TIMEZONE _tz = {120, 0, -60, {0, 9, 0, 5, 2, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Mountain
	static _tzinfo_t mountain()
	{
		static TIMEZONE _tz = {420, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Myanmar
	static _tzinfo_t myanmar()
	{
		static TIMEZONE _tz = {-390, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//N. Central Asia
	static _tzinfo_t ncentralasia()
	{
		static TIMEZONE _tz = {-360, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Nepal
	static _tzinfo_t nepal()
	{
		static TIMEZONE _tz = {-345, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//New Zealand
	static _tzinfo_t newzealand()
	{
		static TIMEZONE _tz = {-720, 0, -60, {0, 3, 0, 3, 2, 0, 0, 0}, {0, 10, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Newfoundland
	static _tzinfo_t newfoundland()
	{
		static TIMEZONE _tz = {210, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//North Asia East
	static _tzinfo_t northasiaeast()
	{
		static TIMEZONE _tz = {-480, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//North Asia
	static _tzinfo_t northasia()
	{
		static TIMEZONE _tz = {-420, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Pacific SA
	static _tzinfo_t pacificsa()
	{
		static TIMEZONE _tz = {240, 0, -60, {0, 3, 6, 2, 0, 0, 0, 0}, {0, 10, 6, 2, 0, 0, 0, 0} };
		return _tz;
	}
	//Pacific
	static _tzinfo_t pacific()
	{
		static TIMEZONE _tz = {480, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Romance
	static _tzinfo_t romance()
	{
		static TIMEZONE _tz = {-60, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//Russian
	static _tzinfo_t russian()
	{
		static TIMEZONE _tz = {-180, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//SA Eastern
	static _tzinfo_t saeastern()
	{
		static TIMEZONE _tz = {180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//SA Pacific
	static _tzinfo_t sapacific()
	{
		static TIMEZONE _tz = {300, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//SA Western
	static _tzinfo_t sawestern()
	{
		static TIMEZONE _tz = {240, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Samoa
	static _tzinfo_t samoa()
	{
		static TIMEZONE _tz = {660, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//SE Asia
	static _tzinfo_t seasia()
	{
		static TIMEZONE _tz = {-420, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Singapore
	static _tzinfo_t singapore()
	{
		static TIMEZONE _tz = {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//South Africa
	static _tzinfo_t southafrica()
	{
		static TIMEZONE _tz = {-120, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Sri Lanka
	static _tzinfo_t srilanka()
	{
		static TIMEZONE _tz = {-360, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Taipei
	static _tzinfo_t taipei()
	{
		static TIMEZONE _tz = {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Tasmania
	static _tzinfo_t tasmania()
	{
		static TIMEZONE _tz = {-600, 0, -60, {0, 3, 0, 5, 3, 0, 0, 0}, {0, 10, 0, 1, 2, 0, 0, 0} };
		return _tz;
	}
	//Tokyo
	static _tzinfo_t tokyo()
	{
		static TIMEZONE _tz = {-540, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Tonga
	static _tzinfo_t tonga()
	{
		static TIMEZONE _tz = {-780, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//US Eastern
	static _tzinfo_t useastern()
	{
		static TIMEZONE _tz = {300, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//US Mountain
	static _tzinfo_t usmountain()
	{
		static TIMEZONE _tz = {420, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Vladivostok
	static _tzinfo_t vladivostok()
	{
		static TIMEZONE _tz = {-600, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//W. Australia
	static _tzinfo_t waustralia()
	{
		static TIMEZONE _tz = {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//W. Central Africa
	static _tzinfo_t wcentralafrica()
	{
		static TIMEZONE _tz = {-60, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//W. Europe
	static _tzinfo_t weurope()
	{
		static TIMEZONE _tz = {-60, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}
	//West Asia
	static _tzinfo_t westasia()
	{
		static TIMEZONE _tz = {-300, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//West Pacific
	static _tzinfo_t westpacific()
	{
		static TIMEZONE _tz = {-600, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
		return _tz;
	}
	//Yakutsk
	static _tzinfo_t yakutsk()
	{
		static TIMEZONE _tz = {-540, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0} };
		return _tz;
	}

public:
	//convert datetime from local to GMT datetime
	_datetime_t to_gmt(const _datetime_t &dto) const
	{
		return dto + bias(dto);
	}
	
	//convert datetime from GMT to local
	_datetime_t to_local(const _datetime_t &dto) const
	{
		return dto - bias(dto);
	}

	_time_t bias(const _datetime_t &dto) const
	{
		long bias = Bias;
		if (have_dst())
			bias += is_dst(dto) ? DaylightBias: StandardBias;
		return static_cast<long>(bias * INTERVAL_MIN);
	}

	//return true if dst is affected, otherwise - false 
	bool is_dst(const _datetime_t &dto) const
	{
		if (!have_dst())
			return false;

		_datetime_t d_date, s_date;
		if (DaylightDate.wYear)
		{
			d_date = DaylightDate;
		}
		else
		{
			// 2002-09-11 AKA Use the year involved in the date being checked for DST rather than this year.
			d_date.set_day_of_week(DaylightDate.wDayOfWeek, DaylightDate.wDay, DaylightDate.wMonth, dto.year());
			d_date.set_time(DaylightDate.wHour, 0, 0);
		}
		
		if (StandardDate.wYear)
		{
			s_date = StandardDate;
		}
		else
		{
			// 2002-09-11 AKA Use the year involved in the date being checked for DST rather than this year.
			s_date.set_day_of_week(StandardDate.wDayOfWeek, StandardDate.wDay, StandardDate.wMonth, dto.year());
			s_date.set_time(StandardDate.wHour, 0, 0);
		}

		// AKA 2002-09-11 Added handling of Southern Hemisphere timezones.
		// Wout Louwers 2003-1-10 Correction when (dto == d_date) (with compare to TzSpecificLocalTimeToSystemTime)
		if (StandardDate.wMonth > DaylightDate.wMonth)
		{	// It is the northern hemisphere.
			return ( dto < s_date && (dto > d_date || dto == d_date));
		}
		else
		{	// It is the southern hemisphere.
			return ( dto < s_date || (dto > d_date || dto == d_date));
		}
	}

	bool have_dst() const 
	{
		return DaylightDate.wMonth != 0;
	}
	
	_tzinfo_t &operator =(const TIMEZONE &tz)
	{
		Bias = tz.Bias;
		StandardBias = tz.StandardBias;
		DaylightBias = tz.DaylightBias;
		StandardDate = tz.StandardDate;
		DaylightDate = tz.DaylightDate;
		return *this;
	}
	_tzinfo_t &operator =(const TIME_ZONE_INFORMATION &tz)
	{
		Bias = tz.Bias;
		StandardBias = tz.StandardBias;
		DaylightBias = tz.DaylightBias;
		StandardDate = tz.StandardDate;
		DaylightDate = tz.DaylightDate;
		return *this;
	}

	LPCTSTR fmt(LPCTSTR sfmt = _TEXT("bias = %d, std. bias = %d, dst. bias = %d")) const
	{
		static TCHAR buff[1024];	   
		_stprintf_s(buff, _countof(buff), sfmt, Bias, StandardBias, DaylightBias);

		return buff;
	}

	//save tz into registry key 
	bool save_reg(LPCTSTR reg_path, LPCTSTR reg_key=_T("TZI"))
	{
		HKEY hkey; bool succ = false;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, reg_path, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS)
		{
			if (RegSetValueEx(hkey, reg_key, 0, REG_BINARY, (UCHAR*)this, sizeof(this)) == ERROR_SUCCESS)
				succ = true;
			RegCloseKey(hkey);
		}
		return succ;
	}
	//load tz from registry key
	bool load_reg(LPCTSTR reg_path, LPCTSTR reg_key=_T("TZI"))
	{
		HKEY hkey; bool succ = false;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, reg_path, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
		{
			DWORD dwType, dwSize = sizeof(this);
			if (RegQueryValueEx(hkey, reg_key, 0, &dwType, (UCHAR*)this, &dwSize) == ERROR_SUCCESS)
				succ = true;
			RegCloseKey(hkey);
		}
		return succ;
	}
};

class _tzlist_t: public std::vector<TIMEZONE_INFO>
{
public:
	_tzlist_t(bool init = true) { if (init) load(); }
public:
	bool load()
	{
		clear();
		static TCHAR regkey[_MAX_PATH];
		OSVERSIONINFO os = { sizeof(OSVERSIONINFO) }; 
		GetVersionEx(&os);
		HKEY hkey;
		if (VER_PLATFORM_WIN32_NT == os.dwPlatformId && RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"), 0, KEY_ENUMERATE_SUB_KEYS, &hkey) == ERROR_SUCCESS ||
			VER_PLATFORM_WIN32_WINDOWS == os.dwPlatformId && RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Time Zones"), 0, KEY_ENUMERATE_SUB_KEYS, &hkey) == ERROR_SUCCESS)
		{
			FILETIME fileTime;
			TIMEZONE_INFO zone;
			DWORD index=0;
			DWORD dwSize=sizeof(zone.st_name)*sizeof(TCHAR);
			memset(zone.lc_name, 0, sizeof(zone.lc_name)*sizeof(TCHAR));
			memset(zone.st_name, 0, sizeof(zone.st_name)*sizeof(TCHAR));
			while(RegEnumKeyEx(hkey,index,zone.st_name,&dwSize,NULL,NULL,NULL,&fileTime)==ERROR_SUCCESS)
			{
				index++;
				HKEY hsubkey;
				if (RegOpenKeyEx(hkey, zone.st_name, 0, KEY_QUERY_VALUE, &hsubkey) == ERROR_SUCCESS)
				{
					dwSize=sizeof(zone.lc_name)*sizeof(TCHAR);
					RegQueryValueEx(hsubkey,_T("Display"),NULL,NULL,(UCHAR*)(zone.lc_name),&dwSize);
					zone.lc_name[dwSize-1] = _T('\0');

					dwSize = sizeof(TIMEZONE);
					RegQueryValueEx(hsubkey,_T("TZI"),NULL,NULL,(UCHAR*)&(zone.tzi),&dwSize);

					RegCloseKey(hsubkey); //Added by Joel 2/25/2003
				}
				if (VER_PLATFORM_WIN32_NT == os.dwPlatformId)
				{
					LPTSTR szST = _tcsstr(zone.st_name, _T(" Standard Time"));
					if (szST)
						*szST = _T('\0');
				}
				push_back(zone);
				dwSize=_MAX_PATH;
			}
			RegCloseKey(hkey); //Added by Joel 2/25/2003
			return true;
		}
		return false;
	}
	//get timezone info using standard timezone name
	//standard timezone name is different in Win9x and WinNT
	//ex. Win9x - "China", WinNT - "China Standard Time" (WinNT = Win9x + "Standard Time")
	//this func want to have win9x name format (ex. "China")
	//!!!note: be careful for timezone name in localized Windows version - it may use not english name
	bool get(LPCTSTR st_name, TIMEZONE &tzi) const
	{
		const_iterator it=find(st_name);
		return it!=end() ? tzi=it->tzi, true : false;
	}
	_tzinfo_t get(LPCTSTR st_name) const
	{
		TIMEZONE tzi; get(st_name, tzi);
		return tzi;
	}
	iterator find(const TIMEZONE &tzi)
	{
		iterator it;
		for (it=begin();it!=end();it++)
		{
			if (!memcmp(&it->tzi, &tzi, sizeof(tzi)))
				break;
		}
		return it;
	}
	const_iterator find(const TIMEZONE &tzi) const
	{
		const_iterator it;
		for (it=begin();it!=end();it++)
		{
			if (!memcmp(&it->tzi, &tzi, sizeof(tzi)))
				break;
		}
		return it;
	}
	iterator find(LPCTSTR st_name)
	{
		iterator it;
		for (it=begin();it!=end();it++)
		{
			if (!_tcscmp(st_name, it->st_name))
				break;
		}
		return it;
	}
	const_iterator find(LPCTSTR st_name) const
	{
		const_iterator it;
		for (it=begin();it!=end();it++)
		{
			if (!_tcscmp(st_name, it->st_name))
				break;
		}
		return it;
	}
};
