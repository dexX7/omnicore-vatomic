#ifndef MASTERCORE_MDEX_H
#define MASTERCORE_MDEX_H

#include "mastercore_log.h"

#include "chain.h"
#include "main.h"
#include "uint256.h"

#include <boost/multiprecision/cpp_dec_float.hpp>

#include <openssl/sha.h>

#include <stdint.h>

#include <fstream>
#include <map>
#include <set>
#include <string>

typedef boost::multiprecision::cpp_dec_float_100 XDOUBLE;

#define DISPLAY_PRECISION_LEN  50
#define INTERNAL_PRECISION_LEN 50

/** A trade on the distributed exchange.
 */
class CMPMetaDEx
{
private:
    int block;
    uint256 txid;
    unsigned int idx; // index within block
    uint32_t property;
    int64_t amount_forsale;
    uint32_t desired_property;
    int64_t amount_desired;
    int64_t amount_remaining;
    unsigned char subaction;
    std::string addr;

public:
    uint256 getHash() const { return txid; }
    void setHash(const uint256& hash) { txid = hash; }

    unsigned int getProperty() const { return property; }
    unsigned int getDesProperty() const { return desired_property; }

    int64_t getAmountForSale() const { return amount_forsale; }
    int64_t getAmountDesired() const { return amount_desired; }
    int64_t getAmountRemaining() const { return amount_remaining; }

    void setAmountForSale(int64_t ao, const std::string& label = "")
    {
        amount_forsale = ao;
        file_log("%s(%ld %s):%s\n", __FUNCTION__, ao, label, ToString());
    }

    void setAmountDesired(int64_t ad, const std::string& label = "")
    {
        amount_desired = ad;
        file_log("%s(%ld %s):%s\n", __FUNCTION__, ad, label, ToString());
    }

    void setAmountRemaining(int64_t ar, const std::string& label = "")
    {
        amount_remaining = ar;
        file_log("%s(%ld %s):%s\n", __FUNCTION__, ar, label, ToString());
    }

    unsigned char getAction() const { return subaction; }

    const std::string& getAddr() const { return addr; }

    int getBlock() const { return block; }
    unsigned int getIdx() const { return idx; }

    uint64_t getBlockTime() const
    {
        CBlockIndex* pblockindex = chainActive[block];
        return pblockindex->GetBlockTime();
    }

    // needed only by the RPC functions
    // needed only by the RPC functions
    CMPMetaDEx()
      : block(0), txid(0), idx(0), property(0), amount_forsale(0), desired_property(0), amount_desired(0),
        amount_remaining(0), subaction(0) {}

    CMPMetaDEx(const std::string& addr, int b, uint32_t c, int64_t nValue, uint32_t cd, int64_t ad,
               const uint256& tx, uint32_t i, unsigned char suba, int64_t ar = 0)
      : block(b), txid(tx), idx(i), property(c), amount_forsale(nValue), desired_property(cd), amount_desired(ad),
        amount_remaining(ar), subaction(suba), addr(addr) {}

    void Set(const std::string&, int, uint32_t, int64_t, uint32_t, int64_t, const uint256&, uint32_t, unsigned char);

    std::string ToString() const;

    XDOUBLE effectivePrice() const;
    XDOUBLE inversePrice() const;

    void saveOffer(std::ofstream& file, SHA256_CTX* shaCtx) const;
};

namespace mastercore
{
struct MetaDEx_compare
{
    bool operator()(const CMPMetaDEx& lhs, const CMPMetaDEx& rhs) const;
};

// ---------------
//! Set of objects sorted by block+idx
typedef std::set<CMPMetaDEx, MetaDEx_compare> md_Set; 
//! Map of prices; there is a set of sorted objects for each price
typedef std::map<XDOUBLE, md_Set> md_PricesMap;
//! Map of properties; there is a map of prices for each property
typedef std::map<uint32_t, md_PricesMap> md_PropertiesMap;

extern md_PropertiesMap metadex;

// TODO: explore a property-pair, instead of a single property as map's key........
md_PricesMap* get_Prices(uint32_t prop);
md_Set* get_Indexes(md_PricesMap* p, XDOUBLE price);
// ---------------

int MetaDEx_ADD(const std::string& sender_addr, uint32_t, int64_t, int block, uint32_t property_desired, int64_t amount_desired, const uint256& txid, unsigned int idx);
int MetaDEx_CANCEL_AT_PRICE(const uint256&, uint32_t, const std::string&, uint32_t, int64_t, uint32_t, int64_t);
int MetaDEx_CANCEL_ALL_FOR_PAIR(const uint256&, uint32_t, const std::string&, uint32_t, uint32_t);
int MetaDEx_CANCEL_EVERYTHING(const uint256& txid, uint32_t block, const std::string& sender_addr, unsigned char ecosystem);

void MetaDEx_debug_print(bool bShowPriceLevel = false, bool bDisplay = false);
}

#endif // MASTERCORE_MDEX_H
