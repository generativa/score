#pragma once
#include <Process/Tools/ToolPalette.hpp>
#include <QPoint>
#include <Scenario/Document/BaseScenario/BaseElementContext.hpp>
#include <Scenario/Palette/Tools/SmartTool.hpp>
#include <score/statemachine/GraphicsSceneToolPalette.hpp>

#include <Scenario/Palette/ScenarioPoint.hpp>
#include <Scenario/Palette/Tool.hpp>
#include <score/document/DocumentContext.hpp>

class BaseGraphicsObject;

namespace Scenario
{
class EditionSettings;
class MoveIntervalInScenario_StateWrapper;
class MoveLeftBraceInScenario_StateWrapper;
class MoveRightBraceInScenario_StateWrapper;
class MoveEventInTopScenario_StateWrapper;
class MoveSynchronizationInTopScenario_StateWrapper;
class ProcessModel;
class DisplayedElementsModel;
class DisplayedElementsPresenter;
class ScenarioDocumentPresenter;

class ScenarioDisplayedElementsToolPalette final
    : public GraphicsSceneToolPalette
{
public:
  ScenarioDisplayedElementsToolPalette(
      const DisplayedElementsModel&,
      ScenarioDocumentPresenter&,
      BaseGraphicsObject&);

  BaseGraphicsObject& view() const;
  const DisplayedElementsPresenter& presenter() const;
  const Scenario::ProcessModel& model() const;
  const BaseElementContext& context() const;
  const Scenario::EditionSettings& editionSettings() const;

  void activate(Scenario::Tool);
  void desactivate(Scenario::Tool);

  void on_pressed(QPointF);
  void on_moved(QPointF);
  void on_released(QPointF);
  void on_cancel();

private:
  Scenario::Point ScenePointToScenarioPoint(QPointF point);
  const DisplayedElementsModel& m_model;
  const Scenario::ProcessModel& m_scenarioModel;
  ScenarioDocumentPresenter& m_presenter;
  BaseElementContext m_context;
  BaseGraphicsObject& m_view;
  const Scenario::EditionSettings& m_editionSettings;

  Scenario::SmartTool<
            Scenario::ProcessModel,
            ScenarioDisplayedElementsToolPalette,
            BaseGraphicsObject,
            Scenario::MoveIntervalInScenario_StateWrapper,
            Scenario::MoveLeftBraceInScenario_StateWrapper,
            Scenario::MoveRightBraceInScenario_StateWrapper,
            Scenario::MoveEventInTopScenario_StateWrapper,
            Scenario::MoveSynchronizationInTopScenario_StateWrapper>
          m_state;

  ToolPaletteInputDispatcher<Scenario::Tool, ScenarioDisplayedElementsToolPalette, BaseElementContext, ScenarioDocumentPresenter>
      m_inputDisp;
};
}
