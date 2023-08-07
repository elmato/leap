#pragma once

#include <fc/reflect/reflect.hpp>
#include <eosio/chain/types.hpp>

using namespace fc;
using namespace eosio::chain;

namespace evm_state_dump { namespace snapshot {

template <typename T>
struct index_ex {
   using value_type = T;
};

template <typename T>
struct index_value {
   uint64_t      primary_key;
   account_name  payer;
   T             secondary_key;
};

struct key_value_object_ex {
   uint64_t              primary_key;
   account_name          payer;
   bytes                 value;
};
using key_value_index_ex = index_ex<key_value_object_ex>;

using index64_value = index_value<uint64_t>;
using index64_index_ex = index_ex<index64_value>;

using index128_value = index_value<uint128_t>;
using index128_index_ex = index_ex<index128_value>;

using index256_value = index_value<std::array<uint128_t, 2>>;
using index256_index_ex = index_ex<index256_value>;

using index_double_value = index_value<float64_t>;
using index_double_index_ex = index_ex<index_double_value>;

using index_long_double_value = index_value<float128_t>;
using index_long_double_index_ex = index_ex<index_long_double_value>;

using contract_database_index_set_ex = index_set<
   key_value_index_ex,
   index64_index_ex,
   index128_index_ex,
   index256_index_ex,
   index_double_index_ex,
   index_long_double_index_ex
>;

struct table_id_object_ex {
   account_name   code;
   scope_name     scope;
   table_name     table;
   account_name   payer;
   uint32_t       count;
};

} //namespace snapshot
} //namespace evm_state_dump

FC_REFLECT(evm_state_dump::snapshot::key_value_object_ex, (primary_key)(payer)(value));
FC_REFLECT(evm_state_dump::snapshot::index64_value, (primary_key)(payer)(secondary_key));
FC_REFLECT(evm_state_dump::snapshot::index128_value, (primary_key)(payer)(secondary_key));
FC_REFLECT(evm_state_dump::snapshot::index256_value, (primary_key)(payer)(secondary_key));
FC_REFLECT(evm_state_dump::snapshot::index_double_value, (primary_key)(payer)(secondary_key));
FC_REFLECT(evm_state_dump::snapshot::index_long_double_value, (primary_key)(payer)(secondary_key));
FC_REFLECT(evm_state_dump::snapshot::table_id_object_ex, (code)(scope)(table)(payer)(count));
