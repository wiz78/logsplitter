/***************************************************************************
	revision             : $Id: logsplitter.cpp,v 1.2 2008-06-12 17:13:48 tellini Exp $
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
#include "logsplitter.h"
#include "exceptions.h"

#include <algorithm>
#include <cctype>
#include <sys/poll.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

//---------------------------------------------------------------------------
LogSplitter::LogSplitter( int argc, const char *argv[] )
{
	if( argc < 2 )
		throw EWrongArgs();
		
	CfgName    = argv[ 1 ];
	Running    = true;
	BufUsed    = 0;
	DefaultLog = NULL;
	Overflowed = false;
	
	time( &LastCheck );

	Cfg.Load( CfgName );
	SetCredentials();
	InitLogFiles();
	LoadState();
}
//---------------------------------------------------------------------------
LogSplitter::~LogSplitter()
{
	delete DefaultLog;
	
	for( int i = 0; i < Logs.size(); i++ )
		delete Logs[ i ];
}
//---------------------------------------------------------------------------
void LogSplitter::SetCredentials( void )
{
	string str = Cfg.GetString( "General", "Group", "" );
	
	if( !str.empty() ) {
		struct group *grp = getgrnam( str.c_str() );
		
		if( grp )
			setgid( grp->gr_gid );
	}
	
	str = Cfg.GetString( "General", "User", "" );

	if( !str.empty() ) {
        struct passwd  *pwd = getpwnam( str.c_str() );

        if( pwd )
            setuid( pwd->pw_uid );
	}
}
//---------------------------------------------------------------------------
void LogSplitter::InitLogFiles( void )
{
	const vector<string>&	sections = Cfg.GetSections();
	string					defLog = Cfg.GetString( "DefaultLog", "Path", "" );
	
	for( int i = 0; i < sections.size(); i++ ) {
		string				sec = sections[ i ];
		string::size_type	pos = sec.find( ' ' );
	
		if( pos != string::npos ) {
			string word = sec.substr( 0, pos );
		
			transform( word.begin(), word.end(), word.begin(), static_cast<int(*)(int)>( tolower )); 
			
			if( word == "host" ) {
				LogFile	*log = new LogFile( sec.substr( pos + 1 ));
				
				log->SetFileName( Cfg.GetString( sec, "Path" ));
				log->SetPeriod( Cfg.GetString( sec, "Period", "1 days" ));
				log->SetPostRotate( Cfg.GetString( sec, "PostRotate" ));

				Logs.push_back( log );

				LogsHash[ log->GetHostLowerCase() ] = log;
			}
		}	
	}
	
	if( !defLog.empty() ) {

		DefaultLog = new LogFile( "" );
		
		DefaultLog->SetFileName( Cfg.GetString( "DefaultLog", "Path" ));
		DefaultLog->SetPeriod( Cfg.GetString( "DefaultLog", "Period", "1 days" ));
		DefaultLog->SetPostRotate( Cfg.GetString( "DefaultLog", "PostRotate" ));
	}
}
//---------------------------------------------------------------------------
void LogSplitter::LoadState( void )
{
	string		file = Cfg.GetString( "General", "StateFile", "/var/run/logsplitter/state.ini" );
	Settings	state;
	
	state.Load( file );

	for( int i = 0; i < Logs.size(); i++ ) {
		LogFile	*log = Logs[ i ];
	
		log->SetLastRotateTime( state.GetInt( log->GetHost(), "LastRotateTime", log->GetLastRotateTime() ));
	}

	if( DefaultLog )
		DefaultLog->SetLastRotateTime( state.GetInt( "DefaultLog", "LastRotateTime", DefaultLog->GetLastRotateTime() ));
}
//---------------------------------------------------------------------------
void LogSplitter::SaveState( void )
{
	ofstream	fh( Cfg.GetString( "General", "StateFile", "/var/run/logsplitter/state.ini" ).c_str(), ios::trunc );

	for( int i = 0; i < Logs.size(); i++ ) {
		LogFile	*log = Logs[ i ];

		fh << "[" << log->GetHost() << "]" << endl;
		fh << "LastRotateTime = " << log->GetLastRotateTime() << endl;
	}

	if( DefaultLog ) {

		fh << "[DefaultLog]" << endl;
		fh << "LastRotateTime = " << DefaultLog->GetLastRotateTime() << endl;
	}
		
	fh.flush();
}
//---------------------------------------------------------------------------
void LogSplitter::Run( void )
{
	try {
		struct pollfd	pfd = { STDIN_FILENO, POLLIN | POLLHUP, 0 };
	
		umask( 027 );
		fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0 ) | O_NONBLOCK );
	
		while( Running ) {

			poll( &pfd, 1, 1000 );
			
			if( pfd.revents & ( POLLIN | POLLHUP ))
				ReadData();
			
			CheckRotations();
			
			pfd.revents = 0;
		}
	}
	catch( const EEOF &e ) {
	}
	catch(...) {
		fprintf( stderr, "Unhandled exception caught" );
	}
	
	SaveState();
}
//---------------------------------------------------------------------------
void LogSplitter::ReadData( void )
{
	int avail = sizeof( Buffer ) - BufUsed, len;
	
	// should never happen, but if it does, swallow the first part of the line
	if( avail < 1 ) {
	
		avail      = sizeof( Buffer );
		BufUsed    = 0;
		Overflowed = true;
	}
	
	len = read( STDIN_FILENO, &Buffer[ BufUsed ], avail );
	
	if(( len == 0 ) || (( len < 0 ) && ( errno != EAGAIN )))
		throw EEOF();
		
	BufUsed += len;
	
	CheckLine();
}
//---------------------------------------------------------------------------
void LogSplitter::CheckLine( void )
{
	bool gotLine;
	
	do {
		gotLine = false;
	
		for( int i = 0; i < BufUsed; i++ )
			if( Buffer[ i ] == '\n' ) {
				
				Buffer[ i ] = '\0';
				
				if(( i > 0 ) && ( Buffer[ i - 1 ] == '\r' ))
					Buffer[ i - 1 ] = '\0';
				
				LogLine( string( Buffer ));
			
				BufUsed -= i + 1;
	
				if( BufUsed )
					memcpy( Buffer, &Buffer[ i + 1 ], BufUsed );
			
				i          = BufUsed;
				Overflowed = false;
				gotLine    = true;
			}

	} while( gotLine );
}
//---------------------------------------------------------------------------
void LogSplitter::LogLine( string line )
{
	string::size_type	pos = line.find( ' ' );
	LogFile				*log = DefaultLog;

	if( !Overflowed && ( pos != string::npos )) {
		string 					vhost = line.substr( 0, pos );
		logHash::const_iterator	l;

		transform( vhost.begin(), vhost.end(), vhost.begin(), static_cast<int(*)(int)>( tolower ));

		l = LogsHash.find( vhost );

		if( l != LogsHash.end() ) {

			log = l->second;

			line.erase( 0, pos + 1 );
		}
	}

	if( log )
		log->Write( line );
}
//---------------------------------------------------------------------------
void LogSplitter::CheckRotations( void )
{
	time_t	now;
	
	time( &now );

	if( LastCheck != now ) {

		for( int i = 0; i < Logs.size(); i++ )
			Logs[ i ]->CheckRotation( now );

		if( DefaultLog )
			DefaultLog->CheckRotation( now );

		LastCheck = now;
	}
}
//---------------------------------------------------------------------------
