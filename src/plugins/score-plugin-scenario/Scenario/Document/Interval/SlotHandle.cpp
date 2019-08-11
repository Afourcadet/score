// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "IntervalPresenter.hpp"
#include "SlotPresenter.hpp"

#include <Process/LayerView.hpp>
#include <Process/LayerPresenter.hpp>
#include <score/graphics/GraphicsItem.hpp>

#include <QPointer>
#include <QGraphicsScene>

namespace Scenario
{

void SlotPresenter::cleanupHeaderFooter()
{
  if (headerDelegate)
  {
    deleteGraphicsItem(headerDelegate);
    headerDelegate = nullptr;
  }
  if (footerDelegate)
  {
    deleteGraphicsItem(footerDelegate);
    footerDelegate = nullptr;
  }
}

void SlotPresenter::cleanup(QGraphicsScene* sc)
{
  if(sc)
  {
    if (header)
    {
      sc->removeItem(header);
      delete header;
      header = nullptr;
    }
    if (footer)
    {
      sc->removeItem(footer);
      delete footer;
      footer = nullptr;
    }
  }
  else
  {
    delete header;
    header = nullptr;
    delete footer;
    footer = nullptr;
  }

  for (LayerData& layer : processes)
  {
    // The presenter will delete their views
    QPointer<Process::LayerView> view_p{layer.view};
    delete layer.presenter;
    if (view_p)
      delete layer.view;
  }
  processes.clear();
}
}
