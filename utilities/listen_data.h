#ifndef LISTEN_DATA_H
#define LISTEN_DATA_H

#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QRunnable>
#include "circularqueue.h"
#include "can_driver_model.h"

class listen_data : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit listen_data(can_driver_model *can_driver_ptr, QObject *parent = nullptr);

signals:

public:
//  virtual void run() override
//  {
//      run_state = true;
//      run_task();
//  }

  void stop()
  {
    run_state = false;
  }

  void run_task();
private:
  bool run_state = false;
  can_driver_model *can_driver_obj = nullptr;
};

#endif // LISTEN_DATA_H
