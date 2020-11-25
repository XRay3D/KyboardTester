#pragma once

#include <QItemDelegate>

class QHBoxLayout;
class QDoubleSpinBox;

struct Widget : public QWidget {
    //        Q_OBJECT
    QHBoxLayout* horizontalLayout;
    QDoubleSpinBox* dsbX;
    QDoubleSpinBox* dsbY;
    void setupUi(QWidget* Form);
    void retranslateUi(QWidget* Form);

    //    public:
    Widget(QWidget* parent);
    virtual ~Widget();
};

class Delegate : public QItemDelegate {
    //    Q_OBJECT

    void commitEditor();

public:
    Delegate(QWidget* parent);
    virtual ~Delegate();

    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};
