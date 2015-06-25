#ifndef OMNICORE_SP_H
#define OMNICORE_SP_H

#include "omnicore/log.h"
#include "omnicore/omnicore.h"
#include "omnicore/persistence.h"

class CBlockIndex;
class uint256;

#include "serialize.h"

#include <boost/filesystem.hpp>

#include <openssl/sha.h>

#include <stdint.h>
#include <stdio.h>

#include <fstream>
#include <map>
#include <string>
#include <vector>

/** LevelDB based storage for currencies, smart properties and tokens.
 */
class CMPSPInfo : public CDBBase
{
public:
    struct Entry {
        // common SP data
        std::string issuer;
        uint16_t prop_type;
        uint32_t prev_prop_id;
        std::string category;
        std::string subcategory;
        std::string name;
        std::string url;
        std::string data;
        int64_t num_tokens;

        // crowdsale generated SP
        uint32_t property_desired;
        int64_t deadline;
        uint8_t early_bird;
        uint8_t percentage;

        // closedearly states, if the SP was a crowdsale and closed due to MAXTOKENS or CLOSE command
        bool close_early;
        bool max_tokens;
        int64_t missedTokens;
        int64_t timeclosed;
        uint256 txid_close;

        // other information
        uint256 txid;
        uint256 creation_block;
        uint256 update_block;
        bool fixed;
        bool manual;

        // for crowdsale properties, schema is 'txid:amtSent:deadlineUnix:userIssuedTokens:IssuerIssuedTokens;'
        // for manual properties, schema is 'txid:grantAmount:revokeAmount;'
        std::map<std::string, std::vector<int64_t> > historicalData;

        Entry();

        ADD_SERIALIZE_METHODS;

        template <typename Stream, typename Operation>
        inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
            READWRITE(issuer);
            READWRITE(prop_type);
            READWRITE(prev_prop_id);
            READWRITE(category);
            READWRITE(subcategory);
            READWRITE(name);
            READWRITE(url);
            READWRITE(data);
            READWRITE(num_tokens);
            READWRITE(property_desired);
            READWRITE(deadline);
            READWRITE(early_bird);
            READWRITE(percentage);
            READWRITE(close_early);
            READWRITE(max_tokens);
            READWRITE(missedTokens);
            READWRITE(timeclosed);
            READWRITE(txid_close);
            READWRITE(txid);
            READWRITE(creation_block);
            READWRITE(update_block);
            READWRITE(fixed);
            READWRITE(manual);
            READWRITE(historicalData);
        }

        bool isDivisible() const;
        void print() const;
    };

private:
    // implied version of msc and tmsc so they don't hit the leveldb
    Entry implied_msc;
    Entry implied_tmsc;

    uint32_t next_spid;
    uint32_t next_test_spid;

public:
    CMPSPInfo(const boost::filesystem::path& path, bool fWipe);
    virtual ~CMPSPInfo();

    void init(uint32_t nextSPID = 0x3UL, uint32_t nextTestSPID = TEST_ECO_PROPERTY_1);

    uint32_t peekNextSPID(uint8_t ecosystem) const;
    uint32_t updateSP(uint32_t propertyID, const Entry& info);
    uint32_t putSP(uint8_t ecosystem, const Entry& info);
    bool getSP(uint32_t spid, Entry& info) const;
    bool hasSP(uint32_t spid) const;
    uint32_t findSPByTX(const uint256& txid) const;

    int64_t popBlock(const uint256& block_hash);

    static std::string const watermarkKey;
    void setWatermark(const uint256& watermark);
    bool getWatermark(uint256& watermark) const;

    void printAll() const;
};

/** A live crowdsale.
 */
class CMPCrowd
{
private:
    uint32_t propertyId;
    int64_t nValue;

    uint32_t property_desired;
    int64_t deadline;
    uint8_t early_bird;
    uint8_t percentage;

    int64_t u_created;
    int64_t i_created;

    uint256 txid; // NOTE: not persisted as it doesnt seem used

    std::map<std::string, std::vector<int64_t> > txFundraiserData; // schema is 'txid:amtSent:deadlineUnix:userIssuedTokens:IssuerIssuedTokens;'

public:
    CMPCrowd();
    CMPCrowd(uint32_t pid, int64_t nv, uint32_t cd, int64_t dl, uint8_t eb, uint8_t per, int64_t uct, int64_t ict);

    uint32_t getPropertyId() const { return propertyId; }

    int64_t getDeadline() const { return deadline; }
    uint32_t getCurrDes() const { return property_desired; }

    void incTokensUserCreated(int64_t amount) { u_created += amount; }
    void incTokensIssuerCreated(int64_t amount) { i_created += amount; }

    int64_t getUserCreated() const { return u_created; }
    int64_t getIssuerCreated() const { return i_created; }

    void insertDatabase(const std::string& txhash, const std::vector<int64_t>& txdata);
    std::map<std::string, std::vector<int64_t> > getDatabase() const { return txFundraiserData; }

    void print(const std::string& address, FILE* fp = stdout) const;
    void saveCrowdSale(std::ofstream& file, SHA256_CTX* shaCtx, const std::string& addr) const;
};

namespace mastercore
{
typedef std::map<std::string, CMPCrowd> CrowdMap;

extern CMPSPInfo* _my_sps;
extern CrowdMap my_crowds;

const char* c_strPropertyType(uint16_t propertyType);

std::string getPropertyName(uint32_t propertyId);
bool isPropertyDivisible(uint32_t propertyId);

CMPCrowd* getCrowd(const std::string& address);

bool isCrowdsaleActive(uint32_t propertyId);
bool isCrowdsalePurchase(const uint256& txid, const std::string& address, int64_t* propertyId, int64_t* userTokens, int64_t* issuerTokens);

// TODO: check, if this could be combined with the other calculate* functions
int64_t calculateFractional(uint16_t propType, uint8_t bonusPerc, int64_t fundraiserSecs,
        int64_t numProps, uint8_t issuerPerc, const std::map<std::string, std::vector<int64_t> >& txFundraiserData,
        const int64_t amountPremined);

void eraseMaxedCrowdsale(const std::string& address, int64_t blockTime, int block);

unsigned int eraseExpiredCrowdsale(const CBlockIndex* pBlockIndex);

// TODO: depreciate
void dumpCrowdsaleInfo(const std::string& address, const CMPCrowd& crowd, bool bExpired = false);
}


#endif // OMNICORE_SP_H
