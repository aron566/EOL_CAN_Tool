#include "debug_window.h"
#include "ui_debug_window.h"

debug_window::debug_window(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::debug_window)
{
  ui->setupUi(this);
}

debug_window::~debug_window()
{
  delete ui;
}
