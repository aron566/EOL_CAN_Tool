#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include <QWidget>

namespace Ui {
class debug_window;
}

class debug_window : public QWidget
{
  Q_OBJECT

public:
  explicit debug_window(QString title, QWidget *parent = nullptr);
  ~debug_window();

private:
  Ui::debug_window *ui;
};

#endif // DEBUG_WINDOW_H
