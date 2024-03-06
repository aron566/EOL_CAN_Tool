/**
 *  @file eol_rdm_debug_window.hpp
 *
 *  @date 2024年01月18日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef EOL_RDM_DEBUG_WINDOW_H
#define EOL_RDM_DEBUG_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QWidget>
/** Private includes ---------------------------------------------------------*/

/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

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
/******************************** End of file *********************************/
