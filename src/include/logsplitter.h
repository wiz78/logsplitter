/***************************************************************************
	revision             : $Id: logsplitter.h,v 1.1.1.1 2008-06-12 17:06:47 tellini Exp $
    copyright            : (C) 2005 by Simone Tellini
    email                : tellini@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOGSPLITTER_H
#define LOGSPLITTER_H

#include "settings.h"
#include "logfile.h"
#include "hash_map.h"

class LogSplitter
{
public:
						LogSplitter( int argc, const char *argv[] );
						~LogSplitter();

	void				Run( void );
	void				Quit( void )		{ Running = false; }

private:
	typedef hash_map<string, LogFile *>	logHash;
	
	string				CfgName;
	Settings			Cfg;
	bool				Running;
	char				Buffer[ 1024 * 64 ];
	bool				Overflowed;
	int					BufUsed;
	LogFile				*DefaultLog;
	vector<LogFile *>	Logs;
	logHash				LogsHash;
	time_t				LastCheck;

	void				SetCredentials( void );
	void				InitLogFiles( void );
	void				LoadState( void );
	void				SaveState( void );

	void				ReadData( void );
	void				CheckLine( void );
	void				LogLine( string line );
	
	void				CheckRotations( void );
};

#endif /* LOGSPLITTER_H */
