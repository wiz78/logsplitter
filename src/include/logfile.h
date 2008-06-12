/***************************************************************************
	revision             : $Id: logfile.h,v 1.1.1.1 2008-06-12 17:06:47 tellini Exp $
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

#ifndef LOGFILE_H
#define LOGFILE_H

#include <string>
#include <fstream>

using namespace std;

class LogFile
{
public:
						LogFile( const string& host );
						~LogFile();

	void				SetFileName( const string& fileName )	{ FileName = fileName; }
	void				SetPostRotate( const string& cmd )		{ PostRotate = cmd; }
	void				SetPeriod( const string& period );
	void				SetLastRotateTime( time_t tm )			{ LastRotateTime = tm; }
	
	const string&		GetHost( void ) const 					{ return( Host ); }
	const string&		GetHostLowerCase( void ) const			{ return( HostLowerCase ); }
	time_t				GetLastRotateTime( void ) const			{ return( LastRotateTime ); }

	void				Write( const string& line );

	void				CheckRotation( time_t now );

private:
	string				Host;
	string				HostLowerCase;
	string				FileName;
	string				PostRotate;
	int					Period;
	time_t				LastRotateTime;
	time_t				LastAccess;
	ofstream			Stream;

	void				CloseLog( void );
	void 				PerformPostRotate( const string& oldFile );
};

#endif /* LOGFILE_H */
