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

#include "dglbusydialog.h"

#include <QProgressBar>

DGLbusyDialog::DGLbusyDialog(QWidget* parent):QProgressDialog(
    "Starting debugging session (waiting for application to "
    "try "
    "use OpenGL)...",
    "Cancel", 0, 1, parent) {
    
    setValue(0);

    QProgressBar *bar = new QProgressBar(this);
    setBar(bar);
    bar->setMaximum(0);
    bar->setMinimum(0);
    bar->setValue(-1);

    setWindowModality(Qt::WindowModal);
    reset();
}


