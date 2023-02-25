#include "listen_data.h"

listen_data::listen_data(can_driver *can_driver_ptr, QObject *parent)
  : QObject{parent}
{
  can_driver_obj = can_driver_ptr;
}

void listen_data::run_task()
{
//  if(nullptr == can_driver_obj)
//  {
//    return;
//  }
//  while(run_state)
//  {
//    can_driver_obj->receice_data();
//    QThread::sleep(1);
//  }
}
