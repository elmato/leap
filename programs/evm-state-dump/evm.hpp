#pragma once
#include <fc/io/raw.hpp>
#include <fc/reflect/reflect.hpp>
#include <eosio/chain/asset.hpp>

using namespace eosio::chain;

namespace evm_state_dump { namespace evm_state {

struct account {
    uint64_t  id;
    bytes     eth_address;
    uint64_t  nonce;
    bytes     balance;
    std::optional<uint64_t> code_id;
};

struct account_code {
    uint64_t    id;
    uint32_t    ref_count;
    bytes       code;
    bytes       code_hash;
};

struct storage {
    uint64_t id;
    bytes    key;
    bytes    value;
};

struct gcstore {
    uint64_t id;
    uint64_t storage_id;
};

struct balance_with_dust {
    asset    balance;
    uint64_t dust;
};

struct balance {
    name              owner;
    balance_with_dust balance;
};

struct nextnonce {
    name     owner;
    uint64_t next_nonce;
};

struct allowed_egress_account {
    name account;
};

struct message_receiver {
    name  account;
    asset min_fee;
};

struct config
{
   unsigned_int   version;
   uint64_t       chainid;
   time_point_sec genesis_time;
   asset          ingress_bridge_fee;
   uint64_t       gas_price;
   uint32_t       miner_cut;
   uint32_t       status;
};

struct config2
{
   uint64_t next_account_id = 0;
};

struct inevm
{
   balance_with_dust balance;
};


} //namespace evm_state
} //namespace evm_state_dump

FC_REFLECT(evm_state_dump::evm_state::account, (id)(eth_address)(nonce)(balance)(code_id));
FC_REFLECT(evm_state_dump::evm_state::account_code, (id)(ref_count)(code)(code_hash));
FC_REFLECT(evm_state_dump::evm_state::storage, (id)(key)(value));
FC_REFLECT(evm_state_dump::evm_state::gcstore, (id)(storage_id));
FC_REFLECT(evm_state_dump::evm_state::balance_with_dust, (balance)(dust));
FC_REFLECT(evm_state_dump::evm_state::balance, (owner)(balance));
FC_REFLECT(evm_state_dump::evm_state::nextnonce, (owner)(next_nonce));
FC_REFLECT(evm_state_dump::evm_state::allowed_egress_account, (account));
FC_REFLECT(evm_state_dump::evm_state::message_receiver, (account)(min_fee));
FC_REFLECT(evm_state_dump::evm_state::config, (version)(chainid)(genesis_time)(ingress_bridge_fee)(gas_price)(miner_cut)(status));
FC_REFLECT(evm_state_dump::evm_state::config2, (next_account_id));
FC_REFLECT(evm_state_dump::evm_state::inevm, (balance));
