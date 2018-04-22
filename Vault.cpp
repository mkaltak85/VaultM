#include "Vault.h"
#include <Adafruit_ST7735.h>

Vault::Vault( uint8_t itemsPerPage, uint8_t naccounts  ) {
   nAccounts = naccounts ; 
   // set number of pages necessary
   nPages =  naccounts/itemsPerPage ; 
   // set page to 1 
   page = 0 ; 
   // selected account = 0 
   account = 0 ; 
   pageFold = false ;
}

Vault::~Vault( ) {
    delete &page;
    delete &nPages;
    delete &nAccounts; 
    delete &account; 
}

void Vault::setPage( uint8_t newPage ){
   page = newPage;
}

void Vault::nextPage( ) {
   if ( page+1 == nPages ) { 
     page = 0 ; 
   } else  { 
     page += 1; 
   }
}

void Vault::nextAccount( ) {
   uint8_t i;
   // store page number
   i = getPage();
   if ( account+1 >= nAccounts ) { 
     account = 0 ; 
   } else  { 
     account += 1 ; 
   }
   // set page number 
   setPage( account/8 ); 

   if ( i != getPage() ) { 
      setPageFold( true );
   } else {
      setPageFold( false );
   }
}
void Vault::setPageFold( bool newPageFold ){
   pageFold = newPageFold;
}

uint8_t Vault::getPage( ){
   return page;
}

uint8_t Vault::getPages( ){
   return nPages;
}

uint8_t Vault::getAccount( ){
   return account;
}

uint8_t Vault::getnAccounts( ){
   return nAccounts;
}

bool Vault::getPageFold( ){
   return pageFold;
}
