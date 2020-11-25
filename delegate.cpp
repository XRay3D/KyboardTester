#include "delegate.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>

enum {
    Name,
    Pos,
    Res
};

Delegate::Delegate(QWidget* parent)
    : QItemDelegate(parent)
{
}

Delegate::~Delegate() { }

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    switch (index.column()) {
    case Name: {
        auto widget = new QLineEdit(parent);
        widget->setAlignment(Qt::AlignCenter);
        return widget;
    }
    case Pos: {
        auto widget = new Widget(parent);
        widget->dsbX->setMinimumHeight(option.rect.height());
        widget->dsbY->setMinimumHeight(option.rect.height());
        return widget;
    }
    default: {
        auto widget = new QSpinBox(parent);
        widget->setRange(0, 1000000);
        widget->setButtonSymbols(QAbstractSpinBox::NoButtons);
        widget->setAlignment(Qt::AlignCenter);
        return widget;
    }
    }
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == Pos) {
        QPointF pt { index.data(Qt::EditRole).toPointF() };
        auto widget = static_cast<Widget*>(editor);
        widget->dsbX->setValue(pt.x());
        widget->dsbY->setValue(pt.x());
    } else {
        QItemDelegate::setEditorData(editor, index);
    }
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() == Pos) {
        auto widget = static_cast<Widget*>(editor);
        model->setData(index, QPointF { widget->dsbX->value(), widget->dsbY->value() });
    } else {
        QItemDelegate::setModelData(editor, model, index);
    }
}

////////////////////////////////////////////////////////////////
/// \brief Widget::Widget
/// \param parent
///
Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
}

Widget::~Widget() { }

void Widget::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QString::fromUtf8("Form"));

    dsbX = new QDoubleSpinBox(Form);
    dsbX->setObjectName(QString::fromUtf8("dsbX"));
    dsbX->setAlignment(Qt::AlignCenter);
    dsbX->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbX->setDecimals(3);

    dsbY = new QDoubleSpinBox(Form);
    dsbY->setObjectName(QString::fromUtf8("dsbY"));
    dsbY->setAlignment(Qt::AlignCenter);
    dsbY->setButtonSymbols(QAbstractSpinBox::NoButtons);
    dsbY->setDecimals(3);

    horizontalLayout = new QHBoxLayout(Form);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->addWidget(dsbX);
    horizontalLayout->addWidget(dsbY);

    retranslateUi(Form);
    QMetaObject::connectSlotsByName(Form);
}

void Widget::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(QApplication::translate("Form", "Form", nullptr));
    dsbX->setPrefix(QApplication::translate("Form", "X: ", nullptr));
    dsbY->setPrefix(QApplication::translate("Form", "Y: ", nullptr));
}
