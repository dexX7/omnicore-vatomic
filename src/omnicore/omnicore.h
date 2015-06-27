#ifndef OMNICORE_OMNICORE_H
#define OMNICORE_OMNICORE_H

class CBitcoinAddress;
class CBlockIndex;
class CTransaction;

#include "omnicore/log.h"
#include "omnicore/persistence.h"
#include "omnicore/tally.h"

#include "sync.h"
#include "uint256.h"
#include "util.h"

#include <boost/filesystem/path.hpp>

#include "leveldb/status.h"

#include "json/json_spirit_value.h"

#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <set>

using json_spirit::Array;

using std::string;

int const MAX_STATE_HISTORY = 50;

#define TEST_ECO_PROPERTY_1 (0x80000003UL)

// could probably also use: int64_t maxInt64 = std::numeric_limits<int64_t>::max();
// maximum numeric values from the spec:
#define MAX_INT_8_BYTES (9223372036854775807UL)

// what should've been in the Exodus address for this block if none were spent
#define DEV_MSC_BLOCK_290629 (1743358325718)

#define SP_STRING_FIELD_LEN 256

// Omni Layer Transaction Class
#define OMNI_CLASS_A 1
#define OMNI_CLASS_B 2
#define OMNI_CLASS_C 3

// Maximum number of keys supported in Class B
#define CLASS_B_MAX_SENDABLE_PACKETS 2

// Master Protocol Transaction (Packet) Version
#define MP_TX_PKT_V0  0
#define MP_TX_PKT_V1  1

// Maximum outputs per BTC Transaction
#define MAX_BTC_OUTPUTS 16

#define MIN_PAYLOAD_SIZE     5
#define PACKET_SIZE_CLASS_A 19
#define PACKET_SIZE         31
#define MAX_PACKETS         64

// Transaction types, from the spec
enum TransactionType {
  MSC_TYPE_SIMPLE_SEND              =  0,
  MSC_TYPE_RESTRICTED_SEND          =  2,
  MSC_TYPE_SEND_TO_OWNERS           =  3,
  MSC_TYPE_SAVINGS_MARK             = 10,
  MSC_TYPE_SAVINGS_COMPROMISED      = 11,
  MSC_TYPE_RATELIMITED_MARK         = 12,
  MSC_TYPE_AUTOMATIC_DISPENSARY     = 15,
  MSC_TYPE_TRADE_OFFER              = 20,
  MSC_TYPE_ACCEPT_OFFER_BTC         = 22,
  MSC_TYPE_METADEX_TRADE            = 25,
  MSC_TYPE_METADEX_CANCEL_PRICE     = 26,
  MSC_TYPE_METADEX_CANCEL_PAIR      = 27,
  MSC_TYPE_METADEX_CANCEL_ECOSYSTEM = 28,
  MSC_TYPE_NOTIFICATION             = 31,
  MSC_TYPE_OFFER_ACCEPT_A_BET       = 40,
  MSC_TYPE_CREATE_PROPERTY_FIXED    = 50,
  MSC_TYPE_CREATE_PROPERTY_VARIABLE = 51,
  MSC_TYPE_PROMOTE_PROPERTY         = 52,
  MSC_TYPE_CLOSE_CROWDSALE          = 53,
  MSC_TYPE_CREATE_PROPERTY_MANUAL   = 54,
  MSC_TYPE_GRANT_PROPERTY_TOKENS    = 55,
  MSC_TYPE_REVOKE_PROPERTY_TOKENS   = 56,
  MSC_TYPE_CHANGE_ISSUER_ADDRESS    = 70,
  OMNICORE_MESSAGE_TYPE_ALERT     = 65535
};

#define MSC_PROPERTY_TYPE_INDIVISIBLE             1
#define MSC_PROPERTY_TYPE_DIVISIBLE               2
#define MSC_PROPERTY_TYPE_INDIVISIBLE_REPLACING   65
#define MSC_PROPERTY_TYPE_DIVISIBLE_REPLACING     66
#define MSC_PROPERTY_TYPE_INDIVISIBLE_APPENDING   129
#define MSC_PROPERTY_TYPE_DIVISIBLE_APPENDING     130

// block height (MainNet) with which the corresponding transaction is considered valid, per spec
enum BLOCKHEIGHTRESTRICTIONS {
// starting block for parsing on TestNet
  START_TESTNET_BLOCK=263000,
  START_REGTEST_BLOCK=5,
  MONEYMAN_REGTEST_BLOCK= 101, // new address to assign MSC & TMSC on RegTest
  MONEYMAN_TESTNET_BLOCK= 270775, // new address to assign MSC & TMSC on TestNet
  POST_EXODUS_BLOCK = 255366,
  MSC_DEX_BLOCK     = 290630,
  MSC_SP_BLOCK      = 297110,
  GENESIS_BLOCK     = 249498,
  LAST_EXODUS_BLOCK = 255365,
  MSC_STO_BLOCK     = 342650,
  MSC_METADEX_BLOCK = 999999,
  MSC_BET_BLOCK     = 999999,
  MSC_MANUALSP_BLOCK= 323230,
  P2SH_BLOCK        = 322000,
  OP_RETURN_BLOCK   = 999999
};

enum FILETYPES {
  FILETYPE_BALANCES = 0,
  FILETYPE_OFFERS,
  FILETYPE_ACCEPTS,
  FILETYPE_GLOBALS,
  FILETYPE_CROWDSALES,
  FILETYPE_MDEXORDERS,
  NUM_FILETYPES
};

#define PKT_RETURNED_OBJECT    (1000)

#define PKT_ERROR             ( -9000)
#define DEX_ERROR_SELLOFFER   (-10000)
#define DEX_ERROR_ACCEPT      (-20000)
#define DEX_ERROR_PAYMENT     (-30000)
// Smart Properties
#define PKT_ERROR_SP          (-40000)
// Send To Owners
#define PKT_ERROR_STO         (-50000)
#define PKT_ERROR_SEND        (-60000)
#define PKT_ERROR_TRADEOFFER  (-70000)
#define PKT_ERROR_METADEX     (-80000)
#define METADEX_ERROR         (-81000)
#define PKT_ERROR_TOKENS      (-82000)

#define OMNI_PROPERTY_BTC   0
#define OMNI_PROPERTY_MSC   1
#define OMNI_PROPERTY_TMSC  2

// forward declarations
std::string FormatDivisibleMP(int64_t n, bool fSign = false);
std::string FormatDivisibleShortMP(int64_t);
std::string FormatMP(uint32_t, int64_t n, bool fSign = false);
bool feeCheck(const std::string& address, size_t nDataSize);

/** Returns the Exodus address. */
const CBitcoinAddress ExodusAddress();

//! Used to indicate, whether to automatically commit created transactions
extern bool autoCommit;

//! Global lock for state objects
extern CCriticalSection cs_tally;

/** LevelDB based storage for STO recipients.
 */
class CMPSTOList : public CDBBase
{
public:
    CMPSTOList(const boost::filesystem::path& path, bool fWipe)
    {
        leveldb::Status status = Open(path, fWipe);
        PrintToConsole("Loading send-to-owners database: %s\n", status.ToString());
    }

    virtual ~CMPSTOList()
    {
        if (msc_debug_persistence) PrintToLog("CMPSTOList closed\n");
    }

    void getRecipients(const uint256 txid, string filterAddress, Array *recipientArray, uint64_t *total, uint64_t *stoFee);
    std::string getMySTOReceipts(string filterAddress);
    int deleteAboveBlock(int blockNum);
    void printStats();
    void printAll();
    bool exists(string address);
    void recordSTOReceive(std::string, const uint256&, int, unsigned int, uint64_t);
};

/** LevelDB based storage for the trade history. Trades are listed with key "txid1+txid2".
 */
class CMPTradeList : public CDBBase
{
public:
    CMPTradeList(const boost::filesystem::path& path, bool fWipe)
    {
        leveldb::Status status = Open(path, fWipe);
        PrintToConsole("Loading trades database: %s\n", status.ToString());
    }

    virtual ~CMPTradeList()
    {
        if (msc_debug_persistence) PrintToLog("CMPTradeList closed\n");
    }

    void recordTrade(const uint256 txid1, const uint256 txid2, string address1, string address2, unsigned int prop1, unsigned int prop2, uint64_t amount1, uint64_t amount2, int blockNum);
    int deleteAboveBlock(int blockNum);
    bool exists(const uint256 &txid);
    void printStats();
    void printAll();
    bool getMatchingTrades(const uint256 txid, unsigned int propertyId, Array *tradeArray, int64_t *totalSold, int64_t *totalBought);
    int getMPTradeCountTotal();
};

/** LevelDB based storage for transactions, with txid as key and validity bit, and other data as value.
 */
class CMPTxList : public CDBBase
{
public:
    CMPTxList(const boost::filesystem::path& path, bool fWipe)
    {
        leveldb::Status status = Open(path, fWipe);
        PrintToConsole("Loading transactions database: %s\n", status.ToString());
    }

    virtual ~CMPTxList()
    {
        if (msc_debug_persistence) PrintToLog("CMPTxList closed\n");
    }

    void recordTX(const uint256 &txid, bool fValid, int nBlock, unsigned int type, uint64_t nValue);
    void recordPaymentTX(const uint256 &txid, bool fValid, int nBlock, unsigned int vout, unsigned int propertyId, uint64_t nValue, string buyer, string seller);
    void recordMetaDExCancelTX(const uint256 &txidMaster, const uint256 &txidSub, bool fValid, int nBlock, unsigned int propertyId, uint64_t nValue);

    string getKeyValue(string key);
    uint256 findMetaDExCancel(const uint256 txid);
    int getNumberOfPurchases(const uint256 txid);
    int getNumberOfMetaDExCancels(const uint256 txid);
    bool getPurchaseDetails(const uint256 txid, int purchaseNumber, string *buyer, string *seller, uint64_t *vout, uint64_t *propertyId, uint64_t *nValue);
    int getMPTransactionCountTotal();
    int getMPTransactionCountBlock(int block);

    bool exists(const uint256 &txid);
    bool getTX(const uint256 &txid, string &value);

    int setLastAlert(int blockHeight);

    void printStats();
    void printAll();

    bool isMPinBlockRange(int, int, bool);
};

//! Available balances of wallet properties
extern std::map<uint32_t, int64_t> global_balance_money;
//! Reserved balances of wallet propertiess
extern std::map<uint32_t, int64_t> global_balance_reserved;
//! Vector containing a list of properties relative to the wallet
extern std::set<uint32_t> global_wallet_property_list;

int64_t getMPbalance(const std::string& address, uint32_t propertyId, TallyType ttype);
int64_t getUserAvailableMPbalance(const std::string& address, uint32_t propertyId);
int IsMyAddress(const std::string& address);

std::string getLabel(const std::string& address);

/** Global handler to initialize Omni Core. */
int mastercore_init();

/** Global handler to shut down Omni Core. */
int mastercore_shutdown();

/** Global handler to total wallet balances. */
void CheckWalletUpdate(bool forceUpdate = false);

int mastercore_handler_disc_begin(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_disc_end(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_block_begin(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_block_end(int nBlockNow, CBlockIndex const * pBlockIndex, unsigned int);
int mastercore_handler_tx(const CTransaction &tx, int nBlock, unsigned int idx, CBlockIndex const *pBlockIndex );
int mastercore_save_state( CBlockIndex const *pBlockIndex );

namespace mastercore
{
extern std::map<std::string, CMPTally> mp_tally_map;
extern CMPTxList *p_txlistdb;
extern CMPTradeList *t_tradelistdb;
extern CMPSTOList *s_stolistdb;

std::string strMPProperty(uint32_t propertyId);

int GetHeight();
uint32_t GetLatestBlockTime();
CBlockIndex* GetBlockIndex(const uint256& hash);

bool isMPinBlockRange(int starting_block, int ending_block, bool bDeleteFound);

std::string FormatIndivisibleMP(int64_t n);

int ClassAgnosticWalletTXBuilder(const std::string& senderAddress, const std::string& receiverAddress, const std::string& redemptionAddress,
                 int64_t referenceAmount, const std::vector<unsigned char>& data, uint256& txid, std::string& rawHex, bool commit);

bool isTestEcosystemProperty(uint32_t propertyId);
bool isMainEcosystemProperty(uint32_t propertyId);
uint32_t GetNextPropertyId(bool maineco); // maybe move into sp

CMPTally* getTally(const std::string& address);

int64_t getTotalTokens(uint32_t propertyId, int64_t* n_owners_total = NULL);

char *c_strMasterProtocolTXType(int i);

bool isTransactionTypeAllowed(int txBlock, unsigned int txProperty, unsigned int txType, unsigned short version, bool bAllowNullProperty = false);

bool getValidMPTX(const uint256 &txid, int *block = NULL, unsigned int *type = NULL, uint64_t *nAmended = NULL);

bool update_tally_map(const std::string& who, uint32_t propertyId, int64_t amount, TallyType ttype);

std::string getTokenLabel(uint32_t propertyId);
}


#endif // OMNICORE_OMNICORE_H
