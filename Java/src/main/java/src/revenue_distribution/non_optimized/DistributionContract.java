package src.revenue_distribution.non_optimized;

import src.TxnRevertException;

import java.util.HashMap;
import java.util.Map;

import static src.TxnRevertException.assertOrRevert;

/**
 * A simple naive revenue distribution contract
 */
public final class DistributionContract {
    /**
     * Basic struct that models all the data associated to one address
     */
    private static final class UserData {
        private double share;
        private double revenue;
        public UserData() {
            this(0);
        }
        public UserData(double share) {
            this.share = share;
            this.revenue = 0;
        }
    }

    // Contract's internal storage
    private final Map<String, UserData> userDataMap;
    private double totalShare;

    /**
     * Contract's constructor, for personal testing setup
     * @param map The address -> share mapping
     */
    public DistributionContract(Map<String, Integer> map) {
        double totalShare = 0;
        var userDataMap = new HashMap<String, UserData>();
        for(var entry : map.entrySet()) {
            assertOrRevert(entry.getValue() > 0);
            userDataMap.put(entry.getKey(), new UserData(entry.getValue()));
            totalShare += entry.getValue();
        }
        this.userDataMap = userDataMap;
        this.totalShare = totalShare;
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
        // Reject 0
        assertOrRevert(change != 0);
        // Check whether the transaction will cause a global over/underflow
        var newTotalShare = totalShare + change;
        assertOrRevert(Double.isFinite(newTotalShare) && newTotalShare >= 0);
        // Update global data
        totalShare = newTotalShare;
        // Update user data
        UserData data = userDataMap.getOrDefault(dest, new UserData());
        var newUserShare = data.share + change;
        // Check whether the transaction will cause a user underflow
        assertOrRevert(newUserShare >= 0);
        data.share = newUserShare;
        userDataMap.put(dest, data);
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
        // Naive loop
        for(var user : userDataMap.values()) {
            user.revenue += (user.share / totalShare) * amount;
        }
    }
}