/*
 * Copyright 2017 <Gustavo Pantuza>
 *
 * ============================================================================
 *
 *       Filename:  gate.h
 *
 *    Description:  Header file of the Vetulus Workers Gate
 *
 *        Version:  1.0
 *        Created:  11/09/2017 14:58:40
 *       Compiler:  g++
 *
 *         Author:  Gustavo Pantuza (gustavopantuza@gmail.com)
 *   Organization:  Computer Science community
 *
 * ============================================================================
 */

#ifndef WORKERS_GATE_H_
#define WORKERS_GATE_H_


#include "./spdlog/spdlog.h"

#include "./config.h"
#include "./pool.h"


namespace spd = spdlog;


class Gate {
 public:
    explicit Gate(WorkersConfigLoader config);
    void listen();
    void shutdown();

    ThreadPool* pool;

 private:
    // Variables read from configuration file
    int threads;
    std::shared_ptr<spd::logger> console;
};

#endif  // WORKERS_GATE_H_
