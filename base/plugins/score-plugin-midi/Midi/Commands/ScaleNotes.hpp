#pragma once
#include <Midi/Commands/CommandFactory.hpp>
#include <Midi/MidiNote.hpp>
#include <score/model/path/Path.hpp>

namespace Midi
{
class ProcessModel;

class ScaleNotes final : public score::Command
{
  SCORE_COMMAND_DECL(Midi::CommandFactoryName(), ScaleNotes, "Scale notes")
public:
  ScaleNotes(
      const ProcessModel& model,
      const std::vector<Id<Note>>& to_move,
      double delta);

  void undo(const score::DocumentContext& ctx) const override;
  void redo(const score::DocumentContext& ctx) const override;

protected:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;

private:
  Path<ProcessModel> m_model;
  std::vector<Id<Note>> m_toScale;
  double m_delta{};
};
}
