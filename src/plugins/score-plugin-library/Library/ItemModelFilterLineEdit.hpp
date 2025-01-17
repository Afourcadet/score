#pragma once
#include <score/widgets/SearchLineEdit.hpp>

#include <QSortFilterProxyModel>
#include <QTreeView>

#include <functional>

namespace Library
{

struct ItemModelFilterLineEdit final : public score::SearchLineEdit
{
public:
  ItemModelFilterLineEdit(
      QSortFilterProxyModel& proxy,
      QTreeView& tv,
      QWidget* p)
      : score::SearchLineEdit{p}
      , m_proxy{proxy}
      , m_view{tv}
  {
    connect(this, &QLineEdit::textEdited, this, [=] { search(); });
  }

  void search() override
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (text() != m_proxy.filterRegExp().pattern())
#else
    if (text() != m_proxy.filterRegularExpression().pattern())
#endif
    {
      m_proxy.setFilterFixedString(text());
      m_proxy.setFilterCaseSensitivity(Qt::CaseInsensitive);

      if (!text().isEmpty())
      {
        m_view.expandAll();
      }
    }

    if (text().isEmpty())
    {
      m_proxy.invalidate();
      m_view.collapseAll();
    }
    if (reset)
    {
      reset();
    }
  }

  std::function<void()> reset;
  QSortFilterProxyModel& m_proxy;
  QTreeView& m_view;
};

}
