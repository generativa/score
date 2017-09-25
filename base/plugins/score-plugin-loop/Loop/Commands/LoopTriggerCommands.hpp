#pragma once
#include <Loop/LoopProcessModel.hpp>
#include <Scenario/Commands/Synchronization/AddTrigger.hpp>
#include <Scenario/Commands/Synchronization/RemoveTrigger.hpp>

SCORE_COMMAND_DECL_T(Scenario::Command::AddTrigger<Loop::ProcessModel>)
SCORE_COMMAND_DECL_T(Scenario::Command::RemoveTrigger<Loop::ProcessModel>)
