#include "UndoPanelFactory.hpp"
#include "UndoPresenter.hpp"
#include "UndoModel.hpp"
#include "UndoView.hpp"
#include "UndoPanelId.hpp"

#include <core/view/View.hpp>

int UndoPanelFactory::panelId() const
{
    return UNDO_PANEL_ID;
}

QString UndoPanelFactory::panelName() const
{
    return "Undo";
}

iscore::PanelViewInterface *UndoPanelFactory::makeView(iscore::View *v)
{
    return new UndoView{v};
}

iscore::PanelPresenterInterface *UndoPanelFactory::makePresenter(iscore::Presenter *parent_presenter,
                                                                 iscore::PanelViewInterface *view)
{
    return new UndoPresenter{parent_presenter, view};
}

iscore::PanelModelInterface *UndoPanelFactory::makeModel(iscore::DocumentModel *m)
{
    return new UndoModel{m};
}
