add_executable( evm-state-dump main.cpp state-dump.cpp )

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

target_include_directories(evm-state-dump PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries( evm-state-dump
        PRIVATE appbase
        PRIVATE eosio_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

copy_bin( evm-state-dump )
install( TARGETS
   evm-state-dump
   COMPONENT base
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
