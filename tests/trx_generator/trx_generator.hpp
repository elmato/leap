#pragma once
#include <trx_provider.hpp>
#include <string>
#include <vector>
#include <appbase/plugin.hpp>
#include <boost/program_options.hpp>
#include <eosio/chain/transaction.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <fc/io/json.hpp>

namespace eosio::testing {

   struct signed_transaction_w_signer {
      signed_transaction_w_signer(eosio::chain::signed_transaction trx, fc::crypto::private_key key) : _trx(move(trx)), _signer(key) {}

      eosio::chain::signed_transaction _trx;
      fc::crypto::private_key _signer;
   };

   struct action_pair_w_keys {
      action_pair_w_keys(eosio::chain::action first_action, eosio::chain::action second_action, fc::crypto::private_key first_act_signer, fc::crypto::private_key second_act_signer)
            : _first_act(first_action), _second_act(second_action), _first_act_priv_key(first_act_signer), _second_act_priv_key(second_act_signer) {}

      eosio::chain::action _first_act;
      eosio::chain::action _second_act;
      fc::crypto::private_key _first_act_priv_key;
      fc::crypto::private_key _second_act_priv_key;
   };

   struct account_name_generator {
      account_name_generator() : _name_index_vec(ACCT_NAME_LEN, 0) {}

      const char* CHARMAP = "12345abcdefghijklmnopqrstuvwxyz";
      const int ACCT_NAME_CHAR_CNT = 31;
      const int ACCT_NAME_LEN = 12;
      const int MAX_PREFEX = 960;
      std::vector<int> _name_index_vec;

      void increment(int index) {
         _name_index_vec[index]++;
         if(_name_index_vec[index] >= ACCT_NAME_CHAR_CNT) {
            _name_index_vec[index] = 0;
            increment(index - 1);
         }
      }

      void increment() {
         increment(_name_index_vec.size() - 1);
      }

      void incrementPrefix() {
         increment(1);
      }

      void setPrefix(int id) {
         if (id > MAX_PREFEX) {
            elog("Account Name Generator Prefix above allowable ${max}", ("max", MAX_PREFEX));
            return;
         }
         _name_index_vec[0] = 0;
         _name_index_vec[1] = 0;
         for(int i = 0; i < id; i++) {
            incrementPrefix();
         }
      };

      std::string calcName() {
         std::string name;
         name.reserve(12);
         for(auto i: _name_index_vec) {
            name += CHARMAP[i];
         }
         return name;
      }
   };

   struct trx_generator_base {
      p2p_trx_provider _provider;
      uint16_t _id;
      eosio::chain::chain_id_type _chain_id;
      eosio::chain::name _contract_owner_account;
      fc::microseconds _trx_expiration;
      eosio::chain::block_id_type _last_irr_block_id;
      std::string _log_dir;

      uint64_t _total_us = 0;
      uint64_t _txcount = 0;

      std::vector<signed_transaction_w_signer> _trxs;

      uint64_t _nonce = 0;
      uint64_t _nonce_prefix = 0;
      bool _stop_on_trx_failed = true;


      trx_generator_base(uint16_t id, std::string chain_id_in, std::string contract_owner_account, fc::microseconds trx_expr, std::string lib_id_str, std::string log_dir, bool stop_on_trx_failed,
         const std::string& peer_endpoint="127.0.0.1", unsigned short port=9876);

      virtual void update_resign_transaction(eosio::chain::signed_transaction& trx, fc::crypto::private_key priv_key, uint64_t& nonce_prefix, uint64_t& nonce,
                                     const fc::microseconds& trx_expiration, const eosio::chain::chain_id_type& chain_id, const eosio::chain::block_id_type& last_irr_block_id);

      void push_transaction(p2p_trx_provider& provider, signed_transaction_w_signer& trx, uint64_t& nonce_prefix,
                            uint64_t& nonce, const fc::microseconds& trx_expiration, const eosio::chain::chain_id_type& chain_id,
                            const eosio::chain::block_id_type& last_irr_block_id);

      void set_transaction_headers(eosio::chain::transaction& trx, const eosio::chain::block_id_type& last_irr_block_id, const fc::microseconds expiration, uint32_t delay_sec = 0);

      signed_transaction_w_signer create_trx_w_actions_and_signer(std::vector<eosio::chain::action> act, const fc::crypto::private_key& priv_key, uint64_t& nonce_prefix, uint64_t& nonce,
                                                                 const fc::microseconds& trx_expiration, const eosio::chain::chain_id_type& chain_id, const eosio::chain::block_id_type& last_irr_block_id);

      void log_first_trx(const std::string& log_dir, const eosio::chain::signed_transaction& trx);

      bool generate_and_send();
      bool tear_down();
      void stop_generation();
      bool stop_on_trx_fail();
   };

   struct transfer_trx_generator : public trx_generator_base {
      const std::vector<std::string> _accts;
      std::vector<std::string> _private_keys_str_vector;

      transfer_trx_generator(uint16_t id, std::string chain_id_in, std::string contract_owner_account, const std::vector<std::string>& accts,
         fc::microseconds trx_expr, const std::vector<std::string>& private_keys_str_vector, std::string lib_id_str, std::string log_dir, bool stop_on_trx_failed,
         const std::string& peer_endpoint="127.0.0.1", unsigned short port=9876);

      std::vector<eosio::chain::name> get_accounts(const std::vector<std::string>& account_str_vector);
      std::vector<fc::crypto::private_key> get_private_keys(const std::vector<std::string>& priv_key_str_vector);

      std::vector<signed_transaction_w_signer> create_initial_transfer_transactions(const std::vector<action_pair_w_keys>& action_pairs_vector, uint64_t& nonce_prefix, uint64_t& nonce, const fc::microseconds& trx_expiration, const eosio::chain::chain_id_type& chain_id, const eosio::chain::block_id_type& last_irr_block_id);
      eosio::chain::bytes make_transfer_data(const eosio::chain::name& from, const eosio::chain::name& to, const eosio::chain::asset& quantity, const std::string&& memo);
      auto make_transfer_action(eosio::chain::name account, eosio::chain::name from, eosio::chain::name to, eosio::chain::asset quantity, std::string memo);
      std::vector<action_pair_w_keys> create_initial_transfer_actions(const std::string& salt, const uint64_t& period, const eosio::chain::name& contract_owner_account,
                                                                 const std::vector<eosio::chain::name>& accounts, const std::vector<fc::crypto::private_key>& priv_keys);

      bool setup();
   };

   struct trx_generator : public trx_generator_base{
      std::string _abi_data_file_path;
      std::string _actions_data_json_file_or_str;
      std::string _actions_auths_json_file_or_str;
      account_name_generator _acct_name_generator;

      eosio::chain::abi_serializer _abi;
      std::vector<fc::mutable_variant_object> _unpacked_actions;
      std::map<int, std::vector<std::string>> _acct_gen_fields;
      std::vector<eosio::chain::action> _actions;

      const fc::microseconds abi_serializer_max_time = fc::seconds(10); // No risk to client side serialization taking a long time

      trx_generator(uint16_t id, std::string chain_id_in, const std::string& abi_data_file, std::string contract_owner_account,
         const std::string& actions_data_json_file_or_str, const std::string& actions_auths_json_file_or_str,
         fc::microseconds trx_expr, std::string lib_id_str, std::string log_dir, bool stop_on_trx_failed,
         const std::string& peer_endpoint="127.0.0.1", unsigned short port=9876);

      void locate_key_words_in_action_mvo(std::vector<std::string>& acct_gen_fields_out, fc::mutable_variant_object& action_mvo, const std::string& key_word);
      void locate_key_words_in_action_array(std::map<int, std::vector<std::string>>& acct_gen_fields_out, fc::variants& action_array, const std::string& key_word);
      void update_key_word_fields_in_sub_action(std::string key, fc::mutable_variant_object& action_mvo, std::string action_inner_key, const std::string key_word);
      void update_key_word_fields_in_action(std::vector<std::string>& acct_gen_fields, fc::mutable_variant_object& action_mvo, const std::string key_word);

      void update_actions();
      virtual void update_resign_transaction(eosio::chain::signed_transaction& trx, fc::crypto::private_key priv_key, uint64_t& nonce_prefix, uint64_t& nonce,
                                     const fc::microseconds& trx_expiration, const eosio::chain::chain_id_type& chain_id, const eosio::chain::block_id_type& last_irr_block_id);

      fc::variant json_from_file_or_string(const std::string& file_or_str, fc::json::parse_type ptype = fc::json::parse_type::legacy_parser);

      bool setup();
   };
}
