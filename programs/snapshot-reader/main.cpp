#include <iostream>
#include <eosio/chain/snapshot.hpp>
#include <fc/crypto/sha256.hpp>

using namespace fc;
using namespace eosio::chain;

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
FC_REFLECT(key_value_object_ex, (primary_key)(payer)(value));
using key_value_index_ex = index_ex<key_value_object_ex>;

using index64_value = index_value<uint64_t>;
FC_REFLECT(index64_value, (primary_key)(payer)(secondary_key));
using index64_index_ex = index_ex<index64_value>;

using index128_value = index_value<uint128_t>;
FC_REFLECT(index128_value, (primary_key)(payer)(secondary_key));
using index128_index_ex = index_ex<index128_value>;

using index256_value = index_value<std::array<uint128_t, 2>>;
FC_REFLECT(index256_value, (primary_key)(payer)(secondary_key));
using index256_index_ex = index_ex<index256_value>;

using index_double_value = index_value<float64_t>;
FC_REFLECT(index_double_value, (primary_key)(payer)(secondary_key));
using index_double_index_ex = index_ex<index_double_value>;

using index_long_double_value = index_value<float128_t>;
FC_REFLECT(index_long_double_value, (primary_key)(payer)(secondary_key));
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
FC_REFLECT(table_id_object_ex, (code)(scope)(table)(payer)(count));


int main(int argc, char** argv) {

   auto infile = std::ifstream(argv[1], (std::ios::in | std::ios::binary));
   istream_snapshot_reader reader(infile);
   reader.validate();

   try {
   reader.read_section("contract_tables", [&]( auto& section ) {
      sha256::encoder enc;
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
               if(tid.code == "eosio.evm"_n)
                  fc::raw::pack(enc, row);
            }
         });
      }
      std::cout << "eosio.evm: " << enc.result().str() << std::endl;
   });

   } FC_LOG_AND_DROP(("Error"));

   return 0;
}
