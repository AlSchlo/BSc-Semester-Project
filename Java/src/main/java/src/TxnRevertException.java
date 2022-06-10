package src;

public final class TxnRevertException extends RuntimeException {
    /**
     * Asserts a condition or reverts the transaction
     * @param condition The condition to be verified
     * @exception TxnRevertException if the condition is false
     */
    public static void assertOrRevert(boolean condition) {
        if(!condition) throw new TxnRevertException();
    }
}