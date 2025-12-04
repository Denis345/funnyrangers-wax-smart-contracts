project(funnyrantoke)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( funnyrantoke funnyrantoke funnyrantoke.cpp )
target_include_directories( funnyrantoke PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( funnyrantoke ${CMAKE_SOURCE_DIR}/../ricardian )