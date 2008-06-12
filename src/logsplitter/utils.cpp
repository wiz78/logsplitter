/***************************************************************************
	revision             : $Id: utils.cpp,v 1.2 2008-06-12 17:13:48 tellini Exp $
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

#include "utils.h"

using namespace std;

//---------------------------------------------------------------------------
std::string Utils::Trim( std::string const& source, char const *delims )
{
	string 				result( source );
	string::size_type	index = result.find_last_not_of( delims );
	
	if( index != string::npos )
		result.erase( ++index );
	
	index = result.find_first_not_of( delims );
	
	if( index != string::npos )
		result.erase( 0, index );
	else
		result.erase();
	
	return( result );
}
//---------------------------------------------------------------------------
