#include "delegate.hpp"

#include <QComboBox>

MyDelegate::MyDelegate(QObject *parent, QStringList _options, bool _editable)
  : QStyledItemDelegate(parent), options(_options), editable(_editable) {}
MyDelegate::MyDelegate(QStringList _options, bool _editable)
  : MyDelegate(nullptr, _options, _editable) {}

QWidget *MyDelegate::createEditor(QWidget                    *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex          &index) const {
  auto cbox = new QComboBox(parent);
  cbox->addItems(options);
  cbox->setEditable(editable);
  return cbox;
}
void MyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  QComboBox *cbox = dynamic_cast<QComboBox *>(editor);
  cbox->setCurrentText(index.data(Qt::EditRole).toString());
}
void MyDelegate::updateEditorGeometry(QWidget                    *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex          &index) const {
  QRect rect{option.rect};
  rect.setWidth(rect.width() + 20);
  editor->setGeometry(rect);
}
void MyDelegate::setModelData(QWidget            *editor,
                              QAbstractItemModel *model,
                              const QModelIndex  &index) const {
  QComboBox *cbox = dynamic_cast<QComboBox *>(editor);
  model->setData(index, cbox->currentText(), Qt::EditRole);
}

void MyDelegate::set_custom_text_option(QStringList _options, bool _editable) {
  options  = _options;
  editable = _editable;
}
