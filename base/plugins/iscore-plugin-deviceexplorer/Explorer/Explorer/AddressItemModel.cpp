#include "AddressItemModel.hpp"
#include <ossia/editor/dataspace/dataspace_visitors.hpp>
#include <ossia/editor/value/value_traits.hpp>
#include <ossia/network/base/node_attributes.hpp>
#include <ossia/network/domain/domain.hpp>
#include <QPainter>
#include <iscore/command/Dispatchers/CommandDispatcher.hpp>
#include <Explorer/Commands/Update/UpdateAddressSettings.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Explorer/DocumentPlugin/NodeUpdateProxy.hpp>
#include <Explorer/Common/AddressSettings/Widgets/AddressSettingsWidget.hpp>
#include <State/Widgets/UnitWidget.hpp>
#include <ossia-qt/metatypes.hpp>
namespace Explorer
{
AddressItemModel::AddressItemModel(QObject* parent):
  QAbstractItemModel{parent}
{

}

void AddressItemModel::setState(
    DeviceExplorerModel* model,
    Device::NodePath nodepath,
    const Device::FullAddressSettings& s)
{
  beginResetModel();
  m_model = model;
  m_path = nodepath;
  m_settings = s;
  endResetModel();
}

void AddressItemModel::clear()
{
  beginResetModel();
  m_settings = {};
  m_model = nullptr;
  m_path = {};
  endResetModel();
}

bool AddressItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(index.column() != 1)
    return false;

  if(index.row() < 0 || index.row() > rowCount({}))
    return false;

  if(!m_model)
    return false;

  if(m_settings.address.path.empty())
    return false; // TODO just rename device ?


  Device::AddressSettings before, after;
  before.name = m_settings.address.path.last();
  (Device::AddressSettingsCommon&) before = m_settings;
  after = before;

  switch(index.row())
  {
    case Rows::Value:
    {
      // In this case we don't make a command, but we directly push the
      // new value.
      auto copy = State::convert::fromQVariant(value);

      // We may have to convert types.
      const ossia::value& orig = before.value;
      if (copy.v.which() != orig.v.which() && !State::convert::convert(orig, copy))
        return false;

      after.value = copy;

      // Note : if we want to disable remote updating, we have to do it
      // here (e.g. if this becomes a settings)
      m_model->deviceModel().updateProxy.updateRemoteValue(State::AddressAccessor{m_settings.address}, copy);
      return true;
    }

    case Rows::Type:
    {
      after.value = ossia::convert(before.value, value.value<ossia::val_type>());
      break;
    }

    case Rows::Min:
    {
      const auto& dom = before.domain.get();
      auto cur_min = dom.get_min();
      auto copy = State::convert::fromQVariant(value);
      if (copy.v.which() != cur_min.v.which() && !State::convert::convert(cur_min, copy))
        return false;

      after.domain.get().set_min(copy);
      break;
    }
    case Rows::Max:
    {
      const auto& dom = before.domain.get();
      auto cur_max = dom.get_max();
      auto copy = State::convert::fromQVariant(value);
      if (copy.v.which() != cur_max.v.which() && !State::convert::convert(cur_max, copy))
        return false;

      after.domain.get().set_max(copy);
      break;
    }
    case Rows::Access:
    {
      after.ioType = (ossia::access_mode)value.toInt();
      break;
    }
    case Rows::Bounding:
    {
      after.clipMode = (ossia::bounding_mode)value.toInt();
      break;
    }
    case Rows::Repetition:
    {
      after.repetitionFilter = value.toInt() != 0 ? ossia::repetition_filter::ON : ossia::repetition_filter::OFF;
      break;
    }
    case Rows::Description:
    {
      ossia::net::set_description(after.extendedAttributes, value.toString().toStdString());
      break;
    }
    case Rows::Tags:
    {
      ISCORE_TODO;
      break;
    }
    case Rows::Unit:
    {
      after.unit.get() = value.value<State::Unit>().get();
      break;
    }
  }

  auto node = (Device::Node*)m_model->convertPathToIndex(m_path).internalPointer();
  if(!node)
    return false;

  if (!m_model->checkAddressEditable(*node, before, after))
    return false;

  CommandDispatcher<> disp{m_model->commandStack()};
  disp.submitCommand(new Explorer::Command::UpdateAddressSettings{
      m_model->deviceModel(), m_path, after});

  return true;
}

QModelIndex AddressItemModel::index(int row, int column, const QModelIndex& parent) const
{
  if(parent == QModelIndex{})
  {
    return createIndex(row, column, nullptr);
  }
  return {};
}

QModelIndex AddressItemModel::parent(const QModelIndex& child) const
{
  return {};
}

int AddressItemModel::rowCount(const QModelIndex& parent) const
{
  if(m_settings.address.device.isEmpty())
    return 0;

  if(!m_settings.value.valid())
    return 2;

  return Rows::Count + m_settings.extendedAttributes.size();
}

int AddressItemModel::columnCount(const QModelIndex& parent) const
{
  return 2;
}

QVariant AddressItemModel::valueColumnData(const State::Value& val, int role) const
{
  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    if (ossia::is_array(val))
    {
      // TODO a nice editor for lists.
      return State::convert::toPrettyString(val);
    }
    else
    {
      return State::convert::value<QVariant>(val);
    }
  }

  return {};
}

QVariant AddressItemModel::data(const QModelIndex& index, int role) const
{
  if(role == Qt::DisplayRole)
  {
    if(!m_settings.value.valid())
    {
      switch(index.column())
      {
        case 0:
        {
          switch(index.row())
          {
            case Rows::Name: return tr("Name");
            case Rows::Address: return tr("Address");
          }
        }
        case 1:
        {
          switch(index.row())
          {
            case Rows::Name: return m_settings.address.path.last();
            case Rows::Address: return m_settings.address.toString();
          }
        }
        default: break;
      }
      return {};
    }

    switch(index.column())
    {
      case 0:
      {
        switch(index.row())
        {
          case Rows::Name: return tr("Name");
          case Rows::Address: return tr("Address");
          case Rows::Value: return tr("Value");
          case Rows::Type: return tr("Type");
          case Rows::Min: return tr("Min");
          case Rows::Max: return tr("Max");
          case Rows::Values: return tr("Values");
          case Rows::Unit: return tr("Unit");
          case Rows::Access: return tr("Access");
          case Rows::Bounding: return tr("Bounding");
          case Rows::Repetition: return tr("Repetition");
          case Rows::Description: return tr("Description");
          case Rows::Tags: return tr("Tags");
          default:
          {
            int idx = index.row() - Rows::Count;
            if(idx >= 0 && idx < m_settings.extendedAttributes.size())
            {
              auto it = m_settings.extendedAttributes.begin();
              std::advance(it, idx);
              return QString::fromStdString(it.key());
            }
          }
        }

        break;
      }
      case 1:
      {
        switch(index.row())
        {
          case Rows::Name:
            return m_settings.address.path.last();
          case Rows::Address:
            return m_settings.address.toString();
          case Rows::Value:
            return valueColumnData(m_settings.value, role);
          case Rows::Type:
          { return State::convert::ValuePrettyTypesArray()[(int)m_settings.value.getType()]; }
          case Rows::Min:
          { return valueColumnData(ossia::get_min(m_settings.domain.get()), role); }
          case Rows::Max:
          { return valueColumnData(ossia::get_max(m_settings.domain.get()), role); }
          case Rows::Values:
          { return valueColumnData(ossia::get_values(m_settings.domain.get()), role); }
          case Rows::Unit:
          { return QString::fromStdString(ossia::get_pretty_unit_text(m_settings.unit.get())); }
          case Rows::Access:
          { return m_settings.ioType ? Device::AccessModeText()[*m_settings.ioType] : tr("None"); }
          case Rows::Bounding:
          { return Device::ClipModePrettyStringMap()[m_settings.clipMode]; }
          case Rows::Repetition:
          { return m_settings.repetitionFilter == ossia::repetition_filter::ON ? tr("Filtered") : tr("Unfiltered"); }
          case Rows::Description:
          {
            auto desc = ossia::net::get_description(m_settings.extendedAttributes);
            if(desc)
              return QString::fromStdString(*desc);
            return {};
          }
          case Rows::Tags:
          {
            QStringList l;

            const auto& tags = ossia::net::get_tags(m_settings.extendedAttributes);
            if(tags)
              for(const auto& s : *tags)
                l += QString::fromStdString(s);

            return l;
          }
          default:
          {
            return {};
          }
        }

        break;
      }
      default:
        break;
    }

  }
  else if(role == Qt::EditRole)
  {
    if(index.column() == 1 && index.row() == Rows::Type)
    {
      switch(index.row())
      {
        case Rows::Type: return (int)m_settings.value.getType();
        case Rows::Access: return (int)*m_settings.ioType;
        case Rows::Bounding: return (int)m_settings.clipMode;
        case Rows::Unit: return QVariant::fromValue(m_settings.unit);
      }
    }
    else
    {
      return {};
    }
  }
  else if(role == Qt::CheckStateRole)
  {
    if(index.column() == 1 && index.row() == Rows::Repetition)
    {
      return m_settings.repetitionFilter == ossia::repetition_filter::ON ? Qt::Checked : Qt::Unchecked;
    }
  }

  return {};
}

Qt::ItemFlags AddressItemModel::flags(const QModelIndex& index) const
{
  if(index.column() == 0)
    return { Qt::ItemIsEnabled };

  Qt::ItemFlags f = QAbstractItemModel::flags(index);
  static const constexpr std::array<Qt::ItemFlags, Rows::Count> flags{{
      { } // name
    , { } // address
    , { Qt::ItemIsEditable } // value
    , { Qt::ItemIsEditable } // type
    , { Qt::ItemIsEditable } // min
    , { Qt::ItemIsEditable } // max
    , { Qt::ItemIsEditable } // values
    , { Qt::ItemIsEditable } // unit
    , { Qt::ItemIsEditable } // access
    , { Qt::ItemIsEditable } // bounding
    , { Qt::ItemIsUserCheckable | Qt::ItemIsEnabled } // repetition
    , { Qt::ItemIsEditable } // description
    , { Qt::ItemIsEditable } // tags
  }};

  if(index.row() < Rows::Count)
    f |= flags[index.row()];

  return f;
}



AddressItemDelegate::AddressItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

AddressItemDelegate::~AddressItemDelegate()
{
}

void AddressItemDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);
  /*
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
  QStyle *style = widget ? widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
  */
}

QWidget*AddressItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  if(index.column() == 0)
    return QStyledItemDelegate::createEditor(parent, option, index);

  switch(index.row())
  {
    case AddressItemModel::Rows::Type:
    {
      auto t = new State::TypeComboBox{parent};
      t->set(index.data(Qt::EditRole).value<ossia::val_type>());
      return t;
    }
    case AddressItemModel::Rows::Access:
    {
      auto t = new Explorer::AccessModeComboBox{parent};
      t->set(index.data(Qt::EditRole).value<ossia::access_mode>());
      return t;
    }
    case AddressItemModel::Rows::Bounding:
    {
      auto t = new Explorer::BoundingModeComboBox{parent};
      t->set(index.data(Qt::EditRole).value<ossia::bounding_mode>());
      return t;
    }
    case AddressItemModel::Rows::Unit:
    {
      auto t = new State::UnitWidget{parent};
      t->setUnit(index.data(Qt::EditRole).value<State::Unit>());
      return t;
    }
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}

void AddressItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if(index.column() == 0)
  {
    QStyledItemDelegate::setEditorData(editor, index);
    return;
  }

  switch(index.row())
  {
    case AddressItemModel::Rows::Type:
    {
      if (auto cb = qobject_cast<State::TypeComboBox*>(editor))
      {
        auto cur = index.data(Qt::EditRole).toInt();
        if(cur >= 0 && cur < State::convert::ValuePrettyTypesArray().size())
          cb->set((ossia::val_type) cur);
        return;
      }
      break;
    }
    case AddressItemModel::Rows::Access:
    {
      if (auto cb = qobject_cast<Explorer::AccessModeComboBox*>(editor))
      {
        auto cur = index.data(Qt::EditRole).toInt();
        if(cur >= 0 && cur < Device::AccessModeText().size())
          cb->set((ossia::access_mode) cur);
        return;
      }
      break;
    }
    case AddressItemModel::Rows::Bounding:
    {
      if (auto cb = qobject_cast<Explorer::BoundingModeComboBox*>(editor))
      {
        auto cur = index.data(Qt::EditRole).toInt();
        if(cur >= 0 && cur < Device::ClipModePrettyStringMap().size())
          cb->set((ossia::bounding_mode) cur);
        return;
      }
      break;
    }
    case AddressItemModel::Rows::Unit:
    {
      if (auto cb = qobject_cast<State::UnitWidget*>(editor))
      {
        auto cur = index.data(Qt::EditRole).value<State::Unit>();
        cb->setUnit(cur);
        return;
      }
      break;
    }
  }

  QStyledItemDelegate::setEditorData(editor, index);
}

void AddressItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if(index.column() == 0)
  {
    QStyledItemDelegate::setModelData(editor, model, index);
    return;
  }

  switch(index.row())
  {
    case AddressItemModel::Rows::Type:
    {
      if (auto cb = qobject_cast<State::TypeComboBox*>(editor))
      {
        model->setData(index, cb->itemData(cb->currentIndex()), Qt::EditRole);
      }
      return;
    }
    case AddressItemModel::Rows::Access:
    {
      if (auto cb = qobject_cast<Explorer::AccessModeComboBox*>(editor))
      {
        model->setData(index, cb->itemData(cb->currentIndex()), Qt::EditRole);
      }
      return;
    }
    case AddressItemModel::Rows::Bounding:
    {
      if (auto cb = qobject_cast<Explorer::BoundingModeComboBox*>(editor))
      {
        model->setData(index, cb->itemData(cb->currentIndex()), Qt::EditRole);
      }
      return;
    }
    case AddressItemModel::Rows::Unit:
    {
      if (auto cb = qobject_cast<State::UnitWidget*>(editor))
      {
        model->setData(index, QVariant::fromValue(cb->unit()), Qt::EditRole);
      }
      return;
    }
  }

  QStyledItemDelegate::setModelData(editor, model, index);
}

}
