/*!
 *  @file qttest.cpp
 *  @brief Simple QT- dialog box for testing building QT/KDE applications
 *
 *  @date 25.08.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

//#include <QApplication>
#include <qapplication.h>
#include <cstdlib>
#include "qttest.hpp"
#include "simpleForm.hpp"

int main( int argc, char** ppArgv )
{
   QApplication app( argc, ppArgv );
   
   SimpleForm dialog;
   dialog.show();
   
   return app.exec();
}

//================================== EOF ======================================
