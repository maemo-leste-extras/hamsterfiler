/**************************************************************************
    Hamster Filer
    Copyright (C) 2012-2013 Grzegorz Gidel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char  *argv[])
{
    QApplication::setOrganizationName("hamsterfiler");
    QApplication::setApplicationName("hamsterfiler");

    QApplication app(argc, argv);

    QLocale::setDefault(QLocale("C"));

    MainWindow window(argc > 1 ? argv[1] : QString());

    if (QString(getenv("USER")) == "root")
        window.setWindowTitle(window.windowTitle() + " (root)");

    window.show();

    return app.exec();
}
