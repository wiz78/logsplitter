/***************************************************************************
	revision             : $Id: exceptions.h,v 1.2 2008-06-12 17:13:48 tellini Exp $
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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

using namespace std;

class EWrongArgs {};
class EEOF {};

class EParse : public runtime_error 
{
public:
				EParse() : runtime_error( "" ) {}
				EParse( const string& what ) : runtime_error( what ) {}
};

#endif /* EXCEPTIONS_H */
