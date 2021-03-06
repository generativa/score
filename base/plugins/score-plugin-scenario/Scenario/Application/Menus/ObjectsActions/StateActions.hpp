#pragma once

#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <score/actions/Menu.hpp>

#include <Process/Layer/LayerContextMenu.hpp>
#include <QAction>
#include <score_plugin_scenario_export.h>
namespace score
{
struct GUIElements;
}
namespace Scenario
{
class ScenarioApplicationPlugin;
class TemporalScenarioPresenter;
class SCORE_PLUGIN_SCENARIO_EXPORT StateActions : public QObject
{
public:
  StateActions(ScenarioApplicationPlugin* parent);

  void makeGUIElements(score::GUIElements& ref);
  void setupContextMenu(Process::LayerContextMenuManager& ctxm);

private:
  CommandDispatcher<> dispatcher();

  ScenarioApplicationPlugin* m_parent{};
  QAction* m_refreshStates{};
  QAction* m_snapshot{};
};
}
