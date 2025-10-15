#ifndef DELEGATE_HPP_
#define DELEGATE_HPP_

#include <QStyledItemDelegate>

class MyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MyDelegate(QObject *parent = nullptr, QStringList options = {}, bool editable = false);
    MyDelegate(QStringList options, bool editable = true);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void set_custom_text_option(QStringList options, bool editable = true);

private:
    QStringList options;
    bool editable;
};

#endif