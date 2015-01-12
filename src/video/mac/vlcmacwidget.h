/*
    Copyright (C) 2010 Benoit Calvez <benoit@litchis.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VLCMACWIDGET_H
#define VLCMACWIDGET_H

#include <QMacCocoaViewContainer>

#import <qmaccocoaviewcontainer_mac.h>

class VlcMacWidget : public QMacCocoaViewContainer
{
public:
    explicit VlcMacWidget(QWidget *parent = 0);
};


#endif // VLCMACWIDGET_H
