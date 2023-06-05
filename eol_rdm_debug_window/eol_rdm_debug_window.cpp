#include "eol_rdm_debug_window.h"
#include "ui_eol_rdm_debug_window.h"

eol_rdm_debug_window::eol_rdm_debug_window(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_rdm_debug_window)
{
  ui->setupUi(this);
}

eol_rdm_debug_window::~eol_rdm_debug_window()
{
  delete ui;
}

/*
dBsm从-100到-80划分为红色，从-80到-60划分为黄色，从-60到-40划分为绿色，从-40到-20划分为蓝色，从-20到0划分为紫色
*/
