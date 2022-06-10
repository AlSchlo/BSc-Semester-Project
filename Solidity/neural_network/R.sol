// SPDX-License-Identifier: MIT

pragma solidity ^0.7.0;

import "./F.sol";

library R {

    using F for bytes16;

    int64 constant multiplier = 0x5DEECE66D;
    int64 constant addend = 0xB;
    int64 constant mask = (1 << 48) - 1;

    struct Random {
        int64 seed;
    }

    function initSeed(int64 seed) internal pure returns (Random memory result) {
        result.seed = (seed ^ multiplier) & mask;
    }

    function nextFloat(Random memory r) internal pure returns (bytes16) {
        int64 v1 = (int64(nextBits(r, 26)) << 27);
        int64 v2 = int64(nextBits(r, 27));
        int64 denom = 1 << 53;
        return F.fromInt(int(v1 + v2)).div(F.fromInt(int(denom)));
    }

    function nextFloat(Random memory r, bytes16 low, bytes16 high) internal pure returns (bytes16) {
        bytes16 f = nextFloat(r);
        return f.mul(high.sub(low)).add(low);
    }

    // --- Private functions --- //

    function nextBits(Random memory r, uint bits) private pure returns (int32) {
        require(1 <= bits && bits <= 32);
        int64 nextSeed = (r.seed * multiplier + addend) & mask;
        // Complex bit-wise operations since solidity does not support SAR 
        // even though the opcode exist
        int64 sign = nextSeed >= 0 ? 1 : 0;
        int64 arithmeticMask = ~((sign << (16 + bits)) - 1);
        r.seed = nextSeed;
        return int32(arithmeticMask | (nextSeed >> (48 - bits)));
    }
}