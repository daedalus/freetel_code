/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: postfilter.h
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 13/09/09
                                                                             
  Postfilter header file.
                                                                             
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __POSTFILTER__
#define __POSTFILTER__

#include "sine.h"

void postfilter(MODEL *model, int voiced, float *bg_est);

#endif
