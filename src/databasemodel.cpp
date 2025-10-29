#include "databasemodel.hpp"
#include "prog.hpp"

#include <QFile>
#include <QTextStream>
#include <QColor>

#define HAS_CODECVT
#include <rapidcsv.h>

void Stats::update_stats_tbl()
{
    MyModule::StatsTable::update_stats_table_text(essential_data);
    emit dataChanged(index(0, 0), index(rowc - 1, colc - 1));
}

Stats::Stats(QObject *parent)
    : QAbstractTableModel(parent)
{
    clear_record(false);
    rowc = MyModule::StatsTable::rows_count();
    colc = MyModule::StatsTable::cols_count();
    stats_table_text = MyModule::StatsTable::stats_table_text();
}
Stats::~Stats() {}

// Essential methods to override
int Stats::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) // For list/table models, parent should be invalid
        return 0;
    return rowc;
}
int Stats::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) // For list/table models, parent should be invalid
        return 0;
    return colc;
}
QVariant Stats::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() >= rowc || index.column() >= colc)
        return {};

    switch (role)
    {
    case Qt::DisplayRole:
        return QString{(*stats_table_text)[index.row()][index.column()].c_str()};
    case Qt::TextAlignmentRole:
        return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::BackgroundRole:
        return prog::env::config::preprocessed::stats_tbl_color_background;
    case Qt::ForegroundRole:
        return prog::env::config::preprocessed::stats_tbl_color_foreground;
    case Qt::FontRole:
    default:
        return {};
    }
}
QVariant Stats::headerData(int section, Qt::Orientation orientation, int role) const
{
    return {};
}
Qt::ItemFlags Stats::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index);
}
bool Stats::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

void Stats::clear_record(bool update)
{
    essential_data.total = 0;
    essential_data.w_st_wins = 0;
    essential_data.l_st_wins = 0;
    essential_data.w_nd_wins = 0;
    essential_data.l_nd_wins = 0;
    essential_data.w_st_loses = 0;
    essential_data.l_st_loses = 0;
    essential_data.w_nd_loses = 0;
    essential_data.l_nd_loses = 0;
    essential_data.w_st_others = 0;
    essential_data.l_st_others = 0;
    essential_data.w_nd_others = 0;
    essential_data.l_nd_others = 0;
    if (update)
        update_stats_tbl();
}

void Stats::add_record(const Record &record, bool inc, bool update)
{
    const auto &coin = record.coin;
    const auto &st_nd = record.st_nd;
    const auto &result = record.result;

    size_t ont_hot = 0b000'00'00;
    ont_hot |= coin == "赢币" ? 0b000'00'01 : 0b000'00'10;
    ont_hot |= st_nd == "先攻" ? 0b000'01'00 : 0b000'10'00;
    ont_hot |= result == "胜利"   ? 0b001'00'00
               : result == "失败" ? 0b010'00'00
                                  : 0b100'00'00;

    auto f_exactly = [ont_hot, inc](size_t &n, size_t mask)
    {
        n = (ont_hot ^ mask) == 0 ? (inc ? n + 1 : n - 1) : n;
    };
    // auto f_any = [ont_hot, inc](size_t &n, size_t mask)
    // {
    //     n = ont_hot & mask ? (inc ? n + 1 : n - 1) : n;
    // };

    inc ? essential_data.total += 1 : essential_data.total -= 1;
    f_exactly(essential_data.w_st_wins, 0b001'01'01);
    f_exactly(essential_data.l_st_wins, 0b001'01'10);
    f_exactly(essential_data.w_nd_wins, 0b001'10'01);
    f_exactly(essential_data.l_nd_wins, 0b001'10'10);
    f_exactly(essential_data.w_st_loses, 0b010'01'01);
    f_exactly(essential_data.l_st_loses, 0b010'01'10);
    f_exactly(essential_data.w_nd_loses, 0b010'10'01);
    f_exactly(essential_data.l_nd_loses, 0b010'10'10);
    f_exactly(essential_data.w_st_others, 0b100'01'01);
    f_exactly(essential_data.l_st_others, 0b100'01'10);
    f_exactly(essential_data.w_nd_others, 0b100'10'01);
    f_exactly(essential_data.l_nd_others, 0b100'10'10);
    if (update)
        update_stats_tbl();
}
void Stats::copy_to_clipboard()
{
    CopyToClipboard(
        QString{
            "%1\t%2\n%3\t%4/%5\n%6\t%7/%8\n%9\t%10\n%11\t%12\n%13\t%14\n",
        }
            .arg((*stats_table_text)[0][0])
            .arg((*stats_table_text)[0][1])
            .arg((*stats_table_text)[1][0])
            .arg((*stats_table_text)[1][1])
            .arg((*stats_table_text)[1][2])
            .arg((*stats_table_text)[2][0])
            .arg((*stats_table_text)[2][1])
            .arg((*stats_table_text)[2][2])
            .arg((*stats_table_text)[3][0])
            .arg((*stats_table_text)[3][1])
            .arg((*stats_table_text)[4][0])
            .arg((*stats_table_text)[4][1])
            .arg((*stats_table_text)[5][0])
            .arg((*stats_table_text)[5][1])
            .toStdString()
            .c_str());
}

DataBase::DataBase(QObject *parent) : QAbstractTableModel(parent)
{
    header_labels << "硬币" << "先后" << "胜负" << "卡组" << "备注" << "时间";
}
DataBase::~DataBase() {}

// Essential methods to override
int DataBase::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) // For list/table models, parent should be invalid
        return 0;
    return db.size();
}
int DataBase::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) // For list/table models, parent should be invalid
        return 0;
    return header_labels.size();
}
QVariant DataBase::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() >= db.size() || index.column() >= header_labels.size())
        return {};

    const auto &rec = db[index.row()];

    using namespace prog::env::config::preprocessed;
    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return index.column() == 0   ? rec.coin
               : index.column() == 1 ? rec.st_nd
               : index.column() == 2 ? rec.result
               : index.column() == 3 ? rec.deck
               : index.column() == 4 ? rec.note
                                     : rec.time;
    case Qt::TextAlignmentRole:
        return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::BackgroundRole:
        return index.column() == 0   ? (rec.coin == "赢币"
                                            ? record_tbl_color_coin_win_background
                                            : record_tbl_color_coin_lose_background)
               : index.column() == 1 ? (rec.st_nd == "先攻"
                                            ? record_tbl_color_st_nd_first_background
                                            : record_tbl_color_st_nd_second_background)
               : index.column() == 2 ? (rec.result == "胜利"
                                            ? record_tbl_color_result_victory_background
                                        : rec.result == "失败"
                                            ? record_tbl_color_result_defeat_background
                                            : record_tbl_color_result_other_background)
                                     : QVariant{};
    case Qt::ForegroundRole:
        return index.column() == 0   ? (rec.coin == "赢币"
                                            ? record_tbl_color_coin_win_foreground
                                            : record_tbl_color_coin_lose_foreground)
               : index.column() == 1 ? (rec.st_nd == "先攻"
                                            ? record_tbl_color_st_nd_first_foreground
                                            : record_tbl_color_st_nd_second_foreground)
               : index.column() == 2 ? (rec.result == "胜利"
                                            ? record_tbl_color_result_victory_foreground
                                        : rec.result == "失败"
                                            ? record_tbl_color_result_defeat_foreground
                                            : record_tbl_color_result_other_foreground)
                                     : QVariant{};
    case Qt::FontRole:
    default:
        return {};
    }
}
QVariant DataBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return int(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
        return orientation == Qt::Horizontal
                   ? (section >= 0 && section < header_labels.size() ? header_labels[section] : QVariant{})
                   : section + 1;
    default:
        return {};
    }
}
Qt::ItemFlags DataBase::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}
bool DataBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.row() >= db.size() || index.column() >= header_labels.size())
        return false;

    auto &rec = db[index.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        if (index.column() == 0)
            return _setData_helper(rec.coin, std::move(value.toString()), index, role);
        else if (index.column() == 1)
            return _setData_helper(rec.st_nd, std::move(value.toString()), index, role);
        else if (index.column() == 2)
            return _setData_helper(rec.result, std::move(value.toString()), index, role);
        else if (index.column() == 3)
            return _setData_helper(rec.deck, std::move(value.toString()), index, role);
        else if (index.column() == 4)
            return _setData_helper(rec.note, std::move(value.toString()), index, role);
        else
            return _setData_helper(rec.time, std::move(value.toString()), index, role);
    }
    break;
    default:
        return false;
    }
}

static QString format_record(size_t no, const Record &record)
{
    return QString{"%1|%2|%3|%4|%5|%6|%7"}
        .arg(no)
        .arg(record.coin)
        .arg(record.st_nd)
        .arg(record.result)
        .arg(record.deck)
        .arg(record.note)
        .arg(record.time);
}

// these two functions will not save csv automatically
void DataBase::append_record(Record rec, bool update)
{
    stats.add_record(rec, true, update);
    associate_csv_content.push_back(format_record(db.size() + 1, rec));
    beginInsertRows({}, db.size(), db.size());
    db.push_back(std::move(rec));
    endInsertRows();
}
size_t DataBase::trunc_last(size_t n)
{
    size_t _n = std::min(n, size_t(db.size()));
    auto first = db.size() - _n;
    if (_n != 0)
    {
        beginRemoveRows({}, first, db.size() - 1);
        for (size_t i = 0; i < _n; ++i)
        {
            stats.add_record(db.back(), false, false);
            db.pop_back();
            associate_csv_content.pop_back();
        }
        endRemoveRows();
        stats.update_stats_tbl();
    }
    return _n;
}

// static std::optional<std::string> load_csv_into_string(const std::filesystem::path &csv_path)
// {
//     std::ifstream fin(csv_path, std::ios::in);
//     if (!fin.good())
//     {
//         return std::nullopt;
//     }
//     std::ostringstream buf;
//     buf << fin.rdbuf();
//     return buf.str();
// }

static QString csv_head{"序号|硬币|先后|胜负|卡组|备注|时间"};
bool DataBase::ensure_csv(std::filesystem::path csv_path)
{
    if (!std::filesystem::exists(csv_path))
    {
        auto _csv_path = csv_path;
        _csv_path.remove_filename();
        std::filesystem::create_directories(_csv_path);
        QFile file(csv_path);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
        {
            return false;
        }
        QTextStream fout(&file);
        fout.setEncoding(QStringConverter::Encoding::Utf8);
        fout << csv_head << Qt::endl;
        return fout.status() == QTextStream::Status::Ok;
    }
    return true;
}

bool DataBase::save_csv()
{
    if (!associate_csv_path.has_value())
    {
        return false;
    }
    return save_csv_as(associate_csv_path.value());
}
bool DataBase::save_csv_as(std::filesystem::path csv_path)
{
    QFile file(csv_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        logln("DataBase : cannot open csv when save");
        return false;
    }
    QTextStream fout(&file);
    fout.setEncoding(QStringConverter::Encoding::Utf8);
    fout << csv_head << Qt::endl;
    for (const auto &line : associate_csv_content)
    {
        fout << line << Qt::endl;
    }
    fout.flush();

    return fout.status() == QTextStream::Status::Ok;
}
bool DataBase::load_csv(std::filesystem::path csv_path)
{
    associate_csv_path = std::nullopt;
    trunc_last(db.size());

    std::ifstream fin(csv_path, std::ios::in);
    if (!fin.good())
    {
        emit warning_corrupted_csv(csv_path);
        logln("DataBase : cannot open csv when load");
        return false;
    }
    try
    {
        rapidcsv::Document doc(fin, rapidcsv::LabelParams(0, 0), rapidcsv::SeparatorParams('|'));
        auto rowc = doc.GetRowCount();
        auto coin_col = doc.GetColumn<QString>("硬币");
        auto st_nd_col = doc.GetColumn<QString>("先后");
        auto result_col = doc.GetColumn<QString>("胜负");
        auto deck_col = doc.GetColumn<QString>("卡组");
        auto note_col = doc.GetColumn<QString>("备注");
        auto time_col = doc.GetColumn<QString>("时间");
        db.reserve(rowc);
        for (size_t i = 0; i < rowc; ++i)
        {
            append_record({std::move(coin_col[i]),
                           std::move(st_nd_col[i]),
                           std::move(result_col[i]),
                           std::move(deck_col[i]),
                           std::move(note_col[i]),
                           std::move(time_col[i])},
                          false);
        }
        stats.update_stats_tbl();
        associate_csv_path = std::move(csv_path);
        emit good_csv(csv_path);
        return true;
    }
    catch (const std::out_of_range &e)
    {
        trunc_last(db.size());
        associate_csv_path = std::nullopt;
        emit warning_corrupted_csv(csv_path);
        logln(std::format("load csv exception :\n\t{}", e.what()));
        logln("csv file is corrupted, fix it first or just remove it and try again");
    }
    catch (...)
    {
        trunc_last(db.size());
        associate_csv_path = std::nullopt;
        emit warning_corrupted_csv(csv_path);
        logln(std::format("load csv exception :\n\tunknown exception"));
    }
    return false;
}

Stats *DataBase::get_stats()
{
    return &stats;
}

bool DataBase::_setData_helper(QString &r, QString value, const QModelIndex &index, int role)
{
    if (r == value)
        return false;
    stats.add_record(db[index.row()], false, false);
    r = std::move(value);
    emit dataChanged(index, index, {role});
    stats.add_record(db[index.row()], true, true);

    associate_csv_content[index.row()] = format_record(index.row() + 1, db[index.row()]);
    save_csv();
    return true;
}

namespace rapidcsv
{
    template <>
    void Converter<QString>::ToVal(const std::string &pStr, QString &pVal) const
    {
        pVal = QString{pStr.c_str()};
    }
    template <>
    void Converter<QString>::ToStr(const QString &pVal, std::string &pStr) const
    {
        pStr = pVal.toStdString();
    }
}