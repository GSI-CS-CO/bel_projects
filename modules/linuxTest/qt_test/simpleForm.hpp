/*!
 *  @file simpleForm.hpp
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
#ifndef _SIMPLEFORM_HPP
#define _SIMPLEFORM_HPP

/*
 * uic simpleForm.ui -o ./generated/ui_simpleForm.hpp
 * moc simpleForm.hpp -o ./generated/moc_simpleFrom.cpp
 */

#include "generated/ui_simpleForm.hpp"

class SimpleForm: public QDialog
{
   Q_OBJECT

   Ui::simpleDialog m_oUi;
   uint             m_counter;
public:
   SimpleForm( QDialog* = nullptr );
   ~SimpleForm( void );
   
public slots:
   void onButtonActionClicked( void );
   void onButtonResetClicked( void );
};

#endif // _SIMPLEFORM_HPP
//================================== EOG ======================================
