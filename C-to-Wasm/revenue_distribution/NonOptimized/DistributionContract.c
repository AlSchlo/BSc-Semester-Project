#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "hashmap.h"
#include "DistributionContract.h"

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
    // Check whether the transaction will cause a global over/underflow
    double newTotalShare = contract->totalShare + change;
    if(newTotalShare < 0 || !isfinite(newTotalShare)) return EXIT_FAILURE;
    // Update global data
    contract->totalShare = newTotalShare;
    // Update user data
    UserState* userState = hashmap_get(contract->userStateMap, &(UserState){ .address = dest });
    UserState tmp = { 0 };
    if(!userState) {
        // If no mapping exists
        tmp = (UserState){ dest, 0, 0 };    
    } else {
        // If there was a mapping
        tmp = *userState;
    }
    double newUserShare = tmp.share + change;
    // Check whether the transaction will cause a user underflow
    if(newUserShare < 0) return EXIT_FAILURE;
    tmp.share = newUserShare;
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
    // Naive loop
    struct Node* i = contract->userStateMap->head;
    void* item;
    while(hashmap_iter(contract->userStateMap, &i, &item)) {
        UserState* userState = item;
        userState->revenue += (userState->share / contract->totalShare) * amount;
        // printf("Nouvelle valeur: %s: %lf\n", userState->address, userState->revenue); // DEBUG
    }
}