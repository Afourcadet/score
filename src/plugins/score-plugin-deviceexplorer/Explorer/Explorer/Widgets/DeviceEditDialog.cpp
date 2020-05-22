// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "DeviceEditDialog.hpp"

#include <Device/Protocol/ProtocolFactoryInterface.hpp>
#include <Device/Protocol/ProtocolList.hpp>
#include <Device/Protocol/ProtocolSettingsWidget.hpp>

#include <score/plugins/InterfaceList.hpp>
#include <score/plugins/StringFactoryKey.hpp>
#include <score/widgets/SignalUtils.hpp>
#include <score/widgets/TextLabel.hpp>

#include <ossia/detail/algorithms.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QPushButton>
#include <QSplitter>
#include <QWidget>
#include <qnamespace.h>

#include <wobjectimpl.h>

#include <QListWidget>
#include <utility>
W_OBJECT_IMPL(Explorer::DeviceEditDialog)
namespace Explorer
{
DeviceEditDialog::DeviceEditDialog(const Device::ProtocolFactoryList& pl, QWidget* parent)
    : QDialog(parent), m_protocolList{pl}, m_protocolWidget{nullptr}, m_index(-1)
{
  setWindowTitle(tr("Add device"));
  auto gridLayout = new QGridLayout{};
  setLayout(gridLayout);
  setModal(true);

  m_buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_buttonBox->addButton(tr("Add"), QDialogButtonBox::AcceptRole );
  m_buttonBox->addButton(QDialogButtonBox::Cancel );

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  m_protocols = new QListWidget{this};
  m_protocols->setFixedWidth(150);

  const auto& skin = score::Skin::instance();
  {
    auto protocolTitle = new QLabel{tr("Protocols"), this};
    protocolTitle->setFont(skin.TitleFont);
    auto p = protocolTitle->palette();
    p.setColor(QPalette::WindowText, QColor("#D5D5D5"));
    protocolTitle->setPalette(p);
    gridLayout->addWidget(protocolTitle,0,0, Qt::AlignHCenter);
  }


  gridLayout->addWidget(m_protocols,1,0);

  m_devices = new QListWidget{this};
  m_devices->setFixedWidth(140);
  gridLayout->addWidget(m_devices,1,1);
  {
    m_devicesLabel = new QLabel{tr("Devices"), this};
    m_devicesLabel->setFont(skin.TitleFont);
    auto p = m_devicesLabel->palette();
    p.setColor(QPalette::WindowText, QColor("#D5D5D5"));
    m_devicesLabel->setPalette(p);
    gridLayout->addWidget(m_devicesLabel,0,1, Qt::AlignHCenter);
  }



  auto mainWidg = new QWidget{this};
  gridLayout->addWidget(mainWidg,0,2,-1,-1);
  m_layout = new QFormLayout;
  m_layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  mainWidg->setLayout(m_layout);
  m_layout->addWidget(m_buttonBox);

  initAvailableProtocols();

  connect(m_protocols, &QListWidget::currentRowChanged,
          this, [this] { selectedProtocolChanged(); });
  connect(m_devices, &QListWidget::currentRowChanged,
          this, [this] { selectedDeviceChanged(); });

  if (m_protocols->count() > 0)
  {
    m_protocols->setCurrentItem(m_protocols->item(0));
    m_index = m_protocols->currentRow();
    selectedProtocolChanged();
  }

  setMinimumWidth(600);
}

DeviceEditDialog::~DeviceEditDialog()
{

}

void DeviceEditDialog::initAvailableProtocols()
{
  // initialize previous settings
  m_previousSettings.clear();

  std::vector<Device::ProtocolFactory*> sorted;
  for (auto& elt : m_protocolList)
  {
    sorted.push_back(&elt);
  }

  ossia::sort(sorted, [](Device::ProtocolFactory* lhs, Device::ProtocolFactory* rhs) {
    return lhs->visualPriority() > rhs->visualPriority();
  });

  for (const auto& prot_pair : sorted)
  {
    auto& prot = *prot_pair;
    auto item = new QListWidgetItem;
    item->setText(prot.prettyName());
    item->setData(Qt::UserRole, QVariant::fromValue(prot.concreteKey()));
    m_protocols->addItem(item);

    m_previousSettings.append(prot.defaultSettings());
  }

  m_index = 0;
}

void DeviceEditDialog::selectedDeviceChanged()
{
  if(!m_devices->isVisible() || m_devices->count() == 0)
    return;
  auto item = m_devices->currentItem();
  if(!item)
    return;

  auto name = item->text();
  auto data = item->data(Qt::UserRole).value<Device::DeviceSettings>();
  if(m_protocolWidget)
    m_protocolWidget->setSettings(data);
}

void DeviceEditDialog::selectedProtocolChanged()
{
  // Clear listener
  if(m_enumerator)
  {
    m_enumerator.reset();
  }

  // Clear devices
  m_devices->clear();

  // Clear protocol widget
  if (m_protocolWidget)
  {
    SCORE_ASSERT(m_index < m_previousSettings.count());

    m_previousSettings[m_index] = m_protocolWidget->getSettings();
    delete m_protocolWidget;
    m_protocolWidget = nullptr;
  }

  // Recreate
  if(!m_protocols->currentItem())
  {
    m_devices->setVisible(false);
    m_devicesLabel->setVisible(false);
    return;
  }

  m_index = m_protocols->currentRow();
  auto key = m_protocols->currentItem()->data(Qt::UserRole).value<UuidKey<Device::ProtocolFactory>>();

  auto protocol = m_protocolList.get(key);
  m_enumerator.reset(protocol->getEnumerator(*(score::DocumentContext*)0));
  if(m_enumerator)
  {
    m_devices->setVisible(true);
    m_devicesLabel->setVisible(true);

    auto addItem = [&] (const Device::DeviceSettings& settings) {
      auto item = new QListWidgetItem;
      item->setText(settings.name);
      item->setData(Qt::UserRole, QVariant::fromValue(settings));
      m_devices->addItem(item);
    };
    auto rmItem = [&] (const QString& name) {
      auto items = m_devices->findItems(name, Qt::MatchExactly);
      for(auto item : items)
        delete m_devices->takeItem(m_devices->row(item));
    };
    connect(m_enumerator.get(), &Device::DeviceEnumerator::deviceAdded,
            this, addItem);
    connect(m_enumerator.get(), &Device::DeviceEnumerator::deviceRemoved,
            this, rmItem);
    m_enumerator->enumerate(addItem);
  }
  else
  {
    m_devices->setVisible(false);
    m_devicesLabel->setVisible(false);
  }
  m_protocolWidget = protocol->makeSettingsWidget();

  if (m_protocolWidget)
  {
    m_layout->insertRow(0, m_protocolWidget);

    QSizePolicy pol{QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding};
    pol.setVerticalStretch(1);
    m_protocolWidget->setSizePolicy(pol);
    m_protocolWidget->setMinimumHeight(200);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    updateGeometry();
  }
}

Device::DeviceSettings DeviceEditDialog::getSettings() const
{
  if (m_protocolWidget)
    return m_protocolWidget->getSettings();

  return {};
}

void DeviceEditDialog::setSettings(const Device::DeviceSettings& settings)
{
  for(int i = 0; i < m_protocols->count(); i++) {
    auto item = m_protocols->item(i);
    if(item->data(Qt::UserRole).value<UuidKey<Device::ProtocolFactory>>() == settings.protocol)
    {
      m_protocols->setCurrentRow(i);
      if(m_protocolWidget)
      {
        m_protocolWidget->setSettings(settings);
      }
      return;
    }
  }
}

void DeviceEditDialog::setEditingInvalidState(bool st)
{
  if (st != m_invalidState)
  {
    m_invalidState = st;
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(st);
  }
}
}
