#include <funnyrangers.hpp>
#include <eosio/system.hpp>


// set up tools config
ACTION funnyrangers::settoolsconf(
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
)
{
   require_auth( get_self());
   auto itr = toolsconf.find(template_id);
   check(itr == toolsconf.end(), "Template already exist");

   toolsconf.emplace(_self, [&](auto &rec){
      rec.template_name = template_name;
      rec.chema_name = chema_name;
      rec.template_id = template_id;
      rec.type = type;
      rec.energy_consumed = energy_consumed;
      rec.crash_chanse = crash_chanse;
      rec.change_price = change_price;
      rec.repair_price = repair_price;
      rec.reward = reward;
      rec.charged_time = charged_time;
      rec.img = img;
   });

}

// clean tools config
ACTION funnyrangers::deltoolsconf(
      int32_t template_id
)
{
   auto itr = toolsconf.find(template_id);
   check(itr != toolsconf.end(), "Template not exist");

   toolsconf.erase(itr);
}



// set up license config
ACTION funnyrangers::setlicconf(
      string template_name,
      name chema_name,
      string type,
      uint64_t template_id,
      vector <asset> change_price,
      string img
)
{
   require_auth( get_self());
   auto itr = licenseconf.find(template_id);
   check(itr == licenseconf.end(), "Template already exist");

   licenseconf.emplace(_self, [&](auto &rec){
      rec.template_name = template_name;
      rec.chema_name = chema_name;
      rec.template_id = template_id;
      rec.type = type;
      rec.change_price = change_price;
      rec.img = img;
   });

}

// clean license config
ACTION funnyrangers::dellicconf(
      int32_t template_id
)
{
   auto itr = licenseconf.find(template_id);
   check(itr != licenseconf.end(), "Template not exist");

   licenseconf.erase(itr);
}







// transfer nft to the game
void funnyrangers::handle_asset_transfer(
   name collection_name,
   name from,
   name to,
   vector <uint64_t> asset_ids,
   string memo
) {

   
   // Make sure to ignore everything except for assets transferred to your contract
   if ( to != get_self() ) { return; }

   // check collection 
   atomicassets::assets_t _assets = atomicassets::assets_t(ATOMIC, get_self().value);

   for (int i=0; i<asset_ids.size(); i++) {
      auto itrAsset = _assets.find(asset_ids[i]);
      check(itrAsset->collection_name==COLLECTION, "wrong collection");
   }


   auto itr_acc = accounts.require_find(from.value, "The user is not registered in the game");

   for (int i=0; i<asset_ids.size(); i++) {
      auto itrAsset = _assets.find(asset_ids[i]);
      assets.emplace(_self, [&](auto &rec){ 
            rec.asset_id = asset_ids[i];
            rec.owner = from;
            rec.template_id = itrAsset->template_id;
      });

      if(toolsconf.find(itrAsset->template_id)!= toolsconf.end()){
            check(checkExistenceTemplateTool(from, itrAsset->template_id), "It is not possible to transfer a Tool that you already have");
            auto toolAsset = toolsconf.find(itrAsset->template_id);
            tools.emplace(_self, [&](auto &rec)
            {
               rec.asset_id = asset_ids[i];
               rec.owner = from;
               rec.template_name = toolAsset->template_name;
               rec.type = toolAsset->type;
               rec.template_id = itrAsset->template_id;
               rec.next_availability = 0;
            });

        }
        else if(licenseconf.find(itrAsset->template_id)!= licenseconf.end()){
            check(checkExistenceTemplateLicenses(from, itrAsset->template_id), "It is not possible to transfer a License that you already have");
            auto licenseAsset = licenseconf.find(itrAsset->template_id);
            license.emplace(_self, [&](auto &rec)
            {
               rec.asset_id = asset_ids[i];
               rec.owner = from;
               rec.template_name = licenseAsset->template_name;
               rec.type = licenseAsset->type;
               rec.template_id = licenseAsset->template_id;
               rec.next_availability = 0;
            });
        }

   }


}


// add new user
ACTION funnyrangers::newuser(
      name owner,
      name partner
)
{
   require_auth(owner);
   auto itrTool = accounts.find((owner.value));
   check(itrTool == accounts.end(), "Account already exists");

   accounts.emplace(_self, [&](auto &rec){
            rec.account = owner;
            rec.energy = 20;
            rec.max_energy = 40;
            rec.resources = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))}; 
            rec.licensehiver = false;
            rec.licenseslayer = false;
            rec.licenseberry = false;
            rec.boxes = 0;
   });

   auto itr_partner = partners.find(name(partner).value);

   if(itr_partner!=partners.end()){
         referrals.emplace(_self, [&](auto &rec){
            rec.account = owner;
            rec.partner = partner;
   });
   }




}


// ustake nft
ACTION funnyrangers::unstake(
      name asset_owner,
      vector <uint64_t> asset_ids
)
{
    uint64_t i;
    require_auth(asset_owner);
    auto itr_acc = accounts.find(asset_owner.value);
   check(itr_acc->energy>=20, "You need to have 20 energy to be able to withdraw!");

    for (int i=0; i<asset_ids.size(); i++) {
      auto itrTool = assets.find(asset_ids[i]);
      check(itrTool != assets.end(), "Asset does not exist");
      check(itrTool->owner == asset_owner , "Can not remove");

      if(tools.find(asset_ids[i])!=tools.end()) check(tools.find(asset_ids[i])->broken==false , "Fix the tool first!");
      
    }




   for (int i=0; i<asset_ids.size(); i++) {
      auto itrTool = assets.find(asset_ids[i]);
      assets.erase(itrTool);

      auto itrTarget2 = tools.find(asset_ids[i]);
      if (itrTarget2 != tools.end()) {
         check(itrTarget2->next_availability <=current_time_point().sec_since_epoch(), "Can not remove, try again later"); 
         tools.erase(itrTarget2);
      }

      auto itrTarget3 = license.find(asset_ids[i]);
      if (itrTarget3 != license.end()){
          check(itrTarget3->next_availability <=current_time_point().sec_since_epoch(), "Can not remove, try again later"); 
          license.erase(itrTarget3);
      } 

      
   }


    name author = get_self();
     action ({
      permission_level{author,"active"_n},
      name("atomicassets"),
      name("transfer"),

      make_tuple(
                    author,
                    asset_owner,
                    asset_ids,
                    string("unstake")
                )
   }).send();
 
}




// deposit all tokens
void funnyrangers::depositAll(
   name from,
   name to,
   vector<asset> quantitys,
   string memo
)
{
   if ( to != get_self() ) { return; }
   if ( memo != "deposit" ) { return; }


   auto balance_itr = accounts.require_find(from.value,
   "The specified account does not have a balance table row");


   for (int i=0; i<quantitys.size(); i++) {

      vector <asset> quantities = balance_itr->resources;
       if(quantities.size()==0){
         quantities = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))};
       }
      bool found_token = false;

      for (auto itr = quantities.begin(); itr != quantities.end(); itr++) {
         if (itr->symbol == quantitys[i].symbol) {
               found_token = true;
               itr->amount += quantitys[i].amount;
               break;
         }
      }

      check(found_token,
         "The specified account does not have a balance for the symbol specified in the quantity");

      //Updating the balances table
      
      accounts.modify(balance_itr, same_payer, [&](auto &_balance) {
         _balance.resources = quantities;
      });
   }
}


// depos one token
void funnyrangers::deposit(
   name from,
   name to,
   asset quantity,
   string memo
)
{
   if ( to != get_self() ) { return; }
   if ( memo != "deposit" ) { return; }


   auto balance_itr = accounts.require_find(from.value,
   "The specified account does not have a balance table row");

     
   
      vector <asset> quantities = balance_itr->resources;
       if(quantities.size()==0){
         quantities = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))};
       }
      bool found_token = false;

      for (auto itr = quantities.begin(); itr != quantities.end(); itr++) {
         if (itr->symbol == quantity.symbol) {
               found_token = true;
               itr->amount += quantity.amount;
               break;
         }
      }

      check(found_token,
         "The specified account does not have a balance for the symbol specified in the quantity");

      //Updating the balances table
      
      accounts.modify(balance_itr, same_payer, [&](auto &_balance) {
         _balance.resources = quantities;
      });
}



   // set up game config
ACTION funnyrangers::setconfiggame(
       name config,
       uint64_t num
)
{
   require_auth( get_self());
   auto itr = configgame.find(config.value);

   if(itr == configgame.end()){
      configgame.emplace(_self, [&](auto &rec){
      rec.config = config;
      rec.num = num;
      });
   }
   else{
      configgame.modify(itr, _self, [&](auto &rec) {
      rec.num=num;
   });
   }   

}


// clean game config  
ACTION funnyrangers::delconfiggame(
       name config
)
{
   require_auth( get_self());
   auto itr = configgame.find(config.value);
   check(itr != configgame.end(), "Config not exist");
   configgame.erase(itr);
}

// Eancrease the balance of a specified account by a specified quantity
void funnyrangers::internal_encrease_balance(
    name owner,
    asset quantity
) {

    auto balance_itr = accounts.require_find(owner.value,
        "The specified account does not have a balance table row");

    vector <asset> quantities = balance_itr->resources;
    bool found_token = false;

    for (auto itr = quantities.begin(); itr != quantities.end(); itr++) {
        if (itr->symbol == quantity.symbol) {
            found_token = true;
             check(itr->amount + quantity.amount >=0,
                "The specified account's balance is lower than the specified quantity");
            itr->amount += quantity.amount;
            break;
        }
    }

    check(found_token,
        "The specified account does not have a balance for the symbol specified in the quantity");

    //Updating the balances table
    if (quantities.size() > 0) {
        accounts.modify(balance_itr, same_payer, [&](auto &_balance) {
            _balance.resources = quantities;
        });
    } 
}



// Decreases the balance of a specified account by a specified quantity
void funnyrangers::internal_decrease_balance(
    name owner,
    asset quantity
) 
{
    auto balance_itr = accounts.require_find(owner.value,
        "The specified account does not have a balance table row");

    vector <asset> quantities = balance_itr->resources;
    bool found_token = false;
    for (auto itr = quantities.begin(); itr != quantities.end(); itr++) {
        if (itr->symbol == quantity.symbol) {
            found_token = true;
             check(itr->amount >= quantity.amount,
                "The specified account's balance is lower than the specified quantity");
            itr->amount -= quantity.amount;
            break;
        }
    }
    check(found_token,
        "The specified account does not have a balance for the symbol specified in the quantity");

    //Updating the balances table
    if (quantities.size() > 0) {
        accounts.modify(balance_itr, same_payer, [&](auto &_balance) {
            _balance.resources = quantities;
        });
    } 
}




ACTION funnyrangers::claim(
   name owner,
   vector <uint64_t> asset_ids,
   uint64_t asset_license
)
{
   require_auth(owner);
   auto itr_acc = accounts.require_find(owner.value, "User not found");
   check(asset_ids.size()>=1 && asset_ids.size()<=3, "The number of instruments must be 1<=n<=3");

   string typeTools = tools.find(asset_ids[0])->type;
    for (int i=0; i<asset_ids.size(); i++) {
      check(tools.find(asset_ids[i])->type==typeTools,"All tools must be of the same type");
      auto itrTool = assets.find(asset_ids[i]);
      check(tools.find(asset_ids[i])->next_availability<=current_time_point().sec_since_epoch(), "The tool is busy");
      check(itrTool != assets.end(), "Asset does not exist");
      check(itrTool->owner == owner , "Wrong owner");
      check(tools.find(asset_ids[i])->broken==false , "Fix the tool first!");
    }

   if(asset_ids.size()==3){
      check(asset_ids[0]!=asset_ids[1] && asset_ids[1]!=asset_ids[2] && asset_ids[0]!=asset_ids[2], "You can't take the same instrument more than once!");
    }
    else if(asset_ids.size()==2){
      check(asset_ids[0]!=asset_ids[1] && asset_ids[1]!=asset_ids[2], "You can't take the same instrument more than once!");
   }

   auto itrLic = license.find(asset_license);
   check(itrLic != license.end(), "Asset does not exist");
   check(itrLic->type == typeTools, "License type mismatch");
   check(itrLic->next_availability<=current_time_point().sec_since_epoch(), "The license is busy");




   uint64_t energy = toolsconf.find(assets.find(asset_ids[0])->template_id)->energy_consumed;

   for (int i=1; i<asset_ids.size(); i++) {
      energy+=toolsconf.find(assets.find(asset_ids[i])->template_id)->energy_consumed;
   }
   check(itr_acc->energy>=energy, "Not enought energy!");

   accounts.modify(itr_acc, _self, [&](auto &rec) {
         rec.energy -= energy;
   });

   // вычисление вероятностей

   vector <uint64_t> resTools;

    for (int i=0; i<asset_ids.size(); i++) {
      double chanse = toolsconf.find(assets.find(asset_ids[i])->template_id)->crash_chanse;

      action(
         permission_level{get_self(), name("active")},
         name("funnyrangers"),
         name("getrandom"),
         make_tuple(
            owner,
            asset_ids[i]
         )
       ).send();


      uint64_t num;
      if(rngcustomers.find(asset_ids[i])==rngcustomers.end()){
         num=100;
      }
      else num =rngcustomers.find(asset_ids[i])->finalnumber;


      if (num>chanse)
      
      {
         resTools.push_back(asset_ids[i]);
      }
      else {

               tools.modify(tools.find(asset_ids[i]), _self, [&](auto &rec) {
                     rec.broken=true;
               });

               brokentools.emplace(_self, [&](auto &rec){
                  rec.owner = owner;
                  rec.asset_id = asset_ids[i];
               });
      }
    }


    if(resTools.size()==0){
      auto itr_rewards = claimrewards.find(owner.value);
      if (itr_rewards!= claimrewards.end()){
         claimrewards.modify(itr_rewards, _self, [&](auto &rec) {
         rec.time = current_time_point().sec_since_epoch();
         rec.reward = asset(0.0000, symbol(symbol_code("RHN"), 4));; 
         });
      }
      else{
         claimrewards.emplace(_self, [&](auto& rec) {
         rec.owner = owner;
         rec.time = current_time_point().sec_since_epoch();
         rec.reward = asset(0.0000, symbol(symbol_code("RHN"), 4));; 
         });
      }
      return;
    }

   asset profit = toolsconf.find(assets.find(resTools[0])->template_id)->reward;

   for (int i=1; i<resTools.size(); i++) {
      profit.amount+=toolsconf.find(assets.find(resTools[i])->template_id)->reward.amount;
   }

   // double mediana;
   uint64_t lvl = 1;
   action({
   permission_level{get_self(), name("active")},
   name("funnyrangers"),
   name("getrandom"),
   make_tuple(
      owner,
      lvl
   )
   }).send();
   float n = 50;
   // mediana = ((rngcustomers.find(lvl)->finalnumber)-50)/100;
   profit.amount+=((rngcustomers.find(lvl)->finalnumber-n)/100)  *10000;

   

   if(typeTools == "honey"){
      profit.amount*=configgame.find(name("honkof").value)->num;
   }
   else if(typeTools =="mushroom"){
      profit.amount*=configgame.find(name("mashkof").value)->num;
   }
   else if(typeTools =="berry"){
      profit.amount*=configgame.find(name("berconf").value)->num;
   };

   auto itr_rewards = claimrewards.find(owner.value);

   if (itr_rewards!= claimrewards.end()){
         claimrewards.modify(itr_rewards, _self, [&](auto &rec) {
            rec.time = current_time_point().sec_since_epoch();
            rec.reward = profit; 
         });
   }
   else{
         claimrewards.emplace(_self, [&](auto& rec) {
            rec.owner = owner;
            rec.time = current_time_point().sec_since_epoch();
            rec.reward = profit; 
         });
   }




   internal_encrease_balance(owner, profit);
   



   for (int i=0; i<resTools.size(); i++) {
      tools.modify(tools.find(resTools[i]), _self, [&](auto &rec) {
         // rec.next_availability = current_time_point().sec_since_epoch()+toolsconf.find(assets.find(resTools[i])->template_id)->charged_time;;
         rec.next_availability = current_time_point().sec_since_epoch()+100;
      });
   }

   license.modify(license.find(asset_license), _self, [&](auto &rec) {
      // rec.next_availability = current_time_point().sec_since_epoch()+toolsconf.find(assets.find(resTools[i])->template_id)->charged_time;
         rec.next_availability = current_time_point().sec_since_epoch()+100;
   });

}



ACTION funnyrangers::fillenergy(
   name owner, 
   uint64_t num
)
{
   require_auth(owner);
   auto itr_acc = accounts.find(owner.value);

   check(itr_acc->energy+num<=itr_acc->max_energy,"Too much energy");
   asset dailEnergyCost;

   if(configgame.find(name("energyres").value)->num==1){
      dailEnergyCost=asset(0.0000, symbol(symbol_code("RHN"), 4));
      dailEnergyCost.amount+=((configgame.find(name("energyhon").value)->num))*num*100;
   }
   else if(configgame.find(name("energyres").value)->num==2)
   {
      dailEnergyCost=asset(0.0000, symbol(symbol_code("RMS"), 4));
      dailEnergyCost.amount+=(configgame.find(name("energymsh").value)->num)*num*100;
   }
   else if(configgame.find(name("energyres").value)->num==3)
   {
      dailEnergyCost=asset(0.0000, symbol(symbol_code("RBR"), 4));
      dailEnergyCost.amount+=(configgame.find(name("energyber").value)->num)*num*100;
   }

   // dailEnergyCost.amount*= num*10000;

   internal_decrease_balance(owner, dailEnergyCost);

   accounts.modify(itr_acc, _self, [&](auto &rec) {
         rec.energy += num;
   });

}


// withdraw tokens
ACTION funnyrangers::withdraw(
   name owner,
   vector <asset> tokens_to_withdraw
)
{
   auto itr_acc = accounts.require_find(owner.value, "User not found");
   check(tokens_to_withdraw.size()<=3, "Withdraw error!");
   check(itr_acc->energy>=20, "You need to have 20 energy to be able to withdraw!");
   
   uint64_t fee = configgame.find(name("fee").value)->num;

   bool isreferral = false;

   if(partners.find(name(owner).value)!=partners.end()){
      if(partners.find(name(owner).value)->active) fee=0;
   }
   else{
      if(referrals.find(name(owner).value)!=referrals.end()){
        
         if(partners.find((referrals.find(name(owner).value)->partner).value)->active){
             fee=fee-2;


            name partner_name = referrals.find(name(owner).value)->partner;
            auto itrpartner = partners.find(partner_name.value);
            
            if(itrpartner!=partners.end()){
               vector <asset> wtoken_partner = tokens_to_withdraw;
               auto quantities = partners.find(name(partner_name).value)->award;
               auto spread = accounts.find(partner_name.value)->resources;

               for (auto itr = wtoken_partner.begin(); itr != wtoken_partner.end(); itr++) {
                  itr->amount = itr->amount/100;
                  // internal_encrease_balance(partner_name, itr);

                  for (auto itr2 = quantities.begin(); itr2 != quantities.end(); itr2++) {
                     if (itr2->symbol == itr->symbol) {
                        itr2->amount += itr->amount;
                        break;
                     }
                  }

                  for (auto itr3 = spread.begin(); itr3 != spread.end(); itr3++) {
                     if (itr3->symbol == itr->symbol) {
                        itr3->amount += itr->amount;
                        break;
                     }
                  }
               }  

               partners.modify(partners.find(name(partner_name).value), same_payer, [&](auto &_balance) {
                  _balance.award = quantities;
               });

               accounts.modify(accounts.find(partner_name.value), same_payer, [&](auto &_balance) {
                  _balance.resources = spread;
                });
            }







         }
      }
   }





   vector <asset> wtoken_komission = tokens_to_withdraw;
   for (auto itr = wtoken_komission.begin(); itr != wtoken_komission.end(); itr++) {
      check(itr->amount > 0, "token_to_withdraw must be positive");
            itr->amount -= itr->amount*fee/100;

   }


   for (int i=0; i<wtoken_komission.size(); i++) {
       internal_decrease_balance(owner, tokens_to_withdraw[i]);

      if (wtoken_komission[i].symbol==symbol(symbol_code("RHN"), 4) || wtoken_komission[i].symbol==symbol(symbol_code("RMS"), 4) || wtoken_komission[i].symbol==symbol(symbol_code("RBR"), 4)  ) {
         action(
               permission_level{get_self(), name("active")},
               name("rangertokens"),
               name("transfer"),
               make_tuple(
                  get_self(),
                  owner,
                  wtoken_komission[i],
                  string("Withdrawal")
               )
         ).send();
      }

   }
    
}


// change something
ACTION funnyrangers::tradetools(
   name owner,
   uint64_t template_change
)
{
   require_auth(owner);
   auto itr_acc = accounts.find(owner.value);

   if(toolsconf.find(template_change)!= toolsconf.end()){
      auto itrTool = toolsconf.find(template_change);
      internal_decrease_balance(owner, toolsconf.find(template_change)->change_price[configgame.find(name("marketres").value)->num-1] );
      mintasset(owner, COLLECTION, itrTool->chema_name, itrTool->template_id);
   }
   else if(licenseconf.find(template_change)!= licenseconf.end()){
      auto itrTool = licenseconf.find(template_change);
      internal_decrease_balance(owner, licenseconf.find(template_change)->change_price[configgame.find(name("marketres").value)->num-1] );
      mintasset(owner, COLLECTION, itrTool->chema_name, itrTool->template_id);
   }
   else {
      check(false, "Wrong template!");
   }

}



// delete user
ACTION funnyrangers::deluser(
      name user
)
{
   require_auth(CONTRACTN);
   vector <uint64_t> killasset;

   for (auto iter1 = tools.begin(); iter1 != tools.end(); iter1++)
   {
      if (iter1->owner ==name(user) )
      {
         killasset.push_back(iter1->asset_id);
      }
   }

   for (int i = 0; i < killasset.size(); i++)
   {
      tools.erase(tools.find(killasset[i]));
      assets.erase(assets.find(killasset[i]));
   }





   vector <uint64_t> killasset2;
   for (auto iter2 = license.begin(); iter2 != license.end(); iter2++)
   {
      if (iter2->owner ==name(user) )
      {
         killasset2.push_back(iter2->asset_id);
      }
   }

   for (int i = 0; i < killasset2.size(); i++)
   {
      license.erase(license.find(killasset2[i]));
      assets.erase(assets.find(killasset2[i]));
   }




   vector <uint64_t> killasset4;
   for (auto iter3 = brokentools.begin(); iter3 != brokentools.end(); iter3++)
   {
      if (iter3->owner ==name(user) )
      {
         killasset4.push_back(iter3->asset_id);
      }
   }

   for (int i = 0; i < killasset4.size(); i++)
   {
      brokentools.erase(brokentools.find(killasset4[i]));
   }



   accounts.erase(accounts.find(name(user).value));

}



// repair tool
ACTION funnyrangers::repairtool(
   name owner,
   uint64_t asset_id
)
{
   require_auth(owner);
   auto itr_acc = accounts.require_find(owner.value, "User not Found");
   auto itr_tool = brokentools.require_find(asset_id, "Tool not Found");


   auto itr_asset = brokentools.find(asset_id);

   if(itr_asset!= brokentools.end()){
      check(itr_asset->owner == owner, "Wrong asset!");
      auto template_repair = assets.find(asset_id)->template_id;
      internal_decrease_balance(owner, toolsconf.find(template_repair)->repair_price[configgame.find(name("repairres").value)->num-1] );
      tools.modify(tools.find(asset_id), _self, [&](auto &rec) {
            rec.broken=false;
      });
      brokentools.erase(brokentools.find(asset_id));
   }

   tools.modify(tools.find(asset_id), _self, [&](auto &rec) {
         rec.broken=false;
   });


}


ACTION funnyrangers::getrandom(name nm, uint64_t customer_id) {

   // check if this customer_id exists in the table
   auto itrCustomer = rngcustomers.find(customer_id);
   // if not, insert a new record
   if (itrCustomer == rngcustomers.end()) {
      rngcustomers.emplace(_self, [&](auto& rec) {
            rec.customer_id = customer_id;
            rec.nm = nm;
      });
   }

    size_t size = transaction_size();
    char buf[size];
    uint32_t read = read_transaction(buf, size);
    check(size == read, "Signing value generation: read_transaction() has failed.");
    checksum256 tx_id = eosio::sha256(buf, read);
    uint64_t signing_value;
    memcpy(&signing_value, tx_id.data(), sizeof(signing_value));

   while (orng::signvals.find(signing_value) != orng::signvals.end()) {
        signing_value++;
    }



   //call orng.wax
   action(
      { get_self(), "active"_n },
      "orng.wax"_n,
      "requestrand"_n,
      std::tuple{ customer_id, signing_value, get_self() })
      .send();
}


ACTION funnyrangers::receiverand(uint64_t customer_id, const checksum256& random_value) {

   //cast the random_value to a smaller number
   uint64_t max_value = 99;
   auto byte_array = random_value.extract_as_byte_array();

   uint64_t random_int = 0;
   for (int i = 0; i < 8; i++) {
      random_int <<= 8;
      random_int |= (uint64_t)byte_array[i];
   }

   uint64_t num1 = random_int % max_value;

   //find the customer record by customer_id
   auto itrCustomer = rngcustomers.find(customer_id);
   //make sure the record exists
   check(itrCustomer != rngcustomers.end(), "customer table not set");
   //update the random numbers by customer_id
   rngcustomers.modify(itrCustomer, _self, [&](auto& rec) {
      rec.random_value = random_value;
      rec.finalnumber = num1;
   });

}


ACTION funnyrangers::setchanset(
   name owner,
   uint64_t template_id,
   double crash_chanse
)
{

   // require_auth(get_self());


   // toolsconf.modify(toolsconf.find(template_id), _self, [&](auto &rec) {
   //          rec.crash_chanse=crash_chanse;
   // });
   uint64_t num = 1;

   // rngcustomers.erase(num);

   //    rngcustomers.modify(rngcustomers.find(num), _self, [&](auto &rec) {
   //       rec.nm = name("funnyrangers");
   // });

   //  accounts.modify(accounts.find(name("funnyrang222").value), _self, [&](auto &rec) {
   //       rec.boxes = 10;
   //       // rec.resources = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))};
   // });

   //     accounts.modify(accounts.find(name("qwertyuio111").value), _self, [&](auto &rec) {
   //       rec.boxes = 10;
   //       // rec.resources = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))};
   // });

   //     accounts.modify(accounts.find(name("qwertyuio111").value), _self, [&](auto &rec) {
   //       rec.resources = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))};
   // });

   // action({
   //    permission_level{get_self(), name("active")},
   //    name("funnyrangers"),
   //    name("getrandom"),
   //    make_tuple(
   //       owner,
   //       num
   //    )
   // }).send();


   //      action ({
   //    permission_level{author,"active"_n},
   //    name("atomicassets"),
   //    name("transfer"),

   //    make_tuple(
   //                  author,
   //                  asset_owner,
   //                  asset_ids,
   //                  string("unstake")
   //              )
   // }).send();

      partners.erase(partners.find(name("funnyrang222").value));
      referrals.erase(referrals.find(name("newtesttestt").value));
      referrals.erase(referrals.find(name("rangertokens").value));

}



ACTION funnyrangers::openbox(
   name owner
)
{
   require_auth(owner);
   auto itr_acc = accounts.require_find(owner.value, "User not Found");
   check(accounts.find(owner.value)->boxes>0, "You don't have boxes!");
   check(configgame.find(name("possibletoope").value)->num==1, "It is not possible to open boxes at the moment");


   vector <uint64_t> chanses= {518947,518947,518947,518946,518946,518946,518945,518945,518945,518950,518950,518949,518949,518948,518948,518953,518952,518951,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518863,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518862,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861,518861};
                             
   uint64_t lvl = 0;
   action({
   permission_level{get_self(), name("active")},
   name("funnyrangers"),
   name("getrandom"),
   make_tuple(
      owner,
      lvl
   )
   }).send();

   uint64_t res_asset = 0;
   asset res_token;

   uint64_t num = rngcustomers.find(0)->finalnumber;
   if(num<=62){
      res_asset = chanses[num];
      if(num<=17){
         auto itrTool = toolsconf.find(chanses[num]);
         mintasset(owner, COLLECTION, itrTool->chema_name, itrTool->template_id);
      }
      else{
         auto itrTool = licenseconf.find(chanses[num]);
         mintasset(owner, COLLECTION, itrTool->chema_name, itrTool->template_id);
      }

   }
   else{
      if(num>=63 && num<=82){
         res_token = asset(100000.0000, symbol(symbol_code("RBR"), 4));
         internal_encrease_balance(owner, asset(100000.0000, symbol(symbol_code("RBR"), 4)));
      }

      if(num>=83 && num<=94){
         res_token = asset(100000.0000, symbol(symbol_code("RMS"), 4));
         internal_encrease_balance(owner, asset(100000.0000, symbol(symbol_code("RMS"), 4)));
      }

      if(num>=95 && num<=99){
         res_token = asset(100000.0000, symbol(symbol_code("RHN"), 4));
         internal_encrease_balance(owner, asset(100000.0000, symbol(symbol_code("RHN"), 4)));
      }
      

   }

   accounts.modify(itr_acc, _self, [&](auto &rec) {
         rec.boxes -= 1;
   });



   auto itr_rewards = resbox.find(owner.value);

   if (itr_rewards!= resbox.end()){
         resbox.modify(itr_rewards, _self, [&](auto &rec) {
            rec.time = current_time_point().sec_since_epoch();
            rec.token = res_token;
            rec.template_id = res_asset;

         });
   }
   else{
         resbox.emplace(_self, [&](auto& rec) {
            rec.owner = owner;
            rec.time = current_time_point().sec_since_epoch();
            rec.token = res_token;
            rec.template_id = res_asset;
         });
   }



}


ACTION funnyrangers::addpartner(
   name account,
   bool active
)
{
   require_auth(get_self());

   auto itr_partner = partners.find(account.value);

   if (itr_partner!= partners.end()){
         partners.modify(itr_partner, _self, [&](auto &rec) {
            rec.active = active;
         });
   }
   else{
         partners.emplace(_self, [&](auto& rec) {
            rec.account = account;
            rec.active = active;
            rec.award = {asset(0.0000, symbol(symbol_code("RHN"), 4)), asset(0.0000, symbol(symbol_code("RMS"), 4)), asset(0.0000, symbol(symbol_code("RBR"), 4))}; 
         });
   }

}




ACTION funnyrangers::addbox(
   name account,
   uint64_t num
)
{
   require_auth(get_self());
   auto itr_acc = accounts.require_find(account.value, "User not Found");


   accounts.modify(itr_acc, _self, [&](auto &rec) {
         rec.boxes += num;
   });

}
