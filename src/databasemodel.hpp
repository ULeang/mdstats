#ifndef DATABASEMODEL_HPP_
#define DATABASEMODEL_HPP_

#include <QAbstractTableModel>
#include <optional>
#include "utils.hpp"

struct Record
{
    QString coin;
    QString st_nd;
    QString result;
    QString deck;
    QString note;
    QString time;
};

class Stats : public QAbstractTableModel
{

    Q_OBJECT

    static const size_t rowc = 6;
    static const size_t colc = 3;

    size_t total;
    size_t w_st_wins;
    size_t l_st_wins;
    size_t w_nd_wins;
    size_t l_nd_wins;
    size_t w_st_loses;
    size_t l_st_loses;
    size_t w_nd_loses;
    size_t l_nd_loses;
    size_t w_st_others;
    size_t l_st_others;
    size_t w_nd_others;
    size_t l_nd_others;

    struct
    {
        QStringList row_data;
    } stats_tbl[rowc];

    void update_stats_tbl();

public:
    Stats(QObject *parent = nullptr);
    ~Stats();

    // Essential methods to override
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void clear_stats();
    void add_record(bool inc, const Record &record);
    void copy_to_clipboard();
};

class DataBase : public QAbstractTableModel
{
    Q_OBJECT

public:
    DataBase(QObject *parent = nullptr);
    ~DataBase();

    // Essential methods to override
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

signals:
    void warning_corrupted_csv(std::filesystem::path path);
    void good_csv(std::filesystem::path path);

public slots:
    // these two functions will not save csv automatically
    void append_record(Record rec);
    size_t trunc_last(size_t n = 1);

    static bool ensure_csv(std::filesystem::path csv_path);

    bool load_csv(std::filesystem::path csv_path);
    bool save_csv();

    Stats *get_stats();

private:
    std::optional<std::filesystem::path> associate_csv_path;
    QStringList associate_csv_content;
    QStringList header_labels;
    QVector<Record> db;
    Stats stats;

    bool _setData_helper(QString &r, QString value, const QModelIndex &index, int role);
};

#endif