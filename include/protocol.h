#pragma once
#include <cstdint>

// pack(1) = no padding between fields, so the struct's byte layout
// matches exactly what we'd send/receive over a real wire protocol.
#pragma pack(push, 1)

struct AddOrderMessage {
    char message_type;      // 'A'
    uint64_t timestamp;     // nanoseconds since midnight
    uint64_t order_id;      // unique order identifier
    char side;               // 'B' = buy, 'S' = sell
    uint32_t price;          // price in ticks (e.g. $150.25 -> 1502500, 4 decimal places)
    uint32_t quantity;       // number of shares
    char symbol[8];           // ticker, space-padded (e.g. "AAPL    ")
};

struct DeleteOrderMessage {
    char message_type;      // 'D'
    uint64_t timestamp;
    uint64_t order_id;      // which order got cancelled
};

struct OrderExecutedMessage {
    char message_type;      // 'E'
    uint64_t timestamp;
    uint64_t order_id;      // which order got (partially) filled
    uint32_t executed_quantity;
};

#pragma pack(pop)
