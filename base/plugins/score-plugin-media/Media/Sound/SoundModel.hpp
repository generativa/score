#pragma once
#include <Process/Process.hpp>
#include <score/serialization/DataStreamVisitor.hpp>
#include <score/serialization/JSONVisitor.hpp>
#include <score/serialization/VisitorCommon.hpp>
#include <Media/Sound/SoundMetadata.hpp>
#include <Media/MediaFileHandle.hpp>
#include <score_plugin_media_export.h>


namespace Media
{
namespace Sound
{
class ProcessModel;

class SCORE_PLUGIN_MEDIA_EXPORT ProcessModel final : public Process::ProcessModel
{
        SCORE_SERIALIZE_FRIENDS
        PROCESS_METADATA_IMPL(Media::Sound::ProcessModel)

        Q_OBJECT
    public:
        explicit ProcessModel(
                const TimeVal& duration,
                const Id<Process::ProcessModel>& id,
                QObject* parent);

        explicit ProcessModel(
                const ProcessModel& source,
                const Id<Process::ProcessModel>& id,
                QObject* parent);

        virtual ~ProcessModel();

        template<typename Impl>
        explicit ProcessModel(
                Impl& vis,
                QObject* parent) :
            Process::ProcessModel{vis, parent}
        {
            vis.writeTo(*this);
            init();
        }

        void setFile(const QString& file);
        void setFile(const MediaFileHandle& file);

        MediaFileHandle& file()
        { return m_file; }
        const MediaFileHandle& file() const
        { return m_file; }

    signals:
        void fileChanged();

    private:
        void init();

        MediaFileHandle m_file;
};

}
}
