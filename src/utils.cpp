#include "utils.hpp"

#include <print>
#include <fstream>

std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale)
{
    return [&ss, hwnd, scale]()
    {
        auto mat = ss.capture_window_mat(hwnd, scale);
        return mat.has_value() ? std::optional<cv::Mat>(mat.value())
                               : std::optional<cv::Mat>{};
    };
}
std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale, cv::Rect crop)
{
    return [&ss, hwnd, scale, crop]()
    {
        auto mat = ss.capture_window_mat(hwnd, scale);
        return mat.has_value() ? std::optional<cv::Mat>(mat.value()(crop))
                               : std::optional<cv::Mat>{};
    };
}

std::tm *get_local_time()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time);
    return local_time;
}

bool ensure_csv(std::filesystem::path csv)
{
    if (!std::filesystem::exists(csv))
    {
        std::filesystem::path csv_path_temp{csv};
        std::filesystem::create_directories(csv_path_temp.remove_filename());
        std::ofstream fout(csv, std::ios::out);
        if (!fout.good())
        {
            return false;
        }
        fout << "序号,硬币,先后,胜负,卡组,时间\n"
             << std::flush;
        return fout.good();
    }
    return true;
}

std::string format_csv_record(size_t no, size_t coin, size_t st_nd, size_t result,
                              const std::string &deck, const std::tm *t)
{
    return std::format("{},{},{},{},{},{}\n",
                       no,
                       coin == 0 ? "赢币" : "输币",
                       st_nd == 0 ? "先攻" : "后攻",
                       result == 0 ? "胜利" : "失败",
                       deck,
                       std::format("{:04}{:02}{:02}-{:02}{:02}{:02}",
                                   t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                   t->tm_hour, t->tm_min, t->tm_sec));
}

bool append_to_csv(const std::filesystem::path &csv, const std::string &record)
{
    std::ofstream fout(csv, std::ios::out | std::ios::app);
    if (!fout.good())
    {
        return false;
    }
    fout << record << std::flush;
    return fout.good();
}

bool check_resources(const std::vector<std::filesystem::path> &files)
{
    for (const auto &f : files)
    {
        if (!std::filesystem::exists(f))
        {
            return false;
        }
    }
    return true;
}

bool log(const std::string &l, LogLevel log_lv)
{
    if (log_lv >= globl_log_level)
    {
        std::clog << l << std::flush;
        return true;
    }
    return false;
}
bool logln(const std::string &l, LogLevel log_lv)
{
    if (log_lv >= globl_log_level)
    {
        std::clog << l << std::endl;
        return true;
    }
    return false;
}

DataBase::DataBase() : rowc(0)
{
    clear_stats();
}
void DataBase::clear_stats()
{
    w_st_wins = 0;
    l_st_wins = 0;
    w_nd_wins = 0;
    l_nd_wins = 0;
    w_st_loses = 0;
    l_st_loses = 0;
    w_nd_loses = 0;
    l_nd_loses = 0;
    w_st_others = 0;
    l_st_others = 0;
    w_nd_others = 0;
    l_nd_others = 0;
}

DataBase::DataBase(const std::string &csv_filename) : DataBase()
{
    load_csv(csv_filename);
}

void DataBase::append_record(std::string coin, std::string st_nd, std::string result,
                             std::string deck, std::string t)
{
    update_stats_by(true, coin, st_nd, result);
    ++rowc;
    coin_col.push_back(std::move(coin));
    st_nd_col.push_back(std::move(st_nd));
    result_col.push_back(std::move(result));
    deck_col.push_back(std::move(deck));
    time_col.push_back(std::move(t));
}
size_t DataBase::trunc_last(size_t n)
{
    size_t i = 0;
    for (; i < n; ++i)
    {
        if (rowc == 0)
        {
            break;
        }
        update_stats_by(false, coin_col[rowc - 1], st_nd_col[rowc - 1], result_col[rowc - 1]);
        --rowc;
        coin_col.pop_back();
        st_nd_col.pop_back();
        result_col.pop_back();
        deck_col.pop_back();
        time_col.pop_back();
    }
    return i;
}

void DataBase::load_csv(const std::string &csv_filename)
{
    rapidcsv::Document doc(csv_filename, rapidcsv::LabelParams(0, 0));
    rowc = doc.GetRowCount();
    coin_col = doc.GetColumn<std::string>("硬币");
    st_nd_col = doc.GetColumn<std::string>("先后");
    result_col = doc.GetColumn<std::string>("胜负");
    deck_col = doc.GetColumn<std::string>("卡组");
    time_col = doc.GetColumn<std::string>("时间");
    doc.Clear();

    clear_stats();
    for (size_t i; i < rowc; ++i)
    {
        update_stats_by(true, coin_col[i], st_nd_col[i], result_col[i]);
    }
}
void DataBase::update_stats_by(bool inc,
                               const std::string &coin,
                               const std::string &st_nd,
                               const std::string &result)
{
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

    f_exactly(w_st_wins, 0b001'01'01);
    f_exactly(l_st_wins, 0b001'01'10);
    f_exactly(w_nd_wins, 0b001'10'01);
    f_exactly(l_nd_wins, 0b001'10'10);
    f_exactly(w_st_loses, 0b010'01'01);
    f_exactly(l_st_loses, 0b010'01'10);
    f_exactly(w_nd_loses, 0b010'10'01);
    f_exactly(l_nd_loses, 0b010'10'10);
    f_exactly(w_st_others, 0b100'01'01);
    f_exactly(l_st_others, 0b100'01'10);
    f_exactly(w_nd_others, 0b100'10'01);
    f_exactly(l_nd_others, 0b100'10'10);
}

void CopyToClipboard(const char *text)
{
    // 打开剪贴板
    if (!OpenClipboard(nullptr))
        return;

    // 清空剪贴板
    EmptyClipboard();

    // 为文本分配全局内存
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size_needed * sizeof(wchar_t));
    if (!hGlobal)
    {
        CloseClipboard();
        return;
    }

    // 将文本复制到全局内存
    wchar_t *pGlobal = static_cast<wchar_t *>(GlobalLock(hGlobal));
    MultiByteToWideChar(CP_UTF8, 0, text, -1, pGlobal, size_needed);
    GlobalUnlock(hGlobal);

    // 设置剪贴板数据
    SetClipboardData(CF_UNICODETEXT, hGlobal);

    // 关闭剪贴板
    CloseClipboard();
}