#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <atomicassets.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

#include <wax-orng-interface.hpp>

using namespace eosio;
using namespace std;

#define ATOMIC name("atomicassets")
#define CONTRACTN name("funnyrangers")
#define COLLECTION name("funnyrangers")

CONTRACT funnyrangers : public contract {
   public:
      using contract::contract;

      TABLE accounts_s
      {
         name account;
         uint64_t energy;
         uint64_t max_energy;
         vector <asset> resources;
         bool licensehiver;
         bool licenseslayer;
         bool licenseberry;
         uint64_t boxes;

         uint64_t primary_key() const { return account.value; };
      };

      typedef multi_index<"accounts"_n, accounts_s> accounts_t;
      accounts_t accounts = accounts_t(get_self(), get_self().value);



      TABLE tools_s
      {
         uint64_t asset_id;
         name owner;
         string template_name;
         string type;
         uint64_t template_id;
         uint64_t next_availability;
         bool broken;


         uint64_t primary_key() const { return asset_id; };
         uint64_t by_owner() const { return owner.value; };
      };

      typedef multi_index<"tools"_n, tools_s,
      indexed_by < name("owner"), const_mem_fun < tools_s, uint64_t, &tools_s::by_owner>>>
      tools_t;
      tools_t tools = tools_t(get_self(), get_self().value);


      TABLE license_s
      {
         uint64_t asset_id;
         name owner;
         string template_name;
         string type;
         uint64_t template_id;
         uint64_t next_availability;

         uint64_t primary_key() const { return asset_id; };
         uint64_t by_owner() const { return owner.value; };
      };

      typedef multi_index<"license"_n, license_s,
      indexed_by < name("owner"), const_mem_fun < license_s, uint64_t, &license_s::by_owner>>>
      license_t;
      license_t license = license_t(get_self(), get_self().value);



      TABLE toolsconf_s
         {
            string template_name;
            name chema_name;
            uint64_t template_id;
            string type;
            uint64_t energy_consumed;
            double crash_chanse;
            vector <asset> change_price;
            vector <asset> repair_price;
            asset reward;
            uint64_t charged_time;

    
            string img;

            uint64_t primary_key() const { return template_id; };
         };

      typedef multi_index<"toolsconf"_n, toolsconf_s> toolsconf_t;
      toolsconf_t toolsconf = toolsconf_t(get_self(), get_self().value);


      TABLE licenseconf_s
         {
            string template_name;
            name chema_name;
            uint64_t template_id;
            string type;
            vector <asset> change_price;
    
            string img;

            uint64_t primary_key() const { return template_id; };
         };

      typedef multi_index<"licenseconf"_n, licenseconf_s> licenseconf_t;
      licenseconf_t licenseconf = licenseconf_t(get_self(), get_self().value);


      TABLE assets_s
         {
            uint64_t asset_id;
            name owner;
            uint64_t template_id;

            uint64_t primary_key() const { return asset_id; };
         };

      typedef multi_index<name("assets"), assets_s> assets_t;
      assets_t assets = assets_t(get_self(), get_self().value);



      TABLE configgame_s
         {
            name config;
            uint64_t num;

            uint64_t primary_key() const { return config.value; };
         };

      typedef multi_index<"configgame"_n, configgame_s> configgame_t;
      configgame_t configgame = configgame_t(get_self(), get_self().value);


      TABLE brokentools_s
      {
         uint64_t asset_id;
         name owner;
         uint64_t primary_key() const { return asset_id; };
      };

      typedef multi_index<"brokentools"_n, brokentools_s> brokentools_t;
      brokentools_t brokentools = brokentools_t(get_self(), get_self().value);



      TABLE claimrewards_s
      {
         
         name owner;
         uint64_t time;
         asset reward;

         uint64_t primary_key() const { return owner.value; };
      };      

      typedef multi_index<"claimrewards"_n, claimrewards_s> claimrewards_t;
      claimrewards_t claimrewards = claimrewards_t(get_self(), get_self().value);


      TABLE resbox_s
      {
         
         name owner;
         uint64_t time;
         uint64_t template_id;
         asset token;

         uint64_t primary_key() const { return owner.value; };
      };      

      typedef multi_index<"resbox"_n, resbox_s> resbox_t;
      resbox_t resbox = resbox_t(get_self(), get_self().value);

      TABLE costenergy_s
      {
         
         uint64_t num;

         uint64_t primary_key() const { return num; };
      };      

      typedef multi_index<"costenergy"_n, costenergy_s> costenergy_t;
      costenergy_t costenergy = costenergy_t(get_self(), get_self().value);



      TABLE partners_s
      {
         
         name account;
         bool active;
         vector <asset> award;

         uint64_t primary_key() const { return account.value; };
      };      

      typedef multi_index<"partners"_n, partners_s> partners_t;
      partners_t partners = partners_t(get_self(), get_self().value);
      

      TABLE referrals_s
      {
         name account;
         name partner;

         uint64_t primary_key() const { return account.value; };
      };      

      typedef multi_index<"referrals"_n, referrals_s> referrals_t;
      referrals_t referrals = referrals_t(get_self(), get_self().value);




      ///////////////// ACTIONS //////////////////

      ACTION settoolsconf(
            string template_name,
            name chema_name,
            uint64_t template_id,
            string type,
            uint64_t energy_consumed,
            double crash_chanse,
            vector <asset> change_price,
            vector <asset> repair_price,
            asset reward,
            uint64_t charged_time,
            string img
      );

      ACTION deltoolsconf(
         int32_t template_id
      );




      ACTION setlicconf(
            string template_name,
            name chema_name,
            string type,
            uint64_t template_id,
            vector <asset> change_price,
            string img
      );

      ACTION dellicconf(
         int32_t template_id
      );


      // handle_asset_transfer stake NFT
      [[eosio::on_notify("atomicassets::logtransfer")]] void handle_asset_transfer(
         name collection_name,
         name from,
         name to,
         vector <uint64_t> asset_ids,
         string memo
      );

      bool checkExistenceTemplateTool(name owner, uint64_t template_id)
      {
         auto groupName = owner;
         auto template_idx = tools.get_index<name("owner")>();
         auto template_itr = template_idx.lower_bound(groupName.value);
         auto template_itr_end = template_idx.upper_bound(groupName.value);

         for ( ; template_itr != template_itr_end; template_itr++ ) {
            if(template_itr->template_id==template_id){
               return false;
            }
         }
         return true;
      }

      bool checkExistenceTemplateLicenses(name owner, uint64_t template_id)
      {
         auto groupName = owner;
         auto template_idx = license.get_index<name("owner")>();
         auto template_itr = template_idx.lower_bound(groupName.value);
         auto template_itr_end = template_idx.upper_bound(groupName.value);

         for ( ; template_itr != template_itr_end; template_itr++ ) {
            if(template_itr->template_id==template_id){
               return false;
            }
         }
         return true;
      }

      // add new user to table
      ACTION newuser(
         name owner,
         name partner
      );


      // unstake nft
      ACTION unstake(
         name asset_owner,
         vector <uint64_t> asset_ids
      );


      [[eosio::on_notify("rangertokens::transfers")]] void depositAll(
            name from,
            name to,
            vector<asset> quantitys,
            string memo
      );

      [[eosio::on_notify("rangertokens::transfer")]] void deposit(
            name from,
            name to,
            asset quantity,
            string memo
      );

      ACTION claim(
         name owner,
         vector <uint64_t> asset_ids,
         uint64_t asset_license
      );

      // set  configgame
      ACTION setconfiggame(
         name config,
         uint64_t num
      );

      // del configgame
      ACTION delconfiggame(
         name config
      );



      // eancrease ballance of token
      void internal_encrease_balance(
         name owner,
         asset quantity
      );

      // decrease ballance of token
      void internal_decrease_balance(
         name owner,
         asset quantity
      );

      // fill energy
      ACTION fillenergy(
         name owner, 
         uint64_t num
      );

      // withdraw tokens from game
      ACTION withdraw(
         name owner,
         vector <asset> tokens_to_withdraw
      );


      // change something
      ACTION tradetools(
         name owner,
         uint64_t template_change
      );



      void mintasset(name owner, name collection, name schema, int32_t template_id)
      {
         vector<uint64_t> returning;
         atomicassets::ATTRIBUTE_MAP nodata = {};
         action(
            permission_level{CONTRACTN, name("active")},
            ATOMIC,
            name("mintasset"),
            make_tuple(CONTRACTN, collection, schema, template_id, owner, nodata, nodata, returning))
            .send();
      };


      // delete user
      ACTION deluser(
         name userandom
      );


      // repair tool
      ACTION repairtool(
         name owner,
         uint64_t asset_id
      );

      ACTION getrandom(
         name nm,
         uint64_t customer_id
      );

      ACTION receiverand(
         uint64_t customer_id,
         const checksum256& random_value
      );

      TABLE rngcustomers_table{
         name nm;
         uint64_t customer_id;
         checksum256 random_value;
         uint64_t finalnumber;
         uint64_t primary_key() const { return customer_id; }
      };

      typedef multi_index<"rngcustomers"_n, rngcustomers_table> rngcustomers_index;
      rngcustomers_index rngcustomers =  rngcustomers_index(get_self(), get_self().value);





      ACTION setchanset(
         name owner,
         uint64_t template_id,
         double crash_chanse
      );



      ACTION openbox(
         name owner
      );

      ACTION addpartner(
         name account,
         bool active
      );


      ACTION addbox(
         name account,
         uint64_t num
      );




      

      




};