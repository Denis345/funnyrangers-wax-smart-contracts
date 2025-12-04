project(funnyrangers)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( funnyrangers funnyrangers funnyrangers.cpp )
target_include_directories( funnyrangers PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( funnyrangers ${CMAKE_SOURCE_DIR}/../ricardian )