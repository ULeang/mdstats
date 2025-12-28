#ifndef OPENCV_MATCHER_HPP_
#define OPENCV_MATCHER_HPP_

#include <expected>
#include <fstream>
#include <functional>
#include <opencv2/opencv.hpp>
#include "prog_config.hpp"
#include "utils.hpp"

template<int CV_FORMAT, int CV_MATCH_METHOD>
class T_Matcher;

template<>
class T_Matcher<CV_8UC3, cv::TM_CCOEFF_NORMED> {
  std::vector<cv::Mat> templ;
  std::ofstream        _flog;

  // used by try_once()
  std::vector<std::tuple<size_t, double, cv::Point>> all_res;

public:
  const size_t size;
  double       threshold;
  bool         log;
  bool         text_log;

  explicit T_Matcher(std::initializer_list<const char *> img_filepath,
                     double                              threshold = 0.9,
                     bool                                log       = false,
                     bool                                text_log  = true)
    : size(img_filepath.size()), threshold(threshold), log(log), text_log(text_log), _flog() {
    if (prog::env::debug::matcher_text_log) {
      _flog.open("log.txt", std::ios::out | std::ios::app);
    }
    templ.reserve(size);
    all_res.reserve(size);
    std::ranges::for_each(img_filepath.begin(), img_filepath.end(), [this](const char *p) {
      templ.push_back(cv::imread(p, cv::IMREAD_COLOR));
    });
  }

  std::expected<size_t, MatcherFailT> try_once(const std::function<std::optional<cv::Mat>()> &f) {
    auto finput = f();
    if (!finput.has_value()) {
      logln("cannot get image");
      return std::unexpected(MatcherFailT::CannotGetImage);
    }
    cv::Mat image = finput.value();

    for (size_t i = 0; i < size; ++i) {
      cv::Mat result;
      result.create(image.rows - templ[i].rows + 1, image.cols - templ[i].cols + 1, CV_8UC3);

      cv::matchTemplate(image, templ[i], result, cv::TM_CCOEFF_NORMED);

      double    minVal, maxVal;
      cv::Point minLoc, maxLoc;
      minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

      if (maxVal > threshold) {
        all_res.push_back({i, maxVal, maxLoc});
      }
    }
    if (all_res.empty()) {
      return std::unexpected(MatcherFailT::TryNExceed);
    }

    auto [c_i, c_maxVal, c_maxLoc] = *std::max_element(
      all_res.begin(), all_res.end(),
      [](const auto &_a, const auto &_b) { return std::get<1>(_a) < std::get<1>(_b); });

    if (log) {
      cv::rectangle(image, c_maxLoc,
                    cv::Point(c_maxLoc.x + templ[c_i].cols, c_maxLoc.y + templ[c_i].rows),
                    cv::Scalar(0, 0, 255), 4);

      auto local_time = get_local_time();

      // the filename may not contain colon ':', otherwise imwrite fails
      cv::imwrite(std::format("{}\\{:04}{:02}{:02}-{:02}{:02}{:02}-{:.5f}.png",
                              prog::env::opencv_log_directory, local_time->tm_year + 1900,
                              local_time->tm_mon + 1, local_time->tm_mday, local_time->tm_hour,
                              local_time->tm_min, local_time->tm_sec, c_maxVal)
                    .c_str(),
                  image);
      // TODO
      if (text_log) {
      }
    }

    all_res.clear();

    return c_i;
  }

  std::expected<size_t, MatcherFailT> keep_try(const std::function<std::optional<cv::Mat>()> &f,
                                               DWORD sleep = 1 * 1000) {
    while (true) {
      auto res = try_once(f);
      if (res.has_value()) {
        return res.value();
      }
      if (res.error() == MatcherFailT::TryNExceed) {
        Sleep(sleep);
        continue;
      }
      return res;
    }
  }

  std::expected<size_t, MatcherFailT> try_n(const std::function<std::optional<cv::Mat>()> &f,
                                            size_t                                         n,
                                            DWORD sleep = 1 * 1000) {
    for (size_t i = 0; i < n; ++i) {
      auto res = try_once(f);
      if (res.has_value()) {
        return res.value();
      }
      if (res.error() == MatcherFailT::TryNExceed) {
        Sleep(sleep);
        continue;
      }
      return res;
    }
    return std::unexpected(MatcherFailT::TryNExceed);
  }
};

using Matcher = T_Matcher<CV_8UC3, cv::TM_CCOEFF_NORMED>;

#endif
