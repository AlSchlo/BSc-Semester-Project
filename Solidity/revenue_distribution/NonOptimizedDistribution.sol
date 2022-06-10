// SPDX-License-Identifier: MIT

pragma solidity 0.7.0;

contract NonOptimizedDistribution {

    //----------------------------//
    //--------- Structs ----------//
    //----------------------------//
    
    /**
     * Contains all user state for revenue distribution
     */ 
    struct UserState {
        uint share; // The owned share of the total stake
        uint revenue; // The total revenue accumulated
        bool present; // Flag to signal whether this user has made a transaction
    }
    
    //----------------------------//
    //----- State Variables ------//
    //----------------------------//

    // Global data
    
    uint public totalShare;

    // Users data

    uint public totalHolders;
    mapping(uint => address) public users;
    mapping(address => UserState) public usersStateMap;

    //----------------------------//
    //------- Constructor --------//
    //----------------------------//
    
    /**
     * @dev Initializes the contract, setting up all needed values.
     */
    constructor(address[] memory _userAddresses, uint[] memory _initialStake) {
        totalHolders = _userAddresses.length;
        require(totalHolders == _initialStake.length);
        totalShare = 0;
        for(uint i = 0; i < totalHolders; ++i) {
            address a = _userAddresses[i];
            uint s = _initialStake[i];
            users[i] = a;
            usersStateMap[a] = UserState(s, 0, true);
            totalShare += s;
        }
    }
    
    //----------------------------//
    //------- EXTERNAL API -------//
    //----------------------------//

    /**
     * @dev Function to add share to the sender address
     * WARNING: Any user may add as much share as they want
     * This has been done to isolate only the revenue distribution
     * and not the transfer (whose time can vary depending on the implementation)
     * 
     * @param _amount The amount to add/remove
     */
    function changeShare(int _amount) public {
        require(_amount != 0, "Amount cannot be zero");
        if(!containsUser(msg.sender)) {
            UserState storage newUserState = usersStateMap[msg.sender]; 
            newUserState.present = true;
            users[totalHolders++] = msg.sender; 
        }
        int newTotalShare = int(totalShare) + _amount;
        int newUserShare = int(usersStateMap[msg.sender].share) + _amount;
        usersStateMap[msg.sender].share = uint(newUserShare);
        totalShare = uint(newTotalShare);
    }

    /**
     * @dev Injects revenue into the contract
     * 
     * @param _amount The amount to add
     */
    function addRevenue(uint _amount) public {
        require(_amount != 0, "Amount cannot be zero");
        distributeRevenue(_amount);
    }

    //----------------------------//
    //------- INTERNAL API -------//
    //----------------------------//

    /**
     * @dev Utility distribution function
     *
     * @param _amount The amount to distribute
     */
    function distributeRevenue(uint _amount) internal {
        // Naive loop
        for(uint i = 0; i < totalHolders; ++i) {
            UserState storage userState = usersStateMap[users[i]];
            userState.revenue += (_amount * userState.share) / totalShare;
        }
    }

    /**
     * @dev Utility view function
     *
     * @param _user The user to check
     */
    function containsUser(address _user) internal view returns (bool) {
        return usersStateMap[_user].present;
    } 
}
