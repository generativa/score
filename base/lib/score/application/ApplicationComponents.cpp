// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <score/plugins/documentdelegate/DocumentDelegateFactory.hpp>
#include <score/tools/exceptions/MissingCommand.hpp>

#include "ApplicationComponents.hpp"
#include <score/command/CommandGeneratorMap.hpp>
#include <score/plugins/application/GUIApplicationPlugin.hpp>
#include <score/plugins/panel/PanelDelegate.hpp>
#include <score/plugins/qt_interfaces/PluginRequirements_QtInterface.hpp>

namespace score
{
ApplicationComponentsData::ApplicationComponentsData() = default;
ApplicationComponentsData::~ApplicationComponentsData()
{
  for(auto& sub_map : commands) 
    for(auto& pr : sub_map.second)
      delete pr.second;
    /*
     for(auto& elt : settings)
     {
         delete elt;
     }*/

  for (auto& elt : guiAppPlugins)
  {
    // TODO some have the presenter in parent,
    // check that it won't cause crashes.
    delete elt;
  }

  // FIXME do not delete static plug-ins ?
  for (auto& elt : addons)
  {
    if (elt.plugin)
    {
      dynamic_cast<QObject*>(elt.plugin)->deleteLater();
    }
  }
}

Command*
ApplicationComponents::instantiateUndoCommand(const CommandData& cmd) const
{
  auto it = m_data.commands.find(cmd.parentKey);
  if (it != m_data.commands.end())
  {
    auto it2 = it->second.find(cmd.commandKey);
    if (it2 != it->second.end())
    {
      return (*it2->second)(cmd.data);
    }
  }

#if defined(SCORE_DEBUG)
  qDebug() << "ALERT: Command" << cmd.parentKey << "::" << cmd.commandKey
           << "could not be instantiated.";
  SCORE_ABORT;
#else
  throw MissingCommandException(cmd.parentKey, cmd.commandKey);
#endif
  return nullptr;
}
}
