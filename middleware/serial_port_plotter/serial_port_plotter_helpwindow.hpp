/***************************************************************************
**  This file is part of Serial Port Plotter                              **
**                                                                        **
**                                                                        **
**  Serial Port Plotter is a program for plotting integer data from       **
**  serial port using Qt and QCustomPlot                                  **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Borislav                                             **
**           Contact: b.kereziev@gmail.com                                **
**           Date: 29.12.14                                               **
****************************************************************************/

#ifndef SERIAL_PORT_PLOTTER_HELPWINDOW_HPP
#define SERIAL_PORT_PLOTTER_HELPWINDOW_HPP

#include <QDialog>

namespace Ui {
    class serial_port_plotter_HelpWindow;
}

class serial_port_plotter_HelpWindow : public QDialog
{
    Q_OBJECT

public:
    explicit serial_port_plotter_HelpWindow(QWidget *parent = 0);
    ~serial_port_plotter_HelpWindow();

private:
    Ui::serial_port_plotter_HelpWindow *ui;
};

#endif // HELPWINDOW_HPP
