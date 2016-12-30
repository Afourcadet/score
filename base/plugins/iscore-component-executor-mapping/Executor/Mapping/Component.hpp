#pragma once
#include <Engine/Executor/ProcessComponent.hpp>

#include <ossia/editor/mapper/mapper.hpp>
#include <ossia/editor/value/value.hpp>
#include <Mapping/MappingModel.hpp>
#include <QPointer>
#include <State/Address.hpp>
namespace ossia
{
class curve_abstract;
}
namespace Device
{
class DeviceList;
}

namespace Mapping
{
namespace RecreateOnPlay
{
class Component final
    : public ::Engine::Execution::
          ProcessComponent_T<Mapping::ProcessModel, ossia::mapper>
{
  COMPONENT_METADATA("da360b58-9885-4106-be54-8e272ed45dbe")
public:
  Component(
      ::Engine::Execution::ConstraintComponent& parentConstraint,
      ::Mapping::ProcessModel& element,
      const ::Engine::Execution::Context& ctx,
      const Id<iscore::Component>& id,
      QObject* parent);

private:
  std::shared_ptr<ossia::curve_abstract> rebuildCurve();

  ossia::val_type m_sourceAddressType{ossia::val_type(-1)};
  ossia::val_type m_targetAddressType{ossia::val_type(-1)};

  template <typename T>
  std::shared_ptr<ossia::curve_abstract> on_curveChanged_impl();

  template <typename X_T, typename Y_T>
  std::shared_ptr<ossia::curve_abstract> on_curveChanged_impl2();

  std::shared_ptr<ossia::curve_abstract> m_ossia_curve;

  const Device::DeviceList& m_deviceList;
};

using ComponentFactory
    = ::Engine::Execution::ProcessComponentFactory_T<Component>;
}
}

ISCORE_CONCRETE_COMPONENT_FACTORY(
    Engine::Execution::ProcessComponentFactory,
    Mapping::RecreateOnPlay::ComponentFactory)
