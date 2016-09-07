#pragma once
#include <ossia/network/local/local.hpp>
#include <ossia/network/generic/generic_device.hpp>
#include <iscore/plugins/documentdelegate/plugin/DocumentDelegatePluginModel.hpp>
#include <iscore/tools/Metadata.hpp>

#include <Engine/Protocols/Local/LocalDevice.hpp>

#include "SetProperty.hpp"
#include "GetProperty.hpp"
#include "Property.hpp"

namespace iscore { class ModelMetadata; }

namespace Scenario
{
class ProcessModel;
class ConstraintModel;
class EventModel;
class TimeNodeModel;
class StateModel;
}
namespace Engine
{
namespace LocalTree
{
class Constraint;
class ISCORE_PLUGIN_ENGINE_EXPORT DocumentPlugin :
        public iscore::DocumentPlugin
{
    public:
        DocumentPlugin(
                iscore::Document& doc,
                QObject* parent);

        ~DocumentPlugin();

        void init();

        ossia::net::generic_device& device() { return m_localDevice; }
        const ossia::net::generic_device& device() const { return m_localDevice; }

        Network::LocalDevice& localDevice()
        { return m_localDeviceWrapper; }

    private:
        void create();
        void cleanup();

        Constraint* m_root{};
        ossia::net::generic_device m_localDevice;
        Network::LocalDevice m_localDeviceWrapper;

};

}
}
