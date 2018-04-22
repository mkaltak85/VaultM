#include "Arduino.h"
#include <Adafruit_ST7735.h>

// vault user interface
class Vault { 
private:
    uint8_t page;
    uint8_t account;
    uint8_t nPages;
    uint8_t nAccounts;
    bool pageFold;
public:
    Vault(uint8_t itemsPerPage, uint8_t naccounts); // constructor
    ~Vault();                     // deconstructor
    void setPage( uint8_t newPage  );
    void nextPage( );
    void nextAccount( );
    void setPageFold( bool newPageFold );
    uint8_t getAccount( );
    uint8_t getPage( );
    uint8_t getPages( );
    uint8_t getnAccounts( );
    bool getPageFold( );
}; 
