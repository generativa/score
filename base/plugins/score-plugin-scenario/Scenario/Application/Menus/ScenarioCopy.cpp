// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ScenarioCopy.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/Synchronization/SynchronizationModel.hpp>
#include <Scenario/Process/Algorithms/ContainersAccessors.hpp>
#include <Scenario/Process/Algorithms/ProcessPolicy.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <algorithm>
#include <score/tools/std/Optional.hpp>
#include <vector>

#include <ossia/detail/algorithms.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/serialization/VisitorCommon.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/Identifier.hpp>
namespace Scenario
{
template <typename Selected_T>
static auto arrayToJson(Selected_T&& selected)
{
  QJsonArray array;
  if (!selected.empty())
  {
    for (const auto& element : selected)
    {
      array.push_back(score::marshall<JSONObject>(*element));
    }
  }

  return array;
}

template <typename Scenario_T>
QJsonObject copySelected(const Scenario_T& sm, QObject* parent)
{
  auto selectedIntervals = selectedElements(getIntervals(sm));
  auto selectedEvents = selectedElements(getEvents(sm));
  auto selectedSynchronizations = selectedElements(getSynchronizations(sm));
  auto selectedStates = selectedElements(getStates(sm));

  for (const IntervalModel* interval : selectedIntervals)
  {
    auto start_it
        = ossia::find_if(selectedStates, [&](const StateModel* state) {
            return state->id() == interval->startState();
          });
    if (start_it == selectedStates.end())
    {
      selectedStates.push_back(&sm.state(interval->startState()));
    }

    auto end_it = ossia::find_if(selectedStates, [&](const StateModel* state) {
      return state->id() == interval->endState();
    });
    if (end_it == selectedStates.end())
    {
      selectedStates.push_back(&sm.state(interval->endState()));
    }
  }

  for (const StateModel* state : selectedStates)
  {
    auto ev_it = ossia::find_if(selectedEvents, [&](const EventModel* event) {
      return state->eventId() == event->id();
    });
    if (ev_it == selectedEvents.end())
    {
      selectedEvents.push_back(&sm.event(state->eventId()));
    }

    // If the previous or next interval is not here, we set it to null in a
    // copy.
  }
  for (const EventModel* event : selectedEvents)
  {
    auto tn_it
        = ossia::find_if(selectedSynchronizations, [&](const SynchronizationModel* tn) {
            return tn->id() == event->synchronization();
          });
    if (tn_it == selectedSynchronizations.end())
    {
      selectedSynchronizations.push_back(&sm.synchronization(event->synchronization()));
    }

    // If some events aren't there, we set them to null in a copy.
  }

  std::vector<SynchronizationModel*> copiedSynchronizations;
  copiedSynchronizations.reserve(selectedSynchronizations.size());
  for (const auto& tn : selectedSynchronizations)
  {
    auto clone_tn = new SynchronizationModel(*tn, tn->id(), nullptr);
    auto events = clone_tn->events();
    for (const auto& event : events)
    {
      auto absent = ossia::none_of(selectedEvents, [&](const EventModel* ev) {
        return ev->id() == event;
      });
      if (absent)
        clone_tn->removeEvent(event);
    }

    copiedSynchronizations.push_back(clone_tn);
  }

  std::vector<EventModel*> copiedEvents;
  copiedEvents.reserve(selectedEvents.size());
  for (const auto& ev : selectedEvents)
  {
    auto clone_ev = new EventModel(*ev, ev->id(), nullptr);
    auto states = clone_ev->states();
    for (const auto& state : states)
    {
      auto absent = ossia::none_of(selectedStates, [&](const StateModel* st) {
        return st->id() == state;
      });
      if (absent)
        clone_ev->removeState(state);
    }

    copiedEvents.push_back(clone_ev);
  }

  std::vector<StateModel*> copiedStates;
  copiedStates.reserve(selectedStates.size());
  auto& stack = score::IDocument::documentContext(*parent).commandStack;
  for (const StateModel* st : selectedStates)
  {
    auto clone_st = new StateModel(*st, st->id(), stack, parent);

    // NOTE : we must not serialize the state with their previous / next
    // interval
    // since they will change once pasted and cause crash at the end of the
    // ctor
    // of StateModel. They are saved in the previous / next state of interval
    // anyway.
    SetNoPreviousInterval(*clone_st);
    SetNoNextInterval(*clone_st);

    copiedStates.push_back(clone_st);
  }

  QJsonObject base;
  base["Intervals"] = arrayToJson(selectedIntervals);
  base["Events"] = arrayToJson(copiedEvents);
  base["TimeNodes"] = arrayToJson(copiedSynchronizations);
  base["States"] = arrayToJson(copiedStates);

  for (auto elt : copiedSynchronizations)
    delete elt;
  for (auto elt : copiedEvents)
    delete elt;
  for (auto elt : copiedStates)
    delete elt;

  return base;
}

QJsonObject copySelectedScenarioElements(const Scenario::ProcessModel& sm)
{
  auto obj = copySelected(sm, const_cast<Scenario::ProcessModel*>(&sm));

  obj["Comments"] = arrayToJson(selectedElements(sm.comments));

  return obj;
}

QJsonObject
copySelectedScenarioElements(const BaseScenarioContainer& sm, QObject* parent)
{
  return copySelected(sm, parent);
}
}
