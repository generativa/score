// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "LibraryPanelDelegate.hpp"
#include <Library/JSONLibrary/LibraryWidget.hpp>
#include <Process/ProcessList.hpp>
#include <QTabWidget>
#include <score/serialization/JSONVisitor.hpp>
#include <score/serialization/VisitorCommon.hpp>
#include <score/application/GUIApplicationContext.hpp>

namespace Library
{
PanelDelegate::PanelDelegate(const score::GUIApplicationContext& ctx)
    : score::PanelDelegate{ctx}, m_widget{new QTabWidget}
{
  auto projectModel = new JSONModel;
  auto projectLib = new LibraryWidget{projectModel, m_widget};
  m_widget->addTab(projectLib, QObject::tr("Project"));

  auto systemModel = new JSONModel;
  auto& procs = ctx.interfaces<Process::ProcessFactoryList>();
  for (Process::ProcessModelFactory& proc : procs)
  {
    LibraryElement e;
    e.category = Category::Process;
    e.name = proc.prettyName();
    e.obj["Type"] = "Process";
    e.obj["uuid"] = toJsonValue(proc.concreteKey().impl());
    systemModel->addElement(e);
  }

  auto systemLib = new LibraryWidget{systemModel, m_widget};
  m_widget->addTab(systemLib, QObject::tr("System"));

  m_widget->setObjectName("LibraryExplorer");
}

QWidget* PanelDelegate::widget()
{
  return m_widget;
}

const score::PanelStatus& PanelDelegate::defaultPanelStatus() const
{
  static const score::PanelStatus status{false, Qt::RightDockWidgetArea, 0,
                                          QObject::tr("Library"),
                                          QObject::tr("Ctrl+Shift+B")};

  return status;
}
}
