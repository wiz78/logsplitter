/***************************************************************************
	revision             : $Id: logfile.cpp,v 1.2 2008-06-12 17:13:48 tellini Exp $
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

#include "main.h"
#include "utils.h"
#include "logfile.h"

#include <algorithm>
#include <cctype>
#include <stdio.h>

//---------------------------------------------------------------------------
LogFile::LogFile( const string& host )
{
	Host          = host;
	HostLowerCase = host;
	
	transform( HostLowerCase.begin(), HostLowerCase.end(),
			   HostLowerCase.begin(), static_cast<int(*)(int)>( tolower ));
		
	time( &LastRotateTime );
}
//---------------------------------------------------------------------------
LogFile::~LogFile()
{
	CloseLog();
}
//---------------------------------------------------------------------------
void LogFile::CloseLog( void )
{
	if( Stream.is_open() ) {
		Stream.flush();
		Stream.close();
	}
}
//---------------------------------------------------------------------------
void LogFile::SetPeriod( const string& period )
{
	string::size_type	pos = period.find( ' ' );
	
	if( pos != string::npos ) {
		string unit = Utils::Trim( period.substr( pos + 1 ));

		Period = atoi( period.substr( 0, pos ).c_str() );
		
		if( unit == "days" )
			Period *= 86400;
		else if( unit == "hours" )
			Period *= 3600;

	} else
		Period = atoi( period.c_str() ) * 3600;
}
//---------------------------------------------------------------------------
void LogFile::Write( const string& line )
{
	if( !Stream.is_open() )
		Stream.open( FileName.c_str(), ios::app );
	
	Stream << line << endl;
	
	time( &LastAccess );
}
//---------------------------------------------------------------------------
void LogFile::CheckRotation( time_t now )
{
	if( now - LastRotateTime >= Period ) {
		bool exists = Stream.is_open();

		if( exists )
			CloseLog();
		else {
			ifstream	fh( FileName.c_str() );
			
			if( fh )
				exists = true;
				
			fh.close();
		}

		if( exists ) {
			string oldFile = FileName + ".old";

			unlink( oldFile.c_str() );
			rename( FileName.c_str(), oldFile.c_str() );

			if( !PostRotate.empty() )
				PerformPostRotate( oldFile );
		}
		
		time( &LastRotateTime );	
	}
}
//---------------------------------------------------------------------------
void LogFile::PerformPostRotate( const string& oldFile )
{
	if( fork() == 0 ) {
		string				cmd = PostRotate;
		string::size_type	pos = 0;
		
		while(( pos = cmd.find( "%f", pos )) != string::npos )
			cmd.replace( pos, 2, oldFile );

		pos = 0;
		
		while(( pos = cmd.find( "%h", pos )) != string::npos )
			cmd.replace( pos, 2, Host );

		execl( "/bin/sh", "sh", "-c", cmd.c_str(), NULL );

		exit( 0 );
	}
}
//---------------------------------------------------------------------------
