#include <iostream>
#include <boost/type_index.hpp>

#include <fc/io/raw.hpp>
#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <fc/crypto/sha256.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/snapshot.hpp>

#include "jsonfile/types.hpp"

#include "state-dump.hpp"
#include "snapshot.hpp"
#include "evm.hpp"

using namespace fc;
using namespace eosio::chain;
using namespace evm_state_dump::snapshot;
using namespace evm_state_dump::evm_state;

namespace evm_state_dump {

struct state_dump_impl {

   template <typename T>
   std::string get_type_name(const T& val) {
      auto name = boost::typeindex::type_id<T>().pretty_name();
      auto pos = name.rfind("::");
      if(pos != std::string::npos) name=name.substr(pos+2);
      return name;
   }

   template <typename T>
   void dump_row(const std::string& section, const T& val) {
      if(current_section_ != section) {
         if(current_section_.size()) { 
            evm_contract_state_outfile_ << std::endl << "]" << "," << std::endl;
         }
         evm_contract_state_outfile_ << "\"" << section << "\":[";
         current_section_ = section;
      } else {
         evm_contract_state_outfile_ << ",";
      }
      evm_contract_state_outfile_ << std::endl << fc::json::to_pretty_string(fc::variant(val), [](size_t s) {});
   }

   template <typename T>
   void dump_row(const T& val, std::optional<uint64_t> extra={}) {
      auto section = get_type_name(val);
      if(extra.has_value()) {
         section += std::to_string(extra.value());
      }

      dump_row(section, val);
   }

   void on(const account& acct, uint64_t scope) {
      account_list.push_back(acct);
      dump_row(acct);
   }

   void on(const account_code& code, uint64_t scope) {
      account_code_list.insert({code.id, code});
      dump_row(code);
   }

   void on(const storage& s, uint64_t scope) {
      storage_list[scope].push_back(s);
      dump_row(s, scope);
   }

   void on(const gcstore& store, uint64_t scope) {
      dump_row(store);
   }

   void on(const balance& bal, uint64_t scope) {
      dump_row(bal);
   }

   void on(const nextnonce& next, uint64_t scope) {
      dump_row(next);
   }

   void on(const allowed_egress_account& allow, uint64_t scope) {
      dump_row(allow);
   }

   void on(const message_receiver& msgrec, uint64_t scope) {
      dump_row(msgrec);
   }

   void on(const config& cfg, uint64_t scope) {
      dump_row(cfg);
   }

   void on(const config2& cfg2, uint64_t scope) {
      dump_row(cfg2);
   }

   void on(const inevm& balance, uint64_t scope) {
      dump_row(balance);
   }

   void process_row(const table_id_object_ex& tid, const key_value_object_ex& val) {

      // EVM state tables
      if(tid.table == "account"_n)
         on(fc::raw::unpack<account>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "accountcode"_n)
         on(fc::raw::unpack<account_code>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "storage"_n)
         on(fc::raw::unpack<storage>(val.value), tid.scope.to_uint64_t());
      // EVM auxiliary tables
      else if(tid.table == "gcstore"_n)
         on(fc::raw::unpack<gcstore>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "balances"_n)
         on(fc::raw::unpack<balance>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "nextnonces"_n)
         on(fc::raw::unpack<nextnonce>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "egresslist"_n)
         on(fc::raw::unpack<allowed_egress_account>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "msgreceiver"_n)
         on(fc::raw::unpack<message_receiver>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "config"_n)
         on(fc::raw::unpack<config>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "config2"_n)
         on(fc::raw::unpack<config2>(val.value), tid.scope.to_uint64_t());
      else if(tid.table == "inevm"_n)
         on(fc::raw::unpack<inevm>(val.value), tid.scope.to_uint64_t());
      else
         wlog("Unknown table ${t}",("t",tid.table));
   }

   void process_row(const table_id_object_ex& tid, const index64_value& val) {
      dump_row("index64", val);
   }

   void process_row(const table_id_object_ex& tid, const index128_value& val) {
      dump_row("index128", val);
   }

   void process_row(const table_id_object_ex& tid, const index256_value& val) {
      dump_row("index256", val);
   }

   void process_row(const table_id_object_ex& tid, const index_double_value& val) {
      dump_row("index_double", val);
   }

   void process_row(const table_id_object_ex& tid, const index_long_double_value& val) {
      dump_row("index_long_double", val);
   }

   void init(const std::string& snapshot_file, const std::string& outdir) {
      snapshot_file_ = snapshot_file;
      outdir_ = outdir;
   }

   void start() {

      auto infile = std::ifstream(snapshot_file_, (std::ios::in | std::ios::binary));
      istream_snapshot_reader reader(infile);
      reader.validate();

      init_evm_contract_state_generation();

      reader.read_section("contract_tables", [&]( auto& section ) {
         fc::sha256::encoder enc;
         bool more = !section.empty();
         while (more) {
            table_id_object_ex tid;
            section.read_row(tid);

            contract_database_index_set_ex::walk_indices([&](auto utils) {
               using utils_t = decltype(utils);

               unsigned_int size;
               more = section.read_row(size);

               for (size_t idx = 0; idx < size.value; idx++) {
                  typename utils_t::index_t::value_type row;
                  more = section.read_row(row);
                  if(tid.code == "eosio.evm"_n) {
                     process_row(tid, row);
                     fc::raw::pack(enc, row);
                  }
               }
            });
         }
         std::cout << "eosio.evm: " << enc.result().str() << std::endl;
      });

      end_evm_contract_state_generation();

      generate_evm_state();
   }

   void init_evm_contract_state_generation() {
      current_section_.clear();
      auto evm_contract_state_filename = outdir_ + "/evm-contrat-state.json";
      dlog("evm_contract_state_filename ${d}", ("d",evm_contract_state_filename));
      if(evm_contract_state_outfile_.is_open()) evm_contract_state_outfile_.close();
      evm_contract_state_outfile_.open(evm_contract_state_filename, (std::ios::out | std::ios::binary));
      evm_contract_state_outfile_ << "{" << std::endl;
   }

   void end_evm_contract_state_generation() {
      if(current_section_.size()) { 
         evm_contract_state_outfile_ << std::endl << "]" << std::endl;
      }
      evm_contract_state_outfile_ << "}" << std::endl;
      evm_contract_state_outfile_.close();
   }

   void generate_evm_state() {
     
      std::list<evm_json_file::account> accounts;
      for(const auto& acct : account_list) {

         FC_ASSERT(acct.eth_address.size() == 20);
         FC_ASSERT(acct.balance.size() == 32);
         
         evm_json_file::account o;
         memcpy(o.address.data(), acct.eth_address.data(), acct.eth_address.size());
         memcpy(o.balance.data(), acct.balance.data(), acct.balance.size());
         
         if(acct.code_id.has_value()) {
            auto it = account_code_list.find(acct.code_id.value());
            FC_ASSERT(it != account_code_list.end());
            FC_ASSERT(it->second.code_hash.size() == 32);
            evm_json_file::b32 code_hash;
            memcpy(code_hash.data(), it->second.code_hash.data(), it->second.code_hash.size());
            o.code_hash = code_hash;
         }

         o.nonce = acct.nonce;
         
         auto sit = storage_list.find(acct.id);
         if(sit != storage_list.end()) {
            for(const auto& _s : sit->second) {
               FC_ASSERT(_s.key.size() == 32);
               FC_ASSERT(_s.value.size() == 32);
               evm_json_file::storage s;
               memcpy(s.key.data(), _s.key.data(), 32);
               memcpy(s.value.data(), _s.value.data(), 32);
               auto it = std::lower_bound(o.slots.begin(), o.slots.end(), s);
               o.slots.insert(it, s);
            }
         }

         auto it = std::lower_bound(accounts.begin(), accounts.end(), o);
         accounts.insert(it, o);
      }

      auto evm_state_file_name=outdir_+"/evm-state.json";
      dlog("evm_state_file_name ${d}", ("d",evm_state_file_name));
      auto outfile = std::ofstream(evm_state_file_name, (std::ios::out | std::ios::binary));
      outfile << accounts << std::endl;
      outfile.close();
   }

   std::vector<account> account_list;
   std::map<uint64_t, account_code> account_code_list;
   std::map<uint64_t, std::vector<storage>> storage_list;
   std::string snapshot_file_;
   std::string outdir_;
   std::string current_section_;
   std::ofstream evm_contract_state_outfile_;
};

state_dump::state_dump() : my(new state_dump_impl()) {}

void state_dump::init(const std::string& snapshot_file, const std::string& outdir) {
   my->init(snapshot_file, outdir);
}

void state_dump::start() {
   my->start();
}

state_dump::~state_dump() {}

} //namespace evm_state_dump


