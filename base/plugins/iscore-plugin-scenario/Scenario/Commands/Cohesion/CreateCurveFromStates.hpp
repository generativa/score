#pragma once
#include <Scenario/Commands/ScenarioCommandFactory.hpp>
#include <Scenario/Commands/Constraint/AddOnlyProcessToConstraint.hpp>
#include <Scenario/Commands/Constraint/Rack/Slot/AddLayerModelToSlot.hpp>
#include <iscore/command/SerializableCommand.hpp>
#include <iscore/tools/ModelPath.hpp>
#include <State/Address.hpp>

class ConstraintModel;

class CreateCurveFromStates : public iscore::SerializableCommand
{
        ISCORE_SERIALIZABLE_COMMAND_DECL(
                ScenarioCommandFactoryName(),
                CreateCurveFromStates,
                "CreateCurveFromStates")
    public:
        CreateCurveFromStates(
                Path<ConstraintModel>&& constraint,
                const std::vector<std::pair<Path<SlotModel>, Id<LayerModel>>>& slotList,
                const Id<Process>& curveId,
                const iscore::Address &address,
                double start,
                double end,
                double min, double max);

        void undo() const override;
        void redo() const override;

    protected:
        void serializeImpl(QDataStream&) const override;
        void deserializeImpl(QDataStream&) override;

    private:
        AddOnlyProcessToConstraint m_addProcessCmd;
        std::vector<Scenario::Command::AddLayerModelToSlot> m_slotsCmd;

        iscore::Address m_address;

        double m_start{}, m_end{};
        double m_min{}, m_max{};

};