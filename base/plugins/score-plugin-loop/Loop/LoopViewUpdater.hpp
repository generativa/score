#pragma once
#include <Scenario/Document/VerticalExtent.hpp>

namespace Scenario
{
class EventPresenter;
class SynchronizationPresenter;
class TemporalIntervalPresenter;
class StatePresenter;
}
namespace Loop
{
class LayerPresenter;
class ViewUpdater
{
public:
  ViewUpdater(LayerPresenter& presenter);

  void updateEvent(const Scenario::EventPresenter& event);

  void updateInterval(const Scenario::TemporalIntervalPresenter& pres);

  void updateSynchronization(const Scenario::SynchronizationPresenter& timesync);

  void updateState(const Scenario::StatePresenter& state);

  LayerPresenter& m_presenter;

  static const Scenario::VerticalExtent extent()
  {
    return {30., 50.};
  }
};
}
