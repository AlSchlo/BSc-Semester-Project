#pragma once

#include "hashmap.h"

// The state of the contract at any moment
typedef struct {
    struct hashmap* userStateMap;
    double totalShare;
} DistributionContract;

// An entry of the hashmap
typedef struct {
    char* address;
    double share;
    double revenue;
} UserState;

/**
 * @brief Constructs a distribution contract
 * 
 * @return DistributionContract*
 */
DistributionContract* constructContract();

/**
 * @brief Destroys a distribution contract
 * 
 * @param contract The contract to be destroyed
 */
void destroyContract(DistributionContract* contract);

/**
 * @brief Function to add share to the destination address
 * WARNING: Any user may add as much share as they want
 * This has been done to isolate only the revenue distribution
 * and not the transfer (whose time can vary depending on the implementation)
 * 
 * @param contract The contract 
 * @param dest The destination address of the change
 * @param change The amount to add/remove
 * @return int Success code: 0 if succeeded else transaction revert
 */
int changeShare(DistributionContract* contract, char* dest, double change);

/**
 * @brief Injects revenue into the contract
 * 
 * @param contract The destination address
 * @param amount The amount to add 
 * @return int Success code: 0 if succeeded else transaction revert
 */
int addRevenue(DistributionContract* contract, double amount);
