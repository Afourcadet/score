#pragma once
#include <Engine/LocalTree/BaseCallbackWrapper.hpp>
#include <Engine/OSSIA2score.hpp>
#include <Engine/score2OSSIA.hpp>
#include <State/ValueConversion.hpp>
#include <State/Value.hpp>

namespace Engine
{
namespace LocalTree
{
template <typename T>
using MatchingType_T = Engine::ossia_to_score::MatchingType<
    std::remove_const_t<std::remove_reference_t<T>>>;

template <typename Property>
struct PropertyWrapper final : public BaseCallbackWrapper
{
  using model_t = typename Property::model_type;
  using param_t = typename Property::param_type;
  model_t& m_model;
  using converter_t
      = Engine::ossia_to_score::MatchingType<typename Property::param_type>;
  PropertyWrapper(
      ossia::net::node_base& param_node,
      ossia::net::parameter_base& param_addr,
      model_t& obj,
      QObject* context)
      : BaseCallbackWrapper{param_node, param_addr}
      , m_model{obj}
  {
    callbackIt
        = addr.add_callback([=](const ossia::value& v) {
      (m_model.*Property::set())(::State::convert::value<param_t>(v));
    });

    QObject::connect(
        &m_model, Property::notify(), context,
        [=] {
          auto newVal = converter_t::convert((m_model.*Property::get())());
          try
          {
            auto res = addr.value();

            if (newVal != res)
            {
              addr.push_value(newVal);
            }
          }
          catch (...)
          {
          }
        },
        Qt::QueuedConnection);

    addr.set_value(converter_t::convert((m_model.*Property::get())()));
  }
};

template <typename Property, typename Object>
auto add_property(
    ossia::net::node_base& n,
    Object& obj,
    QObject* context)
{
  constexpr const auto t = Engine::ossia_to_score::MatchingType<typename Property::param_type>::val;
  auto node = n.create_child(Property::name);
  SCORE_ASSERT(node);

  auto addr = node->create_parameter(t);
  SCORE_ASSERT(addr);

  addr->set_access(ossia::access_mode::BI);
  return std::make_unique<PropertyWrapper<Property>>(*node, *addr, obj, context);
}
}
}
