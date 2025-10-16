#ifndef MATCHER_HPP_
#define MATCHER_HPP_

#include <QThread>
#include <QObject>
#include <QString>

#include <atomic>

#include "prog.hpp"
#include "utils.hpp"

enum class MatcherGotType
{
    Coin,
    St_nd,
    Result,
};

class MatcherWorker : public QObject
{
    Q_OBJECT

    std::atomic_bool external_input_flag;
    std::atomic_bool stop_requested;
    size_t external_input;

public:
    MatcherWorker();

public slots:
    // the work func, normally block the worker thread in a while(true) loop, so use queuedconnection
    ErrorType main_matcher();
    // these two funcs set the inside var of Matcher class to communicate with it, use directconnection
    void set_extern_input(size_t input);
    void request_stop();

signals:
    void got_match_step(MatcherGotType got, size_t n);
    void exited(ErrorType err);
};

#endif