package src.revenue_distribution.optimized;

import src.TxnRevertException;

import java.util.HashMap;
import java.util.Map;

import static src.TxnRevertException.assertOrRevert;

/**
 * A simple naive revenue distribution contract
 */
public final class DistributionContract {

    // Global arbitrary initialization constants
    private final static double INDEX_INIT = 1000000;
    private final static double INCR_PER_REV_INIT = 10000;

    /**
     * Basic struct that models all the data associated to one address
     */
    private static final class UserData {
        private double ownStake;
        private double lastTotalStake;
        private double lastIncrementPerRevenue;
        private double ownAccumulatedTotal;
        private double lastIndex;
        public UserData(GlobalData globalData) {
            this(globalData, 0);
        }
        public UserData(GlobalData globalData, double share) {
            this.ownStake = share;
            this.lastTotalStake = globalData.totalStake;
            this.lastIncrementPerRevenue = globalData.incrementPerRevenue;
            this.ownAccumulatedTotal = 0;
            this.lastIndex = globalData.index;
        }
    }

    /**
     * Basic struct that models all global data of the contract
     */
    private static final class GlobalData {
        private double totalStake;
        private double incrementPerRevenue;
        private double index;
        public GlobalData() {
            this.totalStake = 0;
            this.incrementPerRevenue = INCR_PER_REV_INIT;
            this.index = INDEX_INIT;
        }
    }

    // Contract's internal storage
    private final Map<String, UserData> userDataMap;
    private final GlobalData globalData;


    /**
     * Contract's constructor, for personal testing setup
     * @param map The address -> share mapping
     */
    public DistributionContract(Map<String, Integer> map) {
        var userDataMap = new HashMap<String, UserData>();
        var globalData = new GlobalData();
        // First loop to create global data
        for(var value : map.values()) {
            assertOrRevert(value > 0);
            globalData.totalStake += value;
        }
        // Second loop to create user data, once global data is up-to-date
        for(var entry : map.entrySet()) {
            userDataMap.put(entry.getKey(), new UserData(globalData, entry.getValue()));
        }
        this.userDataMap = userDataMap;
        this.globalData = globalData;
    }

    /**
     * Public function to add share to the sender
     * WARNING: Any user may add as much share as they want
     * This has been done to isolate only the revenue distribution
     * and not the transfer (whose time can vary depending on the implementation)
     * @param dest The destination address of the change
     * @param change The amount to add/remove
     * @exception TxnRevertException if the change incurs an over/underflow
     */
    public void changeShare(String dest, double change) {
        // Sanitization
        assertOrRevert(change != 0);
        var newTotalStake = globalData.totalStake + change;
        var oldTotalStake = globalData.totalStake;
        assertOrRevert(Double.isFinite(newTotalStake) && newTotalStake >= 0);
        UserData userData = userDataMap.getOrDefault(dest, new UserData(globalData));
        assertOrRevert(userData.ownStake + change >= 0);
        // Phase 1
        var freshOwn = userData.ownStake == 0 ? 0 : (globalData.index - userData.lastIndex) * userData.ownStake /
                (userData.lastIncrementPerRevenue * userData.lastTotalStake);
        userData.ownAccumulatedTotal += freshOwn;
        // Phase 2
        if(newTotalStake != 0) { // TODO: Floating points CAN be dangerous!
            globalData.incrementPerRevenue = INCR_PER_REV_INIT;
        } else if(oldTotalStake != 0) {
            globalData.incrementPerRevenue *= oldTotalStake / newTotalStake;
        }
        globalData.totalStake = newTotalStake;
        // Phase 3
        userData.ownStake += change;
        userData.lastIndex = globalData.index;
        userData.lastIncrementPerRevenue = globalData.incrementPerRevenue;
        userData.lastTotalStake = globalData.totalStake;
        userDataMap.put(dest, userData);
    }

    /**
     * Injects revenue into the contract
     * @param amount The amount to add
     * @exception TxnRevertException if amount is not strictly positive
     */
    public void addRevenue(double amount) {
        // Check whether the change is strictly positive
        assertOrRevert(amount > 0);
        distributeRevenue(amount);
    }

    /**
     *  Utility distribution function
     * @param amount The amount to distribute
     */
    private void distributeRevenue(double amount) {
        // Efficient distribution
        globalData.index += globalData.incrementPerRevenue * amount;
    }
}