#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "hashmap.h"
#include "DistributionContract.h"

#define INDEX_INIT 1000000
#define INCR_PER_REV_INIT 10000

// Utility function prototype
void distributeRevenue(DistributionContract* contract, double amount);

// Hash function
uint64_t userDataHash(const void* item, uint64_t seed0, uint64_t seed1) {
    const UserState* userState = item;
    return hashmap_sip(userState->address, strlen(userState->address), seed0, seed1);
}

// Compare function
int userDataCompare(const void* a, const void* b, void* udata) {
    const UserState* userState1 = a;
    const UserState* userState2 = b;
    return strcmp(userState1->address, userState2->address);
}

/**
 * @brief Constructs a distribution contract
 * 
 * @return DistributionContract*
 */
DistributionContract* constructContract() {
    DistributionContract* contract = calloc(1, sizeof(DistributionContract));
    if(!contract) return NULL;
    contract->userStateMap = hashmap_new(sizeof(UserState), 0, 0, 0, userDataHash, userDataCompare, NULL, NULL);
    if(!contract->userStateMap) {
        free(contract);
        return NULL;
    }
    contract->totalStake = 0;
    contract->incrementPerRevenue = INCR_PER_REV_INIT;
    contract->index = INDEX_INIT;
    return contract;
}

/**
 * @brief Destroys a distribution contract
 * 
 * @param contract The contract to be destroyed
 */
void destroyContract(DistributionContract* contract) {
   // Sanitization
   if(!contract) return;
   hashmap_free(contract->userStateMap);
   free(contract);
}

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
int changeShare(DistributionContract* contract, char* dest, double change) {
    // Sanity checks
    if(change == 0 || !contract || !dest) return EXIT_FAILURE;
    double newTotalStake = contract->totalStake + change;
    double oldTotalStake = contract->totalStake;
    if(newTotalStake < 0 || !isfinite(newTotalStake)) return EXIT_FAILURE;
    // Get user data
    UserState* userState = hashmap_get(contract->userStateMap, &(UserState){ .address = dest });
    UserState tmp = { 0 };
    if(!userState) {
        // If no mapping exists
        tmp = (UserState){ dest, 0, contract->totalStake, contract->incrementPerRevenue, 0, contract->index };    
    } else {
        // If there was a mapping
        tmp = *userState;
    }
    if(tmp.ownStake + change < 0) return EXIT_FAILURE;
    // Phase 1
    double freshOwn = tmp.ownStake == 0 ? 0 : (contract->index - tmp.lastIndex) * tmp.ownStake / 
        (tmp.lastIncrementPerRevenue * tmp.lastTotalStake);
    tmp.ownAccumulatedTotal += freshOwn;
    // Phase 2
    if(newTotalStake != 0) { // TODO: Floating points can be dangerous!
        contract->incrementPerRevenue = INCR_PER_REV_INIT;
    } else if(oldTotalStake != 0) {
        contract->incrementPerRevenue *= oldTotalStake / newTotalStake;
    }
    contract->totalStake = newTotalStake;
    // Phase 3
    tmp.ownStake += change;
    tmp.lastIndex = contract->index;
    tmp.lastIncrementPerRevenue = contract->incrementPerRevenue;
    tmp.lastTotalStake = contract->totalStake;
    /// Update
    hashmap_set(contract->userStateMap, &tmp);
    return EXIT_SUCCESS;
}

/**
 * @brief Injects revenue into the contract
 * 
 * @param contract The destination address
 * @param amount The amount to add 
 * @return int Success code: 0 if succeeded else transaction revert
 */
int addRevenue(DistributionContract* contract, double amount) {
    // Sanity checks
    if(!contract || amount <= 0) return EXIT_FAILURE;
    distributeRevenue(contract, amount);
    return EXIT_SUCCESS;
}

/**
 * @brief Utility distribution function
 *
 * @param contract The destination address
 * @param amount The amount to distribute
 */
void distributeRevenue(DistributionContract* contract, double amount) {
    // Efficient distribution
    contract->index += contract->incrementPerRevenue * amount;
}