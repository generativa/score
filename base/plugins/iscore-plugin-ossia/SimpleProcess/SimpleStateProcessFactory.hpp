#pragma once
#include <Process/StateProcessFactory.hpp>
#include <SimpleProcess/SimpleStateProcess.hpp>
#include <iscore/serialization/VisitorCommon.hpp>

class SimpleStateProcessFactory : public StateProcessFactory
{


        // StateProcessFactory interface
    public:
        QString prettyName() const override
        { return QObject::tr("toto"); }

        const StateProcessFactoryKey& key_impl() const override
        {
            static const StateProcessFactoryKey name{"toto"};
            return name;
        }

        StateProcess*make(const Id<StateProcess>& id, QObject* parent) override
        {
            return new SimpleStateProcessModel(id, parent);
        }
        StateProcess*load(const VisitorVariant& vis, QObject* parent) override
        {
            return deserialize_dyn(vis, [&] (auto&& deserializer)
            { return new SimpleStateProcessModel{deserializer, parent};});
        }
};