package benchmarking.revenue_distribution;;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.RunnerException;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import src.revenue_distribution.optimized.DistributionContract;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import static benchmarking.revenue_distribution.OptimizedSimulation.Contracts.*;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Warmup(iterations = 1, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(2)
@State(Scope.Benchmark)
public class OptimizedSimulation {

    @State(Scope.Benchmark)
    public enum Contracts {
        HUNDRED(100), THOUSAND(1000), TEN_THOUSAND(10000), HUNDRED_THOUSAND(100000),
        MILLION(1000000), TEN_MILLION(10000000);

        public final DistributionContract contract;

        Contracts(int n) {
            Map<String, Integer> initMap = new HashMap<>();
            for(int i = 1 ; i <= n ; ++i) {
                initMap.put(Integer.toString(i), i);
            }
            this.contract = new DistributionContract(initMap);
        }
    }

    @Benchmark
    public void hundredUsersDistribution(Blackhole bh) {
        DistributionContract contract = HUNDRED.contract;
        contract.addRevenue(100);
        bh.consume(contract);
    }

    @Benchmark
    public void thousandUsersDistribution(Blackhole bh) {
        DistributionContract contract = THOUSAND.contract;
        contract.addRevenue(1000);
        bh.consume(contract);
    }

    @Benchmark
    public void tenThousandUsersDistribution(Blackhole bh) {
        DistributionContract contract = TEN_THOUSAND.contract;
        contract.addRevenue(10000);
        bh.consume(contract);
    }

    @Benchmark
    public void hundredThousandUsersDistribution(Blackhole bh) {
        DistributionContract contract = HUNDRED_THOUSAND.contract;
        contract.addRevenue(100000);
        bh.consume(contract);
    }

    @Benchmark
    public void millionUsersDistribution(Blackhole bh) {
        DistributionContract contract = MILLION.contract;
        contract.addRevenue(1000000);
        bh.consume(contract);
    }

    @Benchmark
    public void tenMillionUsersDistribution(Blackhole bh) {
        DistributionContract contract = TEN_MILLION.contract;
        contract.addRevenue(10000000);
        bh.consume(contract);
    }

    @Benchmark
    public void hundredUsersChangeShare(Blackhole bh) {
        DistributionContract contract = HUNDRED.contract;
        contract.changeShare("1", 100);
        bh.consume(contract);
    }

    @Benchmark
    public void tenThousandUsersChangeShare(Blackhole bh) {
        DistributionContract contract = TEN_THOUSAND.contract;
        contract.changeShare("100", 10000);
        bh.consume(contract);
    }

    @Benchmark
    public void millionUsersChangeShare(Blackhole bh) {
        DistributionContract contract = MILLION.contract;
        contract.changeShare("10000", 1000000);
        bh.consume(contract);
    }

    public static void main(String[] args) throws RunnerException {

        Options opt = new OptionsBuilder()
                .include(OptimizedSimulation.class.getSimpleName())
                .build();

        new Runner(opt).run();
    }
}