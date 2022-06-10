// SPDX-License-Identifier: MIT

pragma solidity 0.7.0;

contract OptimizedDistribution {

    //----------------------------//
    //--------- Structs ----------//
    //----------------------------//
    
    /**
     * Contains all personal data needed by a user to calculate 
     * the lazy accumulated share of the pool since beginning.
     */ 
    struct UserState {
        uint144 ownStake;
        uint144 lastTotalStake;
        uint144 lastIncrementPerRevenue;
        uint112 ownAccumulatedTotal;
        uint256 lastIndex;
    }

    /**
     * Contains all global data needed by a user to calculate 
     * the lazy accumulated share of the pool since beginning.
     */    
    struct GlobalState {
        uint144 totalStake;
        uint144 incrementPerRevenue;
        uint256 lastIndex;
    }
    
    //----------------------------//
    //-------- Constants ---------//
    //----------------------------//
    
    uint144 constant START_INCREMENT_PER_REVENUE = (2 ** 144) - 1;

    //----------------------------//
    //----- State Variables ------//
    //----------------------------//

    // Global data

    GlobalState globalState;
    
    // Users data
    
    mapping(address => UserState) usersStateMap;

    //----------------------------//
    //------- Constructor --------//
    //----------------------------//
    
    /**
     * @dev Initializes the contract, setting up all needed values.
     * WARNING: The size of both lists must match
     * @param _userAddresses A list of addresses
     * @param _initialStake A list of stakes
     */
    constructor(address[] memory _userAddresses, uint[] memory _initialStake) {
        uint totalHolders = _userAddresses.length;
        require(totalHolders == _initialStake.length);
        // First loop to create global data
        for(uint i = 0; i < totalHolders; ++i) {
            uint s = _initialStake[i];
            globalState.totalStake += uint144(s);
        }
        globalState.incrementPerRevenue = START_INCREMENT_PER_REVENUE;
        // Second loop to create user data, once global data is up-to-date    
        for(uint i = 0; i < totalHolders; ++i) {
            UserState storage userState = usersStateMap[_userAddresses[i]];
            userState.ownStake = uint144(_initialStake[i]);
            userState.lastTotalStake = globalState.totalStake;
            userState.lastIncrementPerRevenue = globalState.incrementPerRevenue;
            userState.ownAccumulatedTotal = 0;
            userState.lastIndex = globalState.lastIndex;
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
    function changeShare(int256 _amount) public {
        require(_amount != 0, "Amount cannot be zero");
        UserState storage userState = usersStateMap[msg.sender];
        // Phase 1
        uint256 freshOwn = userState.ownStake == 0 ? 0 : mulDiv(globalState.lastIndex - userState.lastIndex, 
                userState.ownStake, userState.lastIncrementPerRevenue * userState.lastTotalStake);
        userState.ownAccumulatedTotal += uint112(freshOwn);
        // Phase 2
        uint256 oldTotalStake = globalState.totalStake;
        uint256 newTotalStake = uint256(int256(oldTotalStake) + _amount);
        if(newTotalStake == 0) {
            globalState.incrementPerRevenue = uint144(START_INCREMENT_PER_REVENUE);
        } else if(oldTotalStake != 0) {
            globalState.incrementPerRevenue = uint144(mulDiv(globalState.incrementPerRevenue, oldTotalStake, newTotalStake));
        }
        globalState.totalStake = uint144(newTotalStake);
        // Phase 3
        userState.lastIndex = globalState.lastIndex;
        userState.lastIncrementPerRevenue = globalState.incrementPerRevenue;
        userState.lastTotalStake = globalState.totalStake;
        userState.ownStake = uint144(uint256(int256(uint256(userState.ownStake)) + _amount));
    }

    /**
     * @dev Injects revenue into the contract
     * 
     * @param _amount The amount to add
     */
    function addRevenue(uint256 _amount) public {
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
    function distributeRevenue(uint256 _amount) private {
        // Efficient distribution
        globalState.lastIndex += uint256(globalState.incrementPerRevenue) * _amount;
    }

    /** 
     * @notice Calculates floor(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
     * @param a The multiplicand
     * @param b The multiplier
     * @param denominator The divisor
     * @return result The 256-bit result
     * @dev Credit to Remco Bloemen under MIT license https://xn--2-umb.com/21/muldiv
     */
    function mulDiv(uint256 a, uint256 b, uint256 denominator) private pure returns (uint256 result) {
        // 512-bit multiply [prod1 prod0] = a * b
        // Compute the product mod 2**256 and mod 2**256 - 1
        // then use the Chinese Remainder Theorem to reconstruct
        // the 512 bit result. The result is stored in two 256
        // variables such that product = prod1 * 2**256 + prod0
        uint256 prod0; // Least significant 256 bits of the product
        uint256 prod1; // Most significant 256 bits of the product
        assembly {
            let mm := mulmod(a, b, not(0))
            prod0 := mul(a, b)
            prod1 := sub(sub(mm, prod0), lt(mm, prod0))
        }
    
        // Handle non-overflow cases, 256 by 256 division
        if (prod1 == 0) {
            require(denominator > 0);
            assembly {
                result := div(prod0, denominator)
            }
            return result;
        }
    
        // Make sure the result is less than 2**256.
        // Also prevents denominator == 0
        require(denominator > prod1);
    
        //////////////////////////////////////////////
        //             512 by 256 division.         //
        //////////////////////////////////////////////
    
        // Make division exact by subtracting the remainder from [prod1 prod0]
        // Compute remainder using mulmod
        uint256 remainder;
        assembly {
            remainder := mulmod(a, b, denominator)
        }
        // Subtract 256 bit number from 512 bit number
        assembly {
            prod1 := sub(prod1, gt(remainder, prod0))
            prod0 := sub(prod0, remainder)
        }
    
        // Factor powers of two out of denominator
        // Compute largest power of two divisor of denominator.
        // Always >= 1.
         {
            uint256 twos = (type(uint256).max - denominator + 1) & denominator;
            // Divide denominator by power of two
            assembly {
                denominator := div(denominator, twos)
            }
    
            // Divide [prod1 prod0] by the factors of two
            assembly {
                prod0 := div(prod0, twos)
            }
            // Shift in bits from prod1 into prod0. For this we need
            // to flip `twos` such that it is 2**256 / twos.
            // If twos is zero, then it becomes one
            assembly {
                twos := add(div(sub(0, twos), twos), 1)
            }
            prod0 |= prod1 * twos;
    
            // Invert denominator mod 2**256
            // Now that denominator is an odd number, it has an inverse
            // modulo 2**256 such that denominator * inv = 1 mod 2**256.
            // Compute the inverse by starting with a seed that is correct
            // correct for four bits. That is, denominator * inv = 1 mod 2**4
            uint256 inv = (3 * denominator) ^ 2;
            // Now use Newton-Raphson iteration to improve the precision.
            // Thanks to Hensel's lifting lemma, this also works in modular
            // arithmetic, doubling the correct bits in each step.
            inv *= 2 - denominator * inv; // inverse mod 2**8
            inv *= 2 - denominator * inv; // inverse mod 2**16
            inv *= 2 - denominator * inv; // inverse mod 2**32
            inv *= 2 - denominator * inv; // inverse mod 2**64
            inv *= 2 - denominator * inv; // inverse mod 2**128
            inv *= 2 - denominator * inv; // inverse mod 2**256
    
            // Because the division is now exact we can divide by multiplying
            // with the modular inverse of denominator. This will give us the
            // correct result modulo 2**256. Since the precoditions guarantee
            // that the outcome is less than 2**256, this is the final result.
            // We don't need to compute the high bits of the result and prod1
            // is no longer required.
            result = prod0 * inv;
            return result;
        }
    }
}
