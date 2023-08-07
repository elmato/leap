#pragma once

#include <string>
#include <memory>

namespace evm_state_dump {

struct state_dump {
    explicit state_dump();
    ~state_dump();

    void init(const std::string& snapshot_file, const std::string& outdir);
    void start();
    
    std::unique_ptr<struct state_dump_impl> my;
};

} //namespace evm_state_dump