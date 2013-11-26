/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef DGLQTGUI_H
#define DGLQTGUI_H

// include QTGui.h with some warnings disabled

#pragma warning(push)

// assignment operator could not be generated
#pragma warning(disable: 4512)

// 'uint' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4800)

// 'QStyleOption::state' : class 'QFlags<Enum>' needs to have
// dll-interface to be used by clients of class 'QStyleOption'
#pragma warning(disable: 4251)

#include <QtGui>
#include <QLayoutItem>
#include <QListWidget>
#include <QTreeWidget>
#include <QTableWidget>
#pragma warning(pop)

#endif
