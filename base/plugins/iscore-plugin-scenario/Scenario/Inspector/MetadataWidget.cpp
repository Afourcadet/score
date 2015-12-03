#include <QBoxLayout>
#include <QColorDialog>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSize>
#include <Process//ModelMetadata.hpp>

#include "CommentEdit.hpp"
#include <Inspector/InspectorSectionWidget.hpp>
#include "MetadataWidget.hpp"
#include <iscore/command/Dispatchers/CommandDispatcher.hpp>

class QObject;


MetadataWidget::MetadataWidget(
        const ModelMetadata* metadata,
        CommandDispatcher<>* m,
        const QObject *docObject,
        QWidget* parent) :
    QWidget(parent),
    m_metadata {metadata},
    m_commandDispatcher{m}
{
    QVBoxLayout* metadataLayout = new QVBoxLayout{this};
    setLayout(metadataLayout);

    QHBoxLayout* typeLay = new QHBoxLayout{};
    // type
    m_typeLb = new QLabel("type");

    // LABEL : label + lineEdit in a container
    QWidget* descriptionWidget = new QWidget {this};
    QFormLayout* descriptionLay = new QFormLayout;

    m_scriptingNameLine = new QLineEdit{metadata->name(), this};
    m_labelLine = new QLineEdit{metadata->label(), this};

    descriptionLay->addRow("Name", m_scriptingNameLine);
    descriptionLay->addRow("Label", m_labelLine);

    descriptionWidget->setObjectName("Description");

    descriptionWidget->setLayout(descriptionLay);

     // color
    m_colorButton = new QPushButton{};
    m_colorButton->setMaximumSize(QSize(1.5 * m_colorIconSize, 1.5 * m_colorIconSize));
    m_colorButton->setIconSize(QSize(m_colorIconSize, m_colorIconSize));
    m_colorButtonPixmap.fill(metadata->color());
    m_colorButton->setIcon(QIcon(m_colorButtonPixmap));

    typeLay->addWidget(m_colorButton);
    typeLay->addWidget(m_typeLb);

    // comments
    m_comments = new CommentEdit{metadata->comment(), this};
    auto comments = new InspectorSectionWidget("Comments", false, this);
    comments->addContent(m_comments);
    comments->expand();

    metadataLayout->addLayout(typeLay);
    metadataLayout->addWidget(descriptionWidget);
    metadataLayout->addWidget(comments);

    connect(m_scriptingNameLine, &QLineEdit::editingFinished,
            [ = ]()
    {
        emit scriptingNameChanged(m_scriptingNameLine->text());
    });

    connect(m_labelLine, &QLineEdit::editingFinished,
            [ = ]()
    {
        emit labelChanged(m_labelLine->text());
    });

    connect(m_comments, &CommentEdit::editingFinished,
            [ = ]()
    {
        emit commentsChanged(m_comments->toPlainText());
    });

    connect(m_colorButton,  &QPushButton::clicked,
            this,           &MetadataWidget::changeColor);

    connect(metadata,   &ModelMetadata::metadataChanged,
            this,       &MetadataWidget::updateAsked);
}

QString MetadataWidget::scriptingName() const
{
    return m_scriptingNameLine->text();
}

void MetadataWidget::setScriptingName(QString arg)
{
    if(m_scriptingNameLine->text() == arg)
    {
        return;
    }

    m_scriptingNameLine->setText(arg);
    emit scriptingNameChanged(arg);
}

void MetadataWidget::changeColor()
{
    QColor color = QColorDialog::getColor(m_metadata->color(), this, "Select Color");

    if(color.isValid())
    {
        emit colorChanged(color);
    }
}

void MetadataWidget::setType(QString type)
{
    m_typeLb->setText(type);
}

void MetadataWidget::updateAsked()
{
    m_scriptingNameLine->setText(m_metadata->name());
    m_labelLine->setText(m_metadata->label());
    m_comments->setText(m_metadata->comment());

    m_colorButtonPixmap.fill(m_metadata->color());
    m_colorButton->setIcon(QIcon(m_colorButtonPixmap));
    // m_currentColor = newColor;
}
