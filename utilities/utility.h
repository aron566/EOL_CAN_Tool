/**
 *  @file utility.hpp
 *
 *  @date 2023年02月21日 17:59:10 星期二
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version v1.0.0 aron566 2023.02.21 17:59 初始版本.
 */
#ifndef UTILITY_H
#define UTILITY_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
// #include <limits.h> /**< need variable max value    */
// #include <stdalign.h> /**< need alignof    */
// #include <stdarg.h> /**< need va_start    */
/** Private includes ---------------------------------------------------------*/
#include <QObject>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class utility : public QObject
{
  Q_OBJECT
public:
  explicit utility(QObject *parent = nullptr);

signals:

public:

};

#endif // UTILITY_H
/******************************** End of file *********************************/
