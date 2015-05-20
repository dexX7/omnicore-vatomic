#include "omnicore/createpayload.h"

#include "utilstrencodings.h"

#include <stdint.h>
#include <vector>
#include <string>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(omnicore_create_payload_tests)

BOOST_AUTO_TEST_CASE(payload_simple_send)
{
    // Simple send [type 0, version 0]
    std::vector<unsigned char> vch = CreatePayload_SimpleSend(
        static_cast<uint32_t>(1),          // property: MSC
        static_cast<int64_t>(100000000));  // amount to transfer: 1.0 MSC (in willets)

    BOOST_CHECK_EQUAL(HexStr(vch), "00000000000000010000000005f5e100");
}

BOOST_AUTO_TEST_CASE(payload_send_to_owners)
{
    // Send to owners [type 3, version 0]
    std::vector<unsigned char> vch = CreatePayload_SendToOwners(
        static_cast<uint32_t>(1),          // property: MSC
        static_cast<int64_t>(100000000));  // amount to transfer: 1.0 MSC (in willets)

    BOOST_CHECK_EQUAL(HexStr(vch), "00000003000000010000000005f5e100");
}

BOOST_AUTO_TEST_CASE(payload_dex_offer)
{
    // Sell tokens for bitcoins [type 20, version 1]
    std::vector<unsigned char> vch = CreatePayload_DExSell(
        static_cast<uint32_t>(1),         // property: MSC
        static_cast<int64_t>(100000000),  // amount to transfer: 1.0 MSC (in willets)
        static_cast<int64_t>(20000000),   // amount desired: 0.2 BTC (in satoshis)
        static_cast<uint8_t>(10),         // payment window in blocks
        static_cast<int64_t>(10000),      // commitment fee in satoshis
        static_cast<uint8_t>(1));         // sub-action: new offer

    BOOST_CHECK_EQUAL(HexStr(vch),
        "00010014000000010000000005f5e1000000000001312d000a000000000000271001");
}

BOOST_AUTO_TEST_CASE(payload_meta_dex_offer)
{
    // Trade tokens for tokens [type 21, version 0]
    std::vector<unsigned char> vch = CreatePayload_MetaDExTrade(
        static_cast<uint32_t>(1),          // property: MSC
        static_cast<int64_t>(250000000),   // amount for sale: 2.5 MSC
        static_cast<uint32_t>(31),         // property desired: TetherUS
        static_cast<int64_t>(5000000000),  // amount desired: 50.0 TetherUS
        static_cast<uint8_t>(1));          // sub-action: new offer

    BOOST_CHECK_EQUAL(HexStr(vch),
        "0000001500000001000000000ee6b2800000001f000000012a05f20001");
}

BOOST_AUTO_TEST_CASE(payload_accept_dex_offer)
{
    // Purchase tokens with bitcoins [type 22, version 0]
    std::vector<unsigned char> vch = CreatePayload_DExAccept(
        static_cast<uint32_t>(1),          // property: MSC
        static_cast<int64_t>(130000000));  // amount to transfer: 1.3 MSC (in willets)

    BOOST_CHECK_EQUAL(HexStr(vch), "00000016000000010000000007bfa480");
}

BOOST_AUTO_TEST_CASE(payload_create_property)
{
    // Create property [type 50, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceFixed(
        static_cast<uint8_t>(1),             // ecosystem: main
        static_cast<uint16_t>(1),            // property type: indivisible tokens
        static_cast<uint32_t>(0),            // previous property: none
        std::string("Companies"),            // category
        std::string("Bitcoin Mining"),       // subcategory
        std::string("Quantum Miner"),        // label
        std::string("builder.bitwatch.co"),  // website
        std::string(""),                     // additional information
        static_cast<int64_t>(1000000));      // number of units to create

    BOOST_CHECK_EQUAL(HexStr(vch),
        "0000003201000100000000436f6d70616e69657300426974636f696e204d696e696e67"
        "005175616e74756d204d696e6572006275696c6465722e62697477617463682e636f00"
        "0000000000000f4240");
}

BOOST_AUTO_TEST_CASE(payload_create_property_empty)
{
    // Create property [type 50, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceFixed(
        static_cast<uint8_t>(1),         // ecosystem: main
        static_cast<uint16_t>(1),        // property type: indivisible tokens
        static_cast<uint32_t>(0),        // previous property: none
        std::string(""),                 // category
        std::string(""),                 // subcategory
        std::string(""),                 // label
        std::string(""),                 // website
        std::string(""),                 // additional information
        static_cast<int64_t>(1000000));  // number of units to create

    BOOST_CHECK_EQUAL(vch.size(), 24);
}

BOOST_AUTO_TEST_CASE(payload_create_property_full)
{
    // Create property [type 50, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceFixed(
        static_cast<uint8_t>(1),         // ecosystem: main
        static_cast<uint16_t>(1),        // property type: indivisible tokens
        static_cast<uint32_t>(0),        // previous property: none
        std::string(700, 'x'),           // category
        std::string(700, 'x'),           // subcategory
        std::string(700, 'x'),           // label
        std::string(700, 'x'),           // website
        std::string(700, 'x'),           // additional information
        static_cast<int64_t>(1000000));  // number of units to create

    BOOST_CHECK_EQUAL(vch.size(), 1299);
}

BOOST_AUTO_TEST_CASE(payload_create_crowdsale)
{
    // Create crowdsale [type 51, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceVariable(
        static_cast<uint8_t>(1),             // ecosystem: main
        static_cast<uint16_t>(1),            // property type: indivisible tokens
        static_cast<uint32_t>(0),            // previous property: none
        std::string("Companies"),            // category
        std::string("Bitcoin Mining"),       // subcategory
        std::string("Quantum Miner"),        // label
        std::string("builder.bitwatch.co"),  // website
        std::string(""),                     // additional information
        static_cast<uint32_t>(1),            // property desired: MSC
        static_cast<int64_t>(100),           // tokens per unit vested
        static_cast<uint64_t>(7731414000),   // deadline: 31 Dec 2214 23:00:00 UTC
        static_cast<uint8_t>(10),            // early bird bonus: 10 % per week
        static_cast<uint8_t>(12));           // issuer bonus: 12 %

    BOOST_CHECK_EQUAL(HexStr(vch),
        "0000003301000100000000436f6d70616e69657300426974636f696e204d696e696e67"
        "005175616e74756d204d696e6572006275696c6465722e62697477617463682e636f00"
        "0000000001000000000000006400000001ccd403f00a0c");
}

BOOST_AUTO_TEST_CASE(payload_create_crowdsale_empty)
{
    // Create crowdsale [type 51, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceVariable(
        static_cast<uint8_t>(1),            // ecosystem: main
        static_cast<uint16_t>(1),           // property type: indivisible tokens
        static_cast<uint32_t>(0),           // previous property: none
        std::string(""),                    // category
        std::string(""),                    // subcategory
        std::string(""),                    // label
        std::string(""),                    // website
        std::string(""),                    // additional information
        static_cast<uint32_t>(1),           // property desired: MSC
        static_cast<int64_t>(100),          // tokens per unit vested
        static_cast<uint64_t>(7731414000),  // deadline: 31 Dec 2214 23:00:00 UTC
        static_cast<uint8_t>(10),           // early bird bonus: 10 % per week
        static_cast<uint8_t>(12));          // issuer bonus: 12 %

    BOOST_CHECK_EQUAL(vch.size(), 38);
}

BOOST_AUTO_TEST_CASE(payload_create_crowdsale_full)
{
    // Create crowdsale [type 51, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceVariable(
        static_cast<uint8_t>(1),            // ecosystem: main
        static_cast<uint16_t>(1),           // property type: indivisible tokens
        static_cast<uint32_t>(0),           // previous property: none
        std::string(700, 'x'),              // category
        std::string(700, 'x'),              // subcategory
        std::string(700, 'x'),              // label
        std::string(700, 'x'),              // website
        std::string(700, 'x'),              // additional information
        static_cast<uint32_t>(1),           // property desired: MSC
        static_cast<int64_t>(100),          // tokens per unit vested
        static_cast<uint64_t>(7731414000),  // deadline: 31 Dec 2214 23:00:00 UTC
        static_cast<uint8_t>(10),           // early bird bonus: 10 % per week
        static_cast<uint8_t>(12));          // issuer bonus: 12 %

    BOOST_CHECK_EQUAL(vch.size(), 1313);
}

BOOST_AUTO_TEST_CASE(payload_close_crowdsale)
{
    // Close crowdsale [type 53, version 0]
    std::vector<unsigned char> vch = CreatePayload_CloseCrowdsale(
        static_cast<uint32_t>(9));  // property: SP #9

    BOOST_CHECK_EQUAL(HexStr(vch), "0000003500000009");
}

BOOST_AUTO_TEST_CASE(payload_create_managed_property)
{
    // create managed property [type 54, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceManaged(
        static_cast<uint8_t>(1),             // ecosystem: main
        static_cast<uint16_t>(1),            // property type: indivisible tokens
        static_cast<uint32_t>(0),            // previous property: none
        std::string("Companies"),            // category
        std::string("Bitcoin Mining"),       // subcategory
        std::string("Quantum Miner"),        // label
        std::string("builder.bitwatch.co"),  // website
        std::string(""));                    // additional information

    BOOST_CHECK_EQUAL(HexStr(vch),
        "0000003601000100000000436f6d70616e69657300426974636f696e204d696e696e67"
        "005175616e74756d204d696e6572006275696c6465722e62697477617463682e636f00"
        "00");
}

BOOST_AUTO_TEST_CASE(payload_create_managed_property_empty)
{
    // create managed property [type 54, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceManaged(
        static_cast<uint8_t>(1),   // ecosystem: main
        static_cast<uint16_t>(1),  // property type: indivisible tokens
        static_cast<uint32_t>(0),  // previous property: none
        std::string(""),           // category
        std::string(""),           // subcategory
        std::string(""),           // label
        std::string(""),           // website
        std::string(""));          // additional information

    BOOST_CHECK_EQUAL(vch.size(), 16);
}

BOOST_AUTO_TEST_CASE(payload_create_managed_property_full)
{
    // create managed property [type 54, version 0]
    std::vector<unsigned char> vch = CreatePayload_IssuanceManaged(
        static_cast<uint8_t>(1),   // ecosystem: main
        static_cast<uint16_t>(1),  // property type: indivisible tokens
        static_cast<uint32_t>(0),  // previous property: none
        std::string(700, 'x'),     // category
        std::string(700, 'x'),     // subcategory
        std::string(700, 'x'),     // label
        std::string(700, 'x'),     // website
        std::string(700, 'x'));    // additional information

    BOOST_CHECK_EQUAL(vch.size(), 1291);
}

BOOST_AUTO_TEST_CASE(payload_grant_tokens)
{
    // Grant tokens [type 55, version 0]
    std::vector<unsigned char> vch = CreatePayload_Grant(
        static_cast<uint32_t>(8),                  // property: SP #8
        static_cast<int64_t>(1000),                // number of units to issue
        std::string("First Milestone Reached!"));  // additional information

    BOOST_CHECK_EQUAL(HexStr(vch),
        "000000370000000800000000000003e84669727374204d696c6573746f6e6520526561"
        "636865642100");
}

BOOST_AUTO_TEST_CASE(payload_grant_tokens_empty)
{
    // Grant tokens [type 55, version 0]
    std::vector<unsigned char> vch = CreatePayload_Grant(
        static_cast<uint32_t>(8),                  // property: SP #8
        static_cast<int64_t>(1000),                // number of units to issue
        std::string(""));                          // additional information

    BOOST_CHECK_EQUAL(vch.size(), 17);
}

BOOST_AUTO_TEST_CASE(payload_grant_tokens_full)
{
    // Grant tokens [type 55, version 0]
    std::vector<unsigned char> vch = CreatePayload_Grant(
        static_cast<uint32_t>(8),                  // property: SP #8
        static_cast<int64_t>(1000),                // number of units to issue
        std::string(700, 'x'));                    // additional information

    BOOST_CHECK_EQUAL(vch.size(), 272);
}

BOOST_AUTO_TEST_CASE(payload_revoke_tokens)
{
    // Revoke tokens [type 56, version 0]
    std::vector<unsigned char> vch = CreatePayload_Revoke(
        static_cast<uint32_t>(8),                                   // property: SP #8
        static_cast<int64_t>(1000),                                 // number of units to revoke
        std::string("Redemption of tokens for Bob, Thanks Bob!"));  // additional information

    BOOST_CHECK_EQUAL(HexStr(vch),
        "000000380000000800000000000003e8526564656d7074696f6e206f6620746f6b656e"
        "7320666f7220426f622c205468616e6b7320426f622100");
}

BOOST_AUTO_TEST_CASE(payload_revoke_tokens_empty)
{
    // Revoke tokens [type 56, version 0]
    std::vector<unsigned char> vch = CreatePayload_Revoke(
        static_cast<uint32_t>(8),    // property: SP #8
        static_cast<int64_t>(1000),  // number of units to revoke
        std::string(""));            // additional information

    BOOST_CHECK_EQUAL(vch.size(), 17);
}

BOOST_AUTO_TEST_CASE(payload_revoke_tokens_full)
{
    // Revoke tokens [type 56, version 0]
    std::vector<unsigned char> vch = CreatePayload_Revoke(
        static_cast<uint32_t>(8),    // property: SP #8
        static_cast<int64_t>(1000),  // number of units to revoke
        std::string(700, 'x'));      // additional information

    BOOST_CHECK_EQUAL(vch.size(), 272);
}

BOOST_AUTO_TEST_CASE(payload_change_property_manager)
{
    // Change property manager [type 70, version 0]
    std::vector<unsigned char> vch = CreatePayload_ChangeIssuer(
        static_cast<uint32_t>(13));  // property: SP #13

    BOOST_CHECK_EQUAL(HexStr(vch), "000000460000000d");
}


BOOST_AUTO_TEST_SUITE_END()
