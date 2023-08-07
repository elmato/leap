#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>

#include "state-dump.hpp"

namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;

int main(int argc, char** argv) {

   fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::all);

   bool help;
   std::string snapshot_file;
   std::string outdir;

   options_description cli ("evm-state-dump command line options");
   cli.add_options()
      ("snapshot-file",bpo::value<std::string>(&snapshot_file)->default_value("snapshot.bin"),"Snapshot file")
      ("outidr",bpo::value<std::string>(&outdir)->default_value("."), "Output folder where generated json files will be stored")
      ("help,h", bpo::bool_switch(&help)->default_value(false), "Print this help message and exit.");

   try {
      variables_map vmap;
      bpo::store(bpo::parse_command_line(argc, argv, cli), vmap);
      bpo::notify(vmap);
      
      if(help) {
         cli.print(std::cerr);
         return 0;
      }
      
      if (vmap.count("snapshot-file") > 0)
         snapshot_file = vmap.at("snapshot-file").as<std::string>();

      if (vmap.count("outdir") > 0)
         outdir = vmap.at("outdir").as<std::string>();

      evm_state_dump::state_dump sd;
      sd.init(snapshot_file, outdir);
      sd.start();
   } catch( const fc::exception& e ) {
      elog( "${e}", ("e", e.to_detail_string()));
      return -1;
   } catch( const boost::exception& e ) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
      return -1;
   } catch( const std::exception& e ) {
      elog("${e}", ("e",e.what()));
      return -1;
   } catch( ... ) {
      elog("unknown exception");
      return -1;
   }

   return 0;
}
