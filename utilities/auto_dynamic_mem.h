/**
 *  @file auto_dynamic_mem.hpp
 *
 *  @date 2024年02月28日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief 自动销毁动态内存.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-02-28 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */

#ifndef AUTO_DYNAMIC_MEM_H
#define AUTO_DYNAMIC_MEM_H
/** Includes -----------------------------------------------------------------*/

/** Private includes ---------------------------------------------------------*/
#include <QDebug>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class auto_dynamic_mem
{
public:
  explicit auto_dynamic_mem(quint32 bytes);
  ~auto_dynamic_mem()
  {
    delete[] mem;
  }
private:
  quint8 *mem = nullptr;
};

#endif // AUTO_DYNAMIC_MEM_H
/******************************** End of file *********************************/

