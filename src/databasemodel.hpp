#ifndef DATABASEMODEL_HPP_
#define DATABASEMODEL_HPP_

#include <QAbstractTableModel>
#include <filesystem>
#include <optional>
#include "statstable.hpp"

struct Record {
  QString coin;
  QString st_nd;
  QString result;
  QString deck;
  QString note;
  QString time;
  QString get_column(size_t index) const {
    switch (index) {
      case 0:  return coin;
      case 1:  return st_nd;
      case 2:  return result;
      case 3:  return deck;
      case 4:  return note;
      case 5:  return time;
      default: return {};
    }
  }
  QString &get_column_by_ref(size_t index) {
    switch (index) {
      case 0:  return coin;
      case 1:  return st_nd;
      case 2:  return result;
      case 3:  return deck;
      case 4:  return note;
      case 5:  return time;
      default: throw std::out_of_range(std::format("invalid index '{}'", index));
    }
  }
};

class Stats : public QAbstractTableModel {
  Q_OBJECT

  MyModule::StatsTable::EssentialData         essential_data;
  const MyModule::StatsTable::StatsTableText *stats_table_text;
  QList<QStringList>                          qt_stats_table_text;
  size_t                                      rowc;
  size_t                                      colc;

  void stdstringtext_to_qtstringtext();

public:
  Stats(QObject *parent = nullptr);
  ~Stats();

  // Essential methods to override
  int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int           columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant      headerData(int             section,
                           Qt::Orientation orientation,
                           int             role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  void clear_record(bool update = true);
  void add_record(const Record &record, bool inc, bool update = true);
  void copy_to_clipboard();

  void update_stats_tbl();
};

class DataBase : public QAbstractTableModel {
  Q_OBJECT

public:
  DataBase(QObject *parent = nullptr);
  ~DataBase();

  // Essential methods to override
  int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int           columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant      headerData(int             section,
                           Qt::Orientation orientation,
                           int             role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
  void warning_corrupted_csv(std::filesystem::path path);
  void good_csv(std::filesystem::path path);

public slots:
  // these two functions will not save csv automatically
  void   append_record(Record rec, bool update = true);
  size_t trunc_last(size_t n = 1, bool update = true);

  static bool ensure_csv(std::filesystem::path csv_path);

  bool load_csv(std::filesystem::path csv_path);
  bool save_csv();
  bool save_csv_as(std::filesystem::path csv_path);

  Stats *get_stats();

private:
  std::optional<std::filesystem::path> associate_csv_path;
  QStringList                          associate_csv_content;
  QStringList                          header_labels;
  QVector<Record>                      db;
  Stats                                stats;

  bool _setData_helper(QString &r, QString value, const QModelIndex &index, int role);
};

#endif
