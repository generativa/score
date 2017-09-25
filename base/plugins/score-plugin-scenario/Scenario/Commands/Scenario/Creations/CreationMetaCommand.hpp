#pragma once
#include <score/command/AggregateCommand.hpp>

#include "CreateEvent_State.hpp"
#include "CreateState.hpp"

#include "CreateInterval.hpp"
#include "CreateInterval_State.hpp"
#include "CreateInterval_State_Event.hpp"
#include "CreateInterval_State_Event_Synchronization.hpp"
#include "CreateSequence.hpp"
#include <boost/range/adaptor/reversed.hpp>
namespace Scenario
{
namespace Command
{
class CreationMetaCommand final : public score::AggregateCommand
{
  SCORE_COMMAND_DECL(
      ScenarioCommandFactoryName(),
      CreationMetaCommand,
      "Create elements in scenario")
public:
  void undo(const score::DocumentContext& ctx) const override
  {
    // We only undo the creation commands
    // since the move ones perform unnecessary serialization / etc in this case
    // and don't bring anything to the table.
    // TODO REFACTOR WITH SCENARIOROLLBACKSTRATEGY
    for (auto& cmd : boost::adaptors::reverse(m_cmds))
    {
      if (cmd->key() == CreateInterval::static_key()
          || cmd->key() == CreateState::static_key()
          || cmd->key() == CreateEvent_State::static_key()
          || cmd->key() == CreateInterval_State::static_key()
          || cmd->key() == CreateInterval_State_Event::static_key()
          || cmd->key() == CreateInterval_State_Event_Synchronization::static_key()
          || cmd->key() == CreateSequence::static_key())
      {
        cmd->undo(ctx);
      }
    }
  }
};
}
}
