#ifndef EOL_RDM_DEBUG_WINDOW_H
#define EOL_RDM_DEBUG_WINDOW_H

#include <QWidget>

namespace Ui {
class eol_rdm_debug_window;
}

class eol_rdm_debug_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_rdm_debug_window(QWidget *parent = nullptr);
  ~eol_rdm_debug_window();

private:
  Ui::eol_rdm_debug_window *ui;
};

#endif // EOL_RDM_DEBUG_WINDOW_H
