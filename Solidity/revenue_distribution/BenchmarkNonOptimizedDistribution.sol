// SPDX-License-Identifier: MIT

pragma solidity 0.7.0;

import "./NonOptimizedDistribution.sol";

contract BenchmarkNonOptimizedDistribution is NonOptimizedDistribution {

    uint constant NB_USERS = 100;

    constructor() 
        NonOptimizedDistribution(generateAddresses(NB_USERS), generateStakes(NB_USERS)) {}

    function generateAddresses(uint _nb) private pure returns (address[] memory) {
        address[] memory addresses = new address[](_nb);
        for (uint i = 0; i < _nb; ++i) {
            addresses[i] = address(uint160(i));
        }
        return addresses;
    }

    function generateStakes(uint _nb) private pure returns (uint[] memory) {
        uint[] memory stakes = new uint[](_nb);
        for (uint i = 0; i < _nb; ++i) {
            stakes[i] = i + 1;
        }
        return stakes;
    }

    function BenchmarkDistribute() external {
        super.addRevenue(100);
    }
}
